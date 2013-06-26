#pragma once

#include <mysql/mysql.h>
#include <iostream>

void Concantate(std::string &source);

class MySQLSession
{
public:
    struct MySQLError
    {
        unsigned int errorno;
        std::string errorstr;
    };

private:
    MYSQL *connector;
    MYSQL_RES *result;
    MYSQL_ROW row;
    bool initialized, verbose;
    std::string Hostname, username, password, database;
    int port;

    MySQLError getError();

public:
    std::string runQuery(std::string queryString, bool get = false);
    bool initiate(const std::string& Hostname_, const int& port_, const std::string& username_, const std::string& password_, const std::string& database_ = "");
    void closeSQL();
    MySQLSession(bool verbose_);
};
