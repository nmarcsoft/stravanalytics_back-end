
#include "StravaAuthController.hpp"
#include "Analyzer.hpp"
#include "Connector.hpp"

StravaAuthController::StravaAuthController(int client_id,
                                           const std::string client_secret,
                                           const std::string &redirect_uri,
                                           const std::string &scope)
    : client_id_(client_id), client_secret_(client_secret),
      redirect_uri_(redirect_uri), scope_(scope) {}

void StravaAuthController::exchange_code_for_token() {

  httplib::SSLClient cli("www.strava.com", 443);
  cli.set_ca_cert_path("/etc/ssl/certs/ca-certificates.crt");
  cli.enable_server_certificate_verification(true);

  httplib::Params params = {{"client_id", std::to_string(client_id_)},
                            {"client_secret", client_secret_},
                            {"code", code_},
                            {"grant_type", "authorization_code"}};

  auto res = cli.Post("/oauth/token", params);

  if (!res) {
    std::cerr << "[OAuth] Requête HTTPS échouée !" << std::endl;
    return;
  }

  Json::CharReaderBuilder builder;
  Json::Value json;
  std::string errs;

  std::istringstream iss(res->body);

  cout << "json returned by strava\n" << iss.str() << endl;

  if (!parseFromStream(builder, iss, &json, &errs)) {
    std::cerr << "[OAuth] Erreur JSON : " << errs << std::endl;
    return;
  }

  if (!json.isMember("access_token")) {
    std::cerr << "[OAuth] Erreur OAuth : pas d'access_token" << std::endl;
    return;
  }

  access_token_ = json["access_token"].asString();
  refresh_token_ = json["refresh_token"].asString();
  expires_at_ = json["expires_at"].asInt64();
}

void StravaAuthController::register_routes(httplib::Server &server) {
  server.Get("/auth/login",
             [&](const httplib::Request &, httplib::Response &res) {
               logger.log(DEBUG, "[AUTH] : In /auth/login");
               std::string url = build_authorization_url();
               res.set_redirect(url.c_str());
               logger.log(DEBUG, "[AUTH] : Done for /auth/login");
             });

  server.Get("/auth/callback",
             [&](const httplib::Request &req, httplib::Response &res) {
               if (!req.has_param("code")) {
                 res.status = 400;
                 res.set_content("Missing authorization code", "text/plain");
                 return;
               }

               std::string code = req.get_param_value("code");
               this->code_ = code;

               // TODO (étape suivante) :
               exchange_code_for_token();
               ConnectionManager Connector(this->access_token_);
               Connector.execute();

               analyzer.set_up();
               analyzer.extract();
               // analyzer.debug();

               // Pour l'instant, redirection vers Vue
               res.set_redirect("http://localhost:5173/dashboard");
             });

  server.Get(
      "/api/activities", [&](const httplib::Request &, httplib::Response &res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET, OPTIONS");
        res.set_header("Access-Control-Allow-Headers",
                       "Content-Type, Authorization");
        Json::Value root(Json::arrayValue);

        Json::Value simplified;

        for (const auto &activity : analyzer.get_filtered()) {
          simplified["average_speed"] =
              analyzer.speed_to_pace(activity["average_speed"].asDouble());
          simplified["name"] = activity["name"];
          simplified["start_date"] = activity["start_date"];
          simplified["average_heartrate"] = activity["average_heartrate"];
          root.append(simplified);
        }

        Json::StreamWriterBuilder writer;
        std::string json_str = Json::writeString(writer, root);

        res.set_content(json_str, "application/json");
      });
}

std::string StravaAuthController::build_authorization_url() const {
  return "https://www.strava.com/oauth/authorize"
         "?client_id=" +
         std::to_string(client_id_) +
         "&response_type=code"
         "&redirect_uri=" +
         redirect_uri_ + "&scope=" + scope_;
}
