#include "MySQLSession/MySQLSession.hpp"


void Concantate(std::string &source)
{
    std::string allowedChars{"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890_"};
    for(unsigned int i{0}; i < source.length(); i++)
        if(allowedChars.find(source[i]) == std::string::npos)
            source[i] = '_';
}


MySQLSession::MySQLError MySQLSession::getError()
{
    MySQLError returnValue;
    returnValue.errorno  = mysql_errno(connector);
    returnValue.errorstr = mysql_error(connector);
    if(verbose)
        std::cout<<"MySQL Error "<<std::to_string(returnValue.errorno)<<" : "<<returnValue.errorstr<<std::endl;
    return(returnValue);
}

std::string MySQLSession::runQuery(std::string queryString, bool get)
{
    if(!initialized)
        return "";

    if(mysql_query(connector, queryString.c_str()))
    {
        MySQLError error = getError();
        return std::string{"MySQL Error " + std::to_string(error.errorno) + " : " + error.errorstr};
    }
    //return "";

    if(get)
    {
        int num_fields;

        std::string returnValue;

        result = mysql_store_result(connector);

        num_fields = mysql_num_fields(result);

        while ((row = mysql_fetch_row(result)))
        {
            for(int i = 0; i < num_fields; i++)
            {
                if(verbose)
                    std::cout<<(row[i] ? row[i] : "NULL");
                returnValue += (row[i] ? row[i] : "NULL");
            }
            if(verbose)
                std::cout<<std::endl;
        }

        mysql_free_result(result);

        return returnValue;
    }

    return "";
}

bool MySQLSession::initiate(std::string Hostname_, int port_, std::string username_, std::string password_, std::string database_)
{
    Hostname = Hostname_;
    username = username_;
    password = password_;
    database = database_;
    port = port_;

    if(mysql_real_connect(connector, Hostname.c_str(), username.c_str(), password.c_str(), database.c_str(), port, NULL, 0) == NULL)
    {
        getError();
        initialized = false;
        return initialized;
    }
    initialized = true;
    return initialized;
}

void MySQLSession::closeSQL()
{
    if(!initialized)
        return;
    mysql_close(connector);
}

MySQLSession::MySQLSession(bool verbose_)
{
    verbose = verbose_;
    connector = mysql_init(NULL);

    if(connector == NULL)
        getError();
}
