#ifndef CONNECTOR_HPP
#define CONNECTOR_HPP

#include <iostream>
#include <string>
#include <curl/curl.h>

#include "Logger.hpp"
#include "csvParser.hpp"

class ConnectionManager {
    private:
        std::string access_token;
        std::string url;
    
    public:
        int test_connection();
        ConnectionManager(std::string access_token);
};


#endif // CONNECTOR_HPP