/*
 * Open Hexagon Server Software:
 * Depends on MariaDB's C connector and SFML
 * Written and developed by Mischa Alff, and Vittorio Romeo
 * (c) 2013 Mischa Alff and Vittorio Romeo
 */

//#include <my_global.h>
#include <mysql.h>
#include <iostream>
#include <string>
#include <memory>
#include <SFML/Network.hpp>

class MySQLSession
{
private:
    MYSQL *connector;
    MYSQL_RES *result;
    MYSQL_ROW row;
    bool initialized, verbose;
    std::string Hostname, username, password, database;
    int port;

    struct MySQLError
    {
        unsigned int errorno;
        std::string errorstr;
    };

    MySQLError getError()
    {
        MySQLError returnValue;
        returnValue.errorno = mysql_errno(connector);
        returnValue.errorstr = mysql_error(connector);
        std::cout<<"MySQL Error "<<std::to_string(returnValue.errorno)<<" : "<<returnValue.errorstr<<std::endl;
        return(returnValue);
    }

public:

    std::string runQuery(std::string queryString, bool get = false)
    {
        if(!initialized)
            return "";

        if(mysql_query(connector, queryString.c_str()))
        {
            return getError().errorstr;
        }
        return "";

        if(get)
        {
            int num_fields, i;

            std::string returnValue;

            result = mysql_store_result(connector);

            num_fields = mysql_num_fields(result);

            while ((row = mysql_fetch_row(result)))
            {
                for(i = 0; i < num_fields; i++)
                {
                    if(verbose)
                        std::cout<<(row[i] ? row[i] : "NULL");
                    returnValue += (row[i] ? row[i] : "NULL");
                }
                if(verbose)
                    std::cout<<std::endl;
            }

            mysql_free_result(result);

            return returnValue;
        }
    }

    bool initiate(std::string Hostname_, int port_, std::string username_, std::string password_, std::string database_ = "")
    {
        Hostname = Hostname_;
        username = username_;
        password = password_;
        database = database_;
        port = port_;

        if(mysql_real_connect(connector, Hostname.c_str(), username.c_str(), password.c_str(), database.c_str(), port, NULL, 0) == NULL)
        {
            getError();
            initialized = false;
            return initialized;
        }
        initialized = true;
        return initialized;
    }

    void closeSQL()
    {
        if(!initialized)
            return;
        mysql_close(connector);
    }

    MySQLSession(bool verbose_)
    {
        verbose = verbose_;
        connector = mysql_init(NULL);

        if(connector == NULL)
            getError();
    }
};
void Concantate(std::string &source)
{
    for(int i{0}; i < source.length(); i++)
    {
        switch(source[i])
        {
        case '\'':
        case '\"':
            source[i] = ' ';
        default:
            break;
        }
    }
}

bool handlePacket0x00(sf::Packet &packet, MySQLSession &session, sf::TcpSocket &socket, bool verbose)
{
    // This packet handles a score submission.
    // order: level, difficulty, username, score
    std::string level, username;
    float difficulty, score;
    int8_t temp;

    if(packet >> level >> difficulty >> username >> score)
    {

        Concantate(level);
        Concantate(username);

        if(verbose)
        {
            std::cout<<"Packet extracted successfully!"<<std::endl;
            std::cout<<"Level name: "<<level<<std::endl
                     <<"Difficulty: "<<difficulty<<std::endl
                     <<"Username: "<<username<<std::endl
                     <<"Score: "<<score<<std::endl;
        }


        //CREATE TABLE IF NOT EXISTS testScores (user_name VARCHAR(64) NOT NULL, difficulty DECIMAL(5,2) NOT NULL, score DECIMAL(9,3), PRIMARY KEY ( user_name, difficulty ) );

        std::string query {"CREATE TABLE IF NOT EXISTS " + level + "(user_name VARCHAR(64) NOT NULL, difficulty DECIMAL(5,2) NOT NULL, score DECIMAL(9,3), PRIMARY KEY ( user_name, difficulty ) );"};
        if(verbose) std::cout<<"Running query: "<<query<<std::endl;
        session.runQuery(query);
        query.clear();

        //INSERT INTO  testScores ( user_name, difficulty, score) VALUES ("someone", 1, 123.456);

        query = ("INSERT INTO " + level + " ( user_name, difficulty, score) VALUES (\"" + username + "\", " + std::to_string(difficulty) + ", " + std::to_string(score) + ");");
        if(verbose) std::cout<<"Running query: "<<query<<std::endl;
        session.runQuery(query);
        query.clear();

        //UPDATE somelevel SET score=13 WHERE user_name="John" AND score<13;
        query = ("UPDATE " + level + " SET score = " + std::to_string(score) + " WHERE user_name=\"" + username + "\" AND difficulty=" + std::to_string(difficulty) + " AND score<" + std::to_string(score) + ";");
        if(verbose) std::cout<<"Running query: "<<query<<std::endl;
        session.runQuery(query);

        return true;
    }

    else
    {
        std::cout<<"Error, packet 0x00 did not extract successfully."<<std::endl;
        return false;
    }

}

bool handlePacket0x01(sf::Packet &packet, MySQLSession &session, sf::TcpSocket &socket, bool verbose)
{

}

bool handlePackets(sf::Packet packet, MySQLSession &session, sf::TcpSocket &socket, bool verbose)
{
    int8_t packetIdentifier;
    if(packet >> packetIdentifier)
    {
        if(packetIdentifier == 0)
            handlePacket0x00(packet, session, socket, verbose);

        else if(packetIdentifier == 1)
            ;

    }
    return true;
}

int main(int argc, char** argv)
{
    bool verbose = true;

    MySQLSession mainSession {verbose};
    mainSession.initiate("localhost", 0, "root", "lolpassword", "oh_scores");

    //std::vector</*std::shared_ptr<*/sf::TcpSocket/*>*/> clients;
    unsigned int clients_size = 1000;
    sf::TcpSocket clients[clients_size];
    std::vector<bool>   clientAvailable;

    //clients.resize(1000);
    clientAvailable.assign(clients_size, true);

    sf::TcpListener server;
    server.setBlocking(false);
    server.listen(27272);

    std::cout<<"MySQL client version: "<<mysql_get_client_info()<<" \n";

    while(true)
    {
        unsigned int firstFreeSocket {0};
        for(; firstFreeSocket < clients_size; firstFreeSocket++)
            if(clientAvailable[firstFreeSocket])
                break;

        //std::cout<<firstFreeSocket<<","<<clients.size()<<std::endl;

        if(server.accept(clients[firstFreeSocket]) == sf::Socket::Done)
        {
            std::cout<<"Received packet!"<<std::endl;
            clientAvailable[firstFreeSocket] = false;
            clients[firstFreeSocket].setBlocking(false);
        }

        for(unsigned int i = 0; i < clients_size; i++)
        {
            if(!clientAvailable[i])
            {
                sf::TcpSocket::Status socketStatus;
                sf::Packet tempPacket;
                socketStatus = clients[i].receive(tempPacket);
                if(tempPacket)
                {
                    handlePackets(tempPacket, mainSession, clients[i], true);
                }
            }
        }
        sf::sleep(sf::milliseconds(100));
    }

    mainSession.closeSQL();

    return 0;
}
