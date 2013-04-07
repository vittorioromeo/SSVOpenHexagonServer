/*
 * Open Hexagon Server Software:
 * Depends on MariaDB's C connector, SFML's network library, and Vittorio's SSVUtils
 * Written and developed by Mischa Alff, and Vittorio Romeo
 * (c) 2013 Mischa Alff and Vittorio Romeo
 */

#include <iostream>
#include <string>
#include <memory>
#include <cstring>
#include <SFML/Network.hpp>
#include <SSVUtils/SSVUtils.h>
#include "MySQLSession/MySQLSession.hpp"
#include "PacketHandler/PacketHandler.hpp"

int main(int argc, char** argv)
{
    bool verbose = true;

    MySQLSession mainSession {verbose};
    mainSession.initiate("lolhost", 0, "loluser", "lolpassword", "loldatabase");

    //std::vector</*std::shared_ptr<*/sf::TcpSocket/*>*/> clients;
    const unsigned int clients_size = 1000;
    sf::TcpSocket clients[clients_size];
    bool  clientAvailable[clients_size];

    std::memset(clientAvailable, true, sizeof(bool) * 1000);
    clientAvailable[1] = false;

    //clients.resize(1000);
//    clientAvailable.assign(clients_size, true);

    sf::TcpListener server;
    server.setBlocking(false);
    server.listen(27272);

    if(verbose)
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
            if(verbose)
                std::cout<<"Received packet!"<<std::endl;
            clientAvailable[firstFreeSocket] = false;
            clients[firstFreeSocket].setBlocking(false);
        }

        for(unsigned int i = 0; i < clients_size; i++)
        {
            if(!clientAvailable[i])
            {
                sf::Packet tempPacket;
                clients[i].receive(tempPacket);
                if(tempPacket)
                {
                    if(handlePackets(tempPacket, mainSession, clients[i], verbose))
                    {
                        clients[i].disconnect();
                        clientAvailable[i] = false;
                    }
                }
            }
        }
        sf::sleep(sf::milliseconds(50));
    }

    mainSession.closeSQL();

    return 0;
}
