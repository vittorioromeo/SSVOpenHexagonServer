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

    std::string runQuery(std::string queryString)
    {
        if(!initialized)
            return "";

        if(mysql_query(connector, queryString.c_str()))
        {
            getError();
            return "";
        }

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

bool handlePacket0x00(sf::Packet &packet, MySQLSession &session, sf::TcpSocket &socket, bool verbose)
{
    // This packet handles a score submission.
    // order: level, difficulty, username, score
    std::string level, username;
    float difficulty, score;

    if(packet >> level >> difficulty >> username >> score)
    {
        //CREATE TABLE IF NOT EXISTS testScores (user_name VARCHAR(64) NOT NULL, difficulty DECIMAL(5,2) NOT NULL, score DECIMAL(9,3), PRIMARY KEY ( user_name, difficulty ) );

        std::string query {"CREATE TABLE IF NOT EXISTS " + level + "(user_name VARCHAR(64) NOT NULL, difficulty DECIMAL(5,2) NOT NULL, score DECIMAL(9,3), PRIMARY KEY ( user_name, difficulty ) )"};

        session.runQuery(query);

        query.clear();

        //INSERT INTO  testScores ( user_name, difficulty, score) VALUES ("someone", 1, 123.456);

        query = "INSERT INTO " + level + " ( user_name, difficulty, score) VALUES (\"" + username + ", " + std::to_string(difficulty) + ", " + std::to_string(score) + ")";

        session.runQuery(query);

        query.clear();
        return 1;
    }

    else
    {
        std::cout<<"Error, packet 0x00 did not extract successfully."<<std::endl;
        return 0;
    }

}

bool handlePackets(sf::Packet packet, MySQLSession &session, sf::TcpSocket &socket, bool verbose)
{
    uint8_t packetIdentifier;
    if(packet >> packetIdentifier)
    {
        if(packetIdentifier == '\x00')
            handlePacket0x00(packet, session, socket, verbose);

    }
    return true;
}

int main(int argc, char** argv)
{
    bool verbose = true;

    MySQLSession mainSession {verbose};
    mainSession.initiate("localhost", 0, "root", "isurelydont", "oh_scores");

    std::vector</*std::shared_ptr<*/sf::TcpSocket/*>*/> clients;
    std::vector<bool>   clientAvailable;

    clients.resize(1000);
    clientAvailable.assign(clients.size(), true);

    sf::TcpListener server;
    server.setBlocking(false);
    server.listen(27272);

    std::cout<<"MySQL client version: "<<mysql_get_client_info()<<" \n";

//    for(int i = 0; i < clients.size(); i++)
//    {
//        clients[i].get() = std::make_shared<sf::TcpSocket>;
//    }

//    if(clients[0] == nullptr)
//    {
//        std::cout<<"Poop!"<<std::endl;
//    }

    while(true)
    {
        unsigned int firstFreeSocket {0};
        for(; firstFreeSocket < clients.size(); firstFreeSocket++)
            if(clientAvailable[firstFreeSocket])
                break;

        //std::cout<<firstFreeSocket<<","<<clients.size()<<std::endl;

        if(server.accept(clients[firstFreeSocket]) == sf::Socket::Done)
        {
            std::cout<<"penis"<<std::endl;
            clientAvailable[firstFreeSocket] = false;
            clients[firstFreeSocket].setBlocking(false);
        }

        for(unsigned int i = 0; i < clients.size(); i++)
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
    }

    mainSession.closeSQL();

    return 0;
}




