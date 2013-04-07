#include <SFML/Network.hpp>
#include <SSVUtils/SSVUtils.h>
#include <iostream>

#ifndef HG_DEFINITIONS
#define HG_DEFINITIONS

#ifndef HG_SKEY1
#define HG_SKEY1 "dev1"
#endif

#ifndef HG_SKEY2
#define HG_SKEY2 "dev2"
#endif

#ifndef HG_SKEY3
#define HG_SKEY3 "dev3"
#endif

#ifndef HG_NKEY1
#define HG_NKEY1 123456
#endif

#ifndef HG_ENCRYPTIONKEY
#define HG_ENCRYPTIONKEY getMD5Hash(username + level + ssvu::toStr(score) + getMD5Hash(HG_SKEY1) + getMD5Hash(HG_SKEY2) + getMD5Hash(HG_SKEY3) + getMD5Hash(ssvu::toStr(HG_NKEY1)))
#endif

#endif

std::string getMD5Hash(const std::string& mString)
{
    ssvu::MD5 key{mString};
    return key.GetHash();
}


int main()
{
	bool verbose = false;
	
    //Initialize a socket
    sf::TcpSocket someSocket;
    
    //Prepare the packets
    sf::Packet packet0x00, packet0x01,
               packet0x10, packet0x11;
			   
	std::string username{"penis"}, level{"AB\"; DROP DATABASE;"};
	float score = 123.456;
			   

    //Fill the client to server packets
    packet0x00 << int8_t{0x00} << level << float{1.f} << username << score << HG_ENCRYPTIONKEY;
    packet0x01 << int8_t{0x01} << level << float{1.f} << username;
    
    //Connect to server
    someSocket.setBlocking(true);
    someSocket.connect("localhost", 27272);

    //Send client to server packet, then receive the server response
    someSocket.send(packet0x00);
    someSocket.receive(packet0x10);
    
    uint8_t packetID, pass;
    
    //parse the packet. Check packet number and if it passed or not
    if(packet0x10 >> packetID >> pass)
    {
        if(packetID == 0x10)
        {
            if(pass == 0)
				if(verbose)
					std::cout<<"Successfully submitted score!";
            
            else
				if(verbose)
					std::cout<<"Oops! Something went wrong!";
        }
    }
    
    //Server disconnects after receiving the first packet, so let's do the same
    someSocket.disconnect();
    
    //Let's connect again
    someSocket.connect("localhost", 27272);
    
    //Send/receive the packets again..
    someSocket.send(packet0x01);
    someSocket.receive( packet0x11 );
    
    //Prepare the data types for the response
    std::string response[2];
    
    //parse the packet, check for success and check packet ID
    if(packet0x11 >> packetID >> pass)
    {
        if(pass == 0)
        {
            // if success, extracting strings is safe, let's do so and print them
            if( packet0x11 >> response[0] >> response[1] )
				if(verbose)
					std::cout<<"Received scores!"<<std::endl
							<<response[0]<<std::endl
							<<response[1]<<std::endl;
                         
            else
				if(verbose)
					std::cout<<"Oops! Something went wrong with getting the scores!";
        }
        else
			if(verbose)
				std::cout<<"Oops! Something went wrong with getting the scores!";
    }
    
}