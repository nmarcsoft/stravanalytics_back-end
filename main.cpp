#include "Analyzer.hpp"
#include "Connector.hpp"
#include "Logger.hpp"
#include "StravaOAuth.hpp"
#include "csvParser.hpp"

int main(void) {
  logger.log(INFO, "Program Started.");

  // Récupération de l'acess_token
  Parser test = Parser("../var.csv");

  std::string client_id = test.parse("CLIENT_ID");
  std::string client_secret = test.parse("CLIENT_SECRET");
  StravaOAuth oauth(stoi(client_id), client_secret,
                    "http://localhost:8080/callback");

  oauth.authenticate();
  std::string access_token = oauth.get_access_token();

  // logger.log(DEBUG, "Acess_token = " + access_token);

  // Connection à strava
  ConnectionManager connection_manager(access_token);
  connection_manager.execute();

  Analyzer analyzer;

  analyzer.set_up();
  analyzer.extract();

  analyzer.draw_graph();

  return 0;
}
