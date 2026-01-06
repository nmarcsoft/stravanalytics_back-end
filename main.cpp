#include "Logger.hpp"
#include "StravaAuthController.hpp"
#include <httplib.h>

int main(void) {
  logger.log(INFO, "Program Started.");

  std::string client_id = getenv("CLIENT_ID");
  std::string client_secret = getenv("CLIENT_SECRET");

  httplib::Server server;

  StravaAuthController auth(stoi(client_id), client_secret,
                            "http://localhost:8080/auth/callback");

  auth.register_routes(server);
  server.listen("localhost", 8080);
  logger.log(WARNING, "[MAIN] : Stop listening");
  return 0;
  ;
}
