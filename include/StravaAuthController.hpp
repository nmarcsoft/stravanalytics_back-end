#pragma once

#include <httplib.h>
#include <string>

#include "Logger.hpp"
#include <json/json.h>

class StravaAuthController {
public:
  StravaAuthController(int client_id, const std::string client_secret,
                       const std::string &redirect_uri,
                       const std::string &scope = "activity:read_all");

  void register_routes(httplib::Server &server);

private:
  int client_id_;
  std::string redirect_uri_;
  std::string scope_;

  std::string code_;
  std::string client_secret_;

  std::string access_token_;
  std::string refresh_token_;
  int expires_at_;

  std::string build_authorization_url() const;
  void exchange_code_for_token();
};
