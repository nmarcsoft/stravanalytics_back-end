
#include "StravaOAuth.hpp"
#include <iostream>
#include <chrono>

StravaOAuth::StravaOAuth(int client_id,
                         const std::string& client_secret,
                         const std::string& redirect_uri)
    : client_id(client_id),
      client_secret(client_secret),
      redirect_uri(redirect_uri)
{}


std::string StravaOAuth::build_auth_url() const {
    return "https://www.strava.com/oauth/authorize"
           "?client_id=" + std::to_string(client_id) +
           "&response_type=code"
           "&redirect_uri=" + redirect_uri +
           "&scope=activity:read_all";
}


void StravaOAuth::start_local_server()
{
    httplib::Server svr;

    svr.Get("/callback", [&](const httplib::Request& req, httplib::Response& res) {
        if (req.has_param("code")) {
            authorization_code = req.get_param_value("code");
            server_running = false;

            res.set_content(
                "<h2>Autorisation réussie</h2><p>Vous pouvez fermer cette fenêtre.</p>",
                "text/html"
            );

            std::cout << "[OAuth] Code reçu : " << authorization_code << std::endl;
        }
    });

    while (server_running) {
        svr.listen("localhost", 8080);
    }
}

void StravaOAuth::authenticate()
{
    authorization_code.clear();
    server_running = true;
    server_thread = std::thread(&StravaOAuth::start_local_server, this);

    std::string url = build_auth_url();
    
    std::string cmd = "xdg-open \"" + url + "\""; // Linux / Arch
    system(cmd.c_str());

    while (authorization_code.empty()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // W8er
    }

    if (server_thread.joinable())
        server_thread.join();

    exchange_code_for_token();
}

void StravaOAuth::exchange_code_for_token()
{
    httplib::Client cli("https://www.strava.com");

    httplib::Params params = {
        {"client_id",      std::to_string(client_id)},
        {"client_secret",  client_secret},
        {"code",           authorization_code},
        {"grant_type",     "authorization_code"}
    };

    auto res = cli.Post("/oauth/token", params);

    if (!res || res->status != 200) {
        std::cerr << "[OAuth] Erreur d’échange token !" << std::endl;
        return;
    }

    Json::Value json;
    Json::CharReaderBuilder builder;
    std::string errs;
    std::istringstream iss(res->body);

    if (!parseFromStream(builder, iss, &json, &errs)) {
        std::cerr << "[OAuth] JSON invalide !" << std::endl;
        return;
    }

    access_token = json["access_token"].asString();
    refresh_token = json["refresh_token"].asString();
    expires_at = json["expires_at"].asInt64();

    std::cout << "[OAuth] Nouveau access_token obtenu !" << std::endl;
}

void StravaOAuth::refresh_access_token()
{
    if (std::time(nullptr) < expires_at - 60)
        return;  // encore valide

    httplib::Client cli("https://www.strava.com");

    httplib::Params params = {
        {"client_id",      std::to_string(client_id)},
        {"client_secret",  client_secret},
        {"refresh_token",  refresh_token},
        {"grant_type",     "refresh_token"}
    };

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

std::string StravaOAuth::get_access_token() const
{
    return access_token;
}