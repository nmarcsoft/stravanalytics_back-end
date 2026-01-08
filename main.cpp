#include "Logger.hpp"
#include "StravaAuthController.hpp"
#include <httplib.h>

/***********************************
 *  Entry point of the program
 *
 * @param No parameters
 ***********************************/
int main(void) {
  logger.log(INFO, "[MAIN] : Program Started.");

  std::string client_id = getenv("CLIENT_ID");
  std::string client_secret = getenv("CLIENT_SECRET");
  const char *redirect_env = getenv("REDIRECT_URI");
  std::string redirect_uri =
      redirect_env ? redirect_env : "http://localhost:8080/auth/callback";
  std::string host_url = redirect_env ? "0.0.0.0" : "localhost";

  httplib::Server server;

  StravaAuthController auth(stoi(client_id), client_secret, redirect_uri);

  auth.register_routes(server);
  server.listen(host_url, 8080);
  logger.log(WARNING, "[MAIN] : Stop listening");
  return 0;
  ;
}
