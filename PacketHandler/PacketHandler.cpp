#include <iostream>
#include <string>
#include <SFML/Network.hpp>
#include <SSVUtils/SSVUtils.h>
#include "MySQLSession/MySQLSession.hpp"
#include "PacketHandler/PacketHandler.hpp"


std::string getMD5Hash(const std::string& mString)
{
    ssvu::MD5 key{mString};
    return key.GetHash();
}

sf::Packet buildPacket0x10( uint8_t pass )
{
    sf::Packet returnValue;
    returnValue << uint8_t{0x10} << pass;
    return returnValue;
}

sf::Packet buildPacket0x11( uint8_t pass, std::string topScoresJson, std::string userScoreJson )
{
    sf::Packet returnValue;
    returnValue << uint8_t{0x11} << pass << topScoresJson << userScoreJson;
    return returnValue;
}

bool handlePacket0x00(sf::Packet &packet, MySQLSession &session, sf::TcpSocket &socket, bool verbose)
{
    // This packet handles a score submission.
    // order: level, difficulty, username, score
    std::string level, username, md5hash;
    float difficulty, score;

    if(packet >> level >> difficulty >> username >> score >> md5hash)
    {

        if( HG_ENCRYPTIONKEY != md5hash)
        {
            if(verbose)
                std::cout<<"Hash comparison failed."<<std::endl;
            sf::Packet response = buildPacket0x10(1);
            socket.send(response);
            return false;
        }

        else
        {
            if(verbose)
                std::cout<<"Hash comparison succeeded! Continuing.."<<std::endl;
        }

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


        std::string query {"CREATE TABLE IF NOT EXISTS " + level + "(user_name VARCHAR(64) NOT NULL, difficulty DECIMAL(5,2) NOT NULL, score DECIMAL(9,3), PRIMARY KEY ( user_name, difficulty ) );"};
        if(verbose) std::cout<<"Running query: "<<query<<std::endl;
        session.runQuery(query);
        query.clear();


        query = ("INSERT INTO " + level + " ( user_name, difficulty, score) VALUES (\"" + username + "\", " + std::to_string(difficulty) + ", " + std::to_string(score) + ");");
        if(verbose) std::cout<<"Running query: "<<query<<std::endl;
        session.runQuery(query);
        query.clear();

        query = ("UPDATE " + level + " SET score = " + std::to_string(score) + " WHERE user_name=\"" + username + "\" AND difficulty=" + std::to_string(difficulty) + " AND score<" + std::to_string(score) + ";");
        if(verbose) std::cout<<"Running query: "<<query<<std::endl;
        session.runQuery(query);

        sf::Packet response = buildPacket0x10(0);
        socket.send(response);

        return true;
    }

    else
    {
        if(verbose)
            std::cout<<"Error, packet 0x00 did not extract successfully."<<std::endl;

        sf::Packet response = buildPacket0x10(1);
        socket.send(response);

        return false;
    }

}

bool handlePacket0x01(sf::Packet &packet, MySQLSession &session, sf::TcpSocket &socket, bool verbose)
{
    std::string level, username;
    float difficulty;

    if(packet >> level >> difficulty >> username)
    {
        Concantate(level);
        Concantate(username);

        if(verbose)
        {
            std::cout<<"Packet extracted successfully!"<<std::endl;
            std::cout<<"Level name: "<<level<<std::endl
                     <<"Difficulty: "<<difficulty<<std::endl
                     <<"Username: "<<username<<std::endl;
        }

        std::string query =

R"(SELECT CONCAT("[",
    GROUP_CONCAT(
        CONCAT("{\"p\":", row_number, ","),
        CONCAT("\"n\":\"", user_name, "\""),
        CONCAT(",\"s\":",score,"}")
    ),
    "]"
)
AS json FROM (
    SELECT score, user_name, @curRow := @curRow + 1 AS row_number
    FROM )" + level + R"( JOIN(SELECT @curRow := 0) r WHERE difficulty = )" + std::to_string(difficulty) + R"(
    ORDER BY score DESC LIMIT 8
) AS foo;)";

        //std::cout<<query<<std::endl;

        std::string topScoresJson{session.runQuery(query, true)};

        query.clear();

        query =
R"(SELECT CONCAT("\"ppos\":",
    GROUP_CONCAT(
        CONCAT("{\"p\":", row_number, ","),
        CONCAT("\"n\":\"", user_name, "\""),
        CONCAT(",\"s\":",score,"}")
    )
) AS json FROM (
    SELECT score, user_name, row_number
    FROM (
        SELECT score, user_name, @curRow := @curRow + 1 AS row_number
        FROM )" + level + R"( JOIN(SELECT @curRow := 0) r
        WHERE difficulty = )" + std::to_string(difficulty) + R"(
        ORDER BY score DESC
    ) AS derp
    WHERE user_name = ")" + username + R"("
) AS foo;)";

        if(verbose)
            std::cout<<query<<std::endl;

        std::string userScoreJson{session.runQuery(query, true)};

        sf::Packet response = buildPacket0x11(0, topScoresJson, userScoreJson);

        socket.send(response);
        return true;

    }
    else
    {
        if(verbose)
            std::cout<<"Error: packet 0x01 did not extract successfully."<<std::endl;

        sf::Packet response = buildPacket0x11(1, "", "");
        socket.send(response);

        return false;
    }
    return false;
}

bool handlePackets(sf::Packet packet, MySQLSession &session, sf::TcpSocket &socket, bool verbose)
{
    int8_t packetIdentifier;
    if(packet >> packetIdentifier)
    {
        switch(packetIdentifier)
        {
        case 0x00:
            return handlePacket0x00(packet, session, socket, verbose);
            break;

        case 0x01:
            return handlePacket0x01(packet, session, socket, verbose);
            break;
        }
    }
    return true;
}
