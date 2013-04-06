## Open Hexagon server software ##

This was written using C++, SFML's network library, and MariaDB (MySQL)'s C connector.

## PACKET SPECIFICATION: ##

# 0x00: Request for score Submission #

Client to Server

packet << (uint8) packet number << (string) level << (float) difficulty << (string) username << (float) score;

Will return packet 0x10.


# 0x01: Request for scores #

Client to Server

packet << (uint8) packet number << (string) level << (float) difficulty << (string) username;

Will return packet 0x11.


# 0x10: Request for score submission response #

Server to Client

packet << (uint8) packet number << (uint8) pass;

Success if pass is 0;
Failure if pass is 1;

Excpects no response.


# 0x11: Request for scores response #

Server to Client

packet << (uint8) packet number << (uint8) pass << (string) top 8 scores in json << (string) username's score in json;

Success if pass is 0;
Failure if pass is 1;

Excpects no response.
