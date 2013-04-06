#include <SFML/Network.hpp>
#include <iostream>

int main()
{
	sf::TcpSocket someSocket;
	sf::Packet packet0x00, packet0x01;


    packet0x00 << int8_t{0x00} << std::string{"testScores"} << float{1.f} << std::string{"penis"} << float{123.456};
	packet0x01 << int8_t{0x01} << std::string{"testScores"} << float{1.f} << std::string{"penis"};
	
	someSocket.setBlocking(true);
	someSocket.connect("localhost", 27272);

	someSocket.send(packet0x00);
	someSocket.send(packet0x01);
	
	someSocket.receive( packet0x01 );
	
	std::string response[2];
	
	if(packet0x01 >> response[0] >> response[1])
	{
		std::cout<<response[0]<<std::endl;
		std::cout<<response[1]<<std::endl;
	}

	//sf::sleep(sf::seconds(5));
}
/*#include <SFML/Network.hpp>
int main()
{
	sf::TcpSocket someSocket;
	someSocket.setBlocking(true);
	sf::Packet packet0x00, packet0x01;
	packet0x00 << int8_t{0} << std::string{"testScores"} << int{1} << std::string{"penis"} << float{123.456};
	packet0x00 << int8_t{1} << std::string{"testScores"} << int{1} << std::string{"penis"};
	someSocket.connect("127.0.0.1", 27272);
	someSocket.send(packet0x00);
	someSocket.send(packet0x01);
}*/
