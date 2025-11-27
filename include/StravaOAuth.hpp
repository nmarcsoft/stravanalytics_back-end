#pragma once

#include <string>
#include <thread>
#include <atomic>
#include <optional>

#include <httplib.h>
#include <json/json.h>

class StravaOAuth {
public:
    StravaOAuth(int client_id,
                const std::string& client_secret,
                const std::string& redirect_uri);

    void authenticate();
    std::string get_access_token() const;

private:
    int client_id;
    std::string client_secret;
    std::string redirect_uri;

    std::string authorization_code;
    std::string access_token;
    std::string refresh_token;

    long expires_at = 0;

    std::thread server_thread;
    std::atomic<bool> server_running{false};

    void start_local_server();
    std::string build_auth_url() const;

    void exchange_code_for_token();
    void refresh_access_token();
    void open_authorization_page();
};