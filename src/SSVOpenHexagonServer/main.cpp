// Open Hexagon Server Software:
// Depends on MariaDB's C connector, SFML's network library, and Vittorio's SSVUtils
// Written and developed by Mischa Alff, and Vittorio Romeo
// (c) 2013 Mischa Alff and Vittorio Romeo

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
	std::string host, user, password, database;
	int portToListenOn{27272};

	for(int i{0}; i < argc; i++)
	{
		// TODO: refactor all this stuff

		if(std::string{argv[i]}.find("-h") == 0)
		{
			std::string arg{argv[i]};
			if(argc > i+1 && argv[i+1][0] != '-')
			{
				host = argv[i+1];
				i++;
			}
			else if(strlen(argv[i]) > 2 && argv[i][2] == '=')
				host = std::string{argv[i]}.substr(3, std::string{argv[i]}.length()-3);
		}

		else if(std::string{argv[i]}.find("-u") == 0)
		{
			if(argc > i+1 && argv[i+1][0] != '-')
			{
				user = argv[i+1];
				i++;
			}
			else if(strlen(argv[i]) > 2 && argv[i][2] == '=')
				user = std::string{argv[i]}.substr(3, std::string{argv[i]}.length()-3);
		}

		else if(std::string{argv[i]}.find("-k") == 0)
		{
			if(argc > i+1 && argv[i+1][0] != '-')
			{
				password = argv[i+1];
				i++;
			}
			else if(strlen(argv[i]) > 2 && argv[i][2] == '=')
				password = std::string{argv[i]}.substr(3, std::string{argv[i]}.length()-3);
		}

		else if(std::string{argv[i]}.find("-d") == 0)
		{
			if(argc > i+1 && argv[i+1][0] != '-')
			{
				database = argv[i+1];
				i++;
			}
			else if(strlen(argv[i]) > 2 && argv[i][2] == '=')
				database = std::string{argv[i]}.substr(3, std::string{argv[i]}.length()-3);
		}

		else if(std::string{argv[i]}.find("-p") == 0)
		{
			if(argc > i+1 && argv[i+1][0] != '-')
			{
				portToListenOn = atoi(argv[i+1]);
				i++;
			}
			else if(strlen(argv[i]) > 2 && argv[i][2] == '=')
				portToListenOn = atoi(std::string{std::string{argv[i]}.substr(3, std::string{argv[i]}.length()-3)}.c_str());
		}
	}

	bool verbose{true};

	MySQLSession mainSession{verbose};
	mainSession.initiate(host, 0, user, password, database);

	const unsigned int clients_size = 1000;
	sf::TcpSocket clients[clients_size];
	bool  clientAvailable[clients_size];

	std::memset(clientAvailable, true, sizeof(bool) * 1000);
	clientAvailable[1] = false;

	//clients.resize(1000);
	//clientAvailable.assign(clients_size, true);

	sf::TcpListener server;
	server.setBlocking(false);
	server.listen(portToListenOn);

	if(verbose) std::cout << "MySQL client version: " << mysql_get_client_info() << " \n";

	while(true)
	{
		unsigned int firstFreeSocket{0};
		for(; firstFreeSocket < clients_size; ++firstFreeSocket) if(clientAvailable[firstFreeSocket]) break;
		// TODO: what does this for loop do?

		// std::cout << firstFreeSocket << "," << clients.size() << std::endl;

		if(server.accept(clients[firstFreeSocket]) == sf::Socket::Done)
		{
			if(verbose) std::cout << "Received packet!" << std::endl;
			clientAvailable[firstFreeSocket] = false;
			clients[firstFreeSocket].setBlocking(false);
		}

		for(unsigned int i{0}; i < clients_size; ++i)
		{
			if(clientAvailable[i]) continue;

			sf::Packet tempPacket;
			clients[i].receive(tempPacket);
			if(tempPacket && handlePackets(tempPacket, mainSession, clients[i], verbose))
			{
				clients[i].disconnect();
				clientAvailable[i] = false;
			}
		}

		sf::sleep(sf::milliseconds(50));
	}

	mainSession.closeSQL();
	return 0;
}
