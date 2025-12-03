#ifndef CONNECTOR_HPP
#define CONNECTOR_HPP

#include <iostream>
#include <string>
#include <curl/curl.h>
#include <json/json.h>
#include <httplib.h>
#include <ctime>
#include <iomanip>
#include <sstream>

#include "Logger.hpp"
#include "csvParser.hpp"

class ConnectionManager {
    private:
        std::string access_token;
        std::string url;
    
    public:
        int send_request();
        ConnectionManager(std::string access_token);
        void execute();
};



#endif // CONNECTOR_HPP