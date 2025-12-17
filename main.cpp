#include "Logger.hpp"
#include "StravaAuthController.hpp"
#include "StravaOAuth.hpp"
#include "csvParser.hpp"
#include <httplib.h>

int main(void) {
  logger.log(INFO, "Program Started.");

  // Récupération de l'acess_token
  Parser parser = Parser("var.csv");

  std::string client_id = parser.parse("CLIENT_ID");
  std::string client_secret = parser.parse("CLIENT_SECRET");
  // StravaOAuth oauth(stoi(client_id), client_secret,
  //                  "http://localhost:8080/callback");

  // oauth.authenticate();
  // std::string access_token = oauth.get_access_token();

  // logger.log(DEBUG, "Acess_token = " + access_token);

  // Connection à strava

  httplib::Server server;

  StravaAuthController auth(stoi(client_id), client_secret,
                            "http://localhost:8080/auth/callback");
  auth.register_routes(server);
  server.listen("localhost", 8080);
  logger.log(WARNING, "[MAIN] : Stop listening");
  return 0;
  ;
}
