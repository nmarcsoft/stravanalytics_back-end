#ifndef CONNECTOR_HPP
#define CONNECTOR_HPP

#include <ctime>
#include <curl/curl.h>
#include <httplib.h>
#include <iomanip>
#include <iostream>
#include <json/json.h>
#include <sstream>
#include <string>

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
