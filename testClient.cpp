#include <SFML/Network.hpp>
int main()
{
	sf::TcpSocket someSocket;
	sf::Packet somePacket;


    //if(packet >> level >> difficulty >> username >> score)
	somePacket << int8_t{0x00} << std::string{"someLevel"} << float{1.f} << std::string{"Bob"} << float{20.345};

	//somePacket << float{12.345} << std::string{"Bob"} << float{1.f} << std::string{"someLevel"} << int{'\x00'};

	someSocket.setBlocking(true);
	someSocket.connect("localhost", 27272);

	someSocket.send(somePacket);

	//sf::sleep(sf::seconds(5));
}
