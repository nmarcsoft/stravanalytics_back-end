
#include "StravaOAuth.hpp"
#include <chrono>
#include <iostream>

StravaOAuth::StravaOAuth(int client_id, const std::string &client_secret,
                         const std::string &redirect_uri)
    : client_id(client_id), client_secret(client_secret),
      redirect_uri(redirect_uri) {}

std::string StravaOAuth::build_auth_url() const {
  return "https://www.strava.com/oauth/authorize"
         "?client_id=" +
         std::to_string(client_id) +
         "&response_type=code"
         "&redirect_uri=" +
         redirect_uri + "&scope=activity:read_all";
}

void StravaOAuth::start_local_server() {
  std::cout << "[OAuth] Thread serveur démarré" << std::endl;

  httplib::Server svr;

  svr.Get(
      "/callback", [&](const httplib::Request &req, httplib::Response &res) {
        if (req.has_param("code")) {
          authorization_code = req.get_param_value("code");
          server_running = false;

          std::cout << "[OAuth] Code reçu dans thread : " << authorization_code
                    << std::endl;

          res.set_content("<h2>Fermer cette page pour continuer</h2>",
                          "text/html"); // TODO : à voir si possible de faire le
                                        // reste sur page web

          svr.stop(); // <-- STOP !
        }
      });

  svr.listen("localhost", 8080);

  std::cout << "[OAuth] Fin du thread serveur" << std::endl;
}

void StravaOAuth::open_authorization_page() {
  std::string url = "https://www.strava.com/oauth/authorize?"
                    "client_id=" +
                    std::to_string(client_id) +
                    "&response_type=code"
                    "&redirect_uri=" +
                    redirect_uri + "&scope=activity:read_all";

  std::cout << "[OAuth] Ouverture du navigateur sur : " << url << std::endl;

#if defined(_WIN32)
  std::string cmd = "start " + url;
#elif defined(__APPLE__)
  std::string cmd = "open \"" + url + "\"";
#else
  // Linux : xdg-open (Arch Linux aussi)
  std::string cmd = "xdg-open \"" + url + "\"";
#endif

  system(cmd.c_str());
}

void StravaOAuth::authenticate() {
  std::cout << "[OAuth] Lancement du serveur OAuth..." << std::endl;

  // Lancement du serveur dans un thread
  std::thread server_thread(&StravaOAuth::start_local_server, this);

  // Ouvre le navigateur pour que l'utilisateur autorise l'application
  open_authorization_page();

  std::cout << "[OAuth] En attente de la fin du serveur..." << std::endl;

  server_thread.join(); // <-- Cette ligne doit revenir maintenant

  std::cout << "[OAuth] Code final = " << authorization_code << std::endl;

  // Échange le code contre un token
  exchange_code_for_token();
}

void StravaOAuth::exchange_code_for_token() {
  std::cout << "[OAuth] Tentative d’échange du code..." << std::endl;
  std::cout << "[OAuth] Code envoyé = " << authorization_code << std::endl;

  httplib::SSLClient cli("www.strava.com", 443);
  cli.set_ca_cert_path("/etc/ssl/certs/ca-certificates.crt");
  cli.enable_server_certificate_verification(true);

  httplib::Params params = {{"client_id", std::to_string(client_id)},
                            {"client_secret", client_secret},
                            {"code", authorization_code},
                            {"grant_type", "authorization_code"}};

  auto res = cli.Post("/oauth/token", params);

  if (!res) {
    std::cerr << "[OAuth] Requête HTTPS échouée !" << std::endl;
    return;
  }

  std::cout << "[OAuth] Réponse brute : " << res->body << std::endl;

  Json::CharReaderBuilder builder;
  Json::Value json;
  std::string errs;

  std::istringstream iss(res->body);

  if (!parseFromStream(builder, iss, &json, &errs)) {
    std::cerr << "[OAuth] Erreur JSON : " << errs << std::endl;
    return;
  }

  if (!json.isMember("access_token")) {
    std::cerr << "[OAuth] Erreur OAuth : pas d'access_token" << std::endl;
    return;
  }

  access_token = json["access_token"].asString();
  refresh_token = json["refresh_token"].asString();
  expires_at = json["expires_at"].asInt64();

  std::cout << "[OAuth] Nouveau access_token obtenu !" << std::endl;
}

void StravaOAuth::refresh_access_token() {
  if (std::time(nullptr) < expires_at - 60)
    return; // encore valide

  httplib::Client cli("https://www.strava.com");

  httplib::Params params = {{"client_id", std::to_string(client_id)},
                            {"client_secret", client_secret},
                            {"refresh_token", refresh_token},
                            {"grant_type", "refresh_token"}};

  auto res = cli.Post("/oauth/token", params);

  if (!res || res->status != 200) {
    std::cerr << "[OAuth] Erreur refresh token !" << std::endl;
    return;
  }

  Json::Value json;
  Json::CharReaderBuilder builder;
  std::string errs;
  std::istringstream iss(res->body);

  parseFromStream(builder, iss, &json, &errs);

  access_token = json["access_token"].asString();
  refresh_token = json["refresh_token"].asString();
  expires_at = json["expires_at"].asInt64();

  std::cout << "[OAuth] Token rafraîchi." << std::endl;
}

std::string StravaOAuth::get_access_token() const { return access_token; }
