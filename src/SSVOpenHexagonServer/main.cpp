// Open Hexagon Server Software:
// Depends on MariaDB's C connector, SFML's network library, and Vittorio's SSVUtils
// Written and developed by Mischa Alff, and Vittorio Romeo
// (c) 2013 Mischa Alff and Vittorio Romeo

#include <iostream>
#include <algorithm>
#include <string>
#include <memory>
#include <cstring>
#include <string>
#include <cmath>
#include <list>
#include <SFML/Network.hpp>
#include <SSVUtils/SSVUtils.h>
#include "MySQLSession/MySQLSession.hpp"
#include "PacketHandler/PacketHandler.hpp"

class Client
{
public:
    bool inUse;
    sf::TcpSocket* socket;
    uint32_t timeSignature, id;

    bool operator<(const Client& a)
    {
        if(inUse != a.inUse)
            return inUse < a.inUse;

        if(timeSignature != a.timeSignature)
            return timeSignature < a.timeSignature;

        return socket->getRemoteAddress().toInteger() < a.socket->getRemoteAddress().toInteger();
    }

    bool operator>(const Client& a)
    {
        if(inUse != a.inUse)
            return inUse > a.inUse;

        if(timeSignature != a.timeSignature)
            return timeSignature > a.timeSignature;

        return socket->getRemoteAddress().toInteger() > a.socket->getRemoteAddress().toInteger();
    }

    bool operator==(const Client& a)
    {
        return socket->getRemoteAddress() == a.socket->getRemoteAddress();
    }

    Client()
    {
        socket = new sf::TcpSocket;
        inUse = false;
        timeSignature = 0;
    }

    ~Client()
    {
        delete socket;
    }
};


std::string parseArguments(const std::vector<std::string>& Arguments, unsigned int& i, const std::string flag )
{
    if(Arguments[i].find(flag) == 0)
    {
        if(Arguments.size() > i+1 && Arguments[i+1][0] != '-')
        {
            i++;
            return Arguments[i];
        }
        else if(Arguments[i].length() > 2 && Arguments[i][2] == '=')
            return Arguments[i].substr(3, Arguments[i].length() - 3);
    }
    return "";
}

int main(int argc, char** argv)
{
    //Timer that will be used to timeout sockets
    sf::Clock timer;
    timer.restart();

    //Commence argument parsing...
    std::vector<std::string> Arguments;
    for(int i {1}; i < argc; i++)
        Arguments.push_back(std::string {argv[i]});

    std::string host, user, password, database;
    unsigned int portToListenOn{65536};

    //Argument parsing loop
    for(unsigned int i = 0; i < Arguments.size(); i++)
    {
        if(host == "")
            host = parseArguments(Arguments, i, "-h");

        if(user == "")
            user = parseArguments(Arguments, i, "-u");

        if(password == "")
            password = parseArguments(Arguments, i, "-k");

        if(database == "")
            database = parseArguments(Arguments, i, "-d");

        if(portToListenOn > 49151)
            portToListenOn = std::atoi( parseArguments(Arguments, i, "-p").c_str());
    }

    //True if debug, false if release
    bool verbose {true};

    //Create a MySQL session
    MySQLSession mainSession {verbose};
    mainSession.initiate(host, 0, user, password, database);

    //Clients storage system
    std::list<Client> clients;
    clients.emplace_back();
    uint32_t idCounter{0};

    //Server listener setup
    sf::TcpListener server;
    server.setBlocking(false);
    server.listen(27272);

    if(verbose) std::cout << "MySQL client version: " << mysql_get_client_info() << " \n";

    while(true)
    {
        //Find the first available Client class
        std::list<Client>::iterator placeholderClientItr = std::find_if(clients.begin(), clients.end(), [] (Client &someClient) { return someClient.inUse == false;});

        //Verify that the iterator is valid..
        if(placeholderClientItr != clients.end())
        {
            //Accept a potential connection
            sf::Socket::Status a = server.accept(*placeholderClientItr->socket);
            //If the connection is valid..
            if(a == sf::Socket::Done)
            {
                //Check whether the specified IP is already connected to the server
                bool IPExists = false;
                for(auto &it : clients)
                {
                    if((*placeholderClientItr == it) && ((*placeholderClientItr).id != it.id))
                    {
                        std::cout<<"Lololoadajs;dkja\n";
                        IPExists = true;
                        placeholderClientItr->socket->disconnect();
                        break;
                    }
                }
                //If not, continue normally!
                if(!IPExists)
                {
                    placeholderClientItr->inUse = true;
                    placeholderClientItr->socket->setBlocking(false);
                    placeholderClientItr->timeSignature = timer.getElapsedTime().asSeconds();
                    //If no new available client classes exist, create a new one to create room
                    if(std::find_if(clients.begin(), clients.end(), [] (Client &someClient) { return someClient.inUse == false;}) == clients.end())
                    {
                        clients.emplace_back();
                        clients.back().id = ++idCounter;
                    }
                    clients.sort();
                }
            }
        }

        for(auto& it : clients)
        {
            if(!it.inUse) continue;

            //If the client hasn't done anything in 15 seconds, disconnect.
            if(it.timeSignature < timer.getElapsedTime().asSeconds()-15)
            {
                it.socket->disconnect();
                it.inUse = false;
                it.timeSignature = 0;
                it.id = 0;
                continue;
            }

            //Else, check whether we have a packet to receive!
            sf::Packet tempPacket;
            it.socket->receive(tempPacket);
            //Process the packet..
            if(tempPacket && handlePackets(tempPacket, mainSession, (*it.socket), verbose))
            {
                it.socket->disconnect();
                it.inUse = false;
            }
        }
        if(verbose)std::cout<<clients.size();
        //Sleep to prevent 100% CPU usage
        sf::sleep(sf::milliseconds(30));
    }

    //Close the MySQL session
    mainSession.closeSQL();
    return 0;
}
