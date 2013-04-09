#include <iostream>
#include <string>
#include <SFML/Network.hpp>
#include <SSVUtils/SSVUtils.h>
#include "MySQLSession/MySQLSession.hpp"

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

sf::Packet buildPacket0x10( uint8_t pass );
sf::Packet buildPacket0x11( uint8_t pass, std::string topScoresJson, std::string userScoreJson );

bool handlePacket0x00(sf::Packet &packet, MySQLSession &session, sf::TcpSocket &socket, bool verbose);
bool handlePacket0x01(sf::Packet &packet, MySQLSession &session, sf::TcpSocket &socket, bool verbose);
bool handlePackets(sf::Packet packet, MySQLSession &session, sf::TcpSocket &socket, bool verbose);
