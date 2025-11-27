#include "Connector.hpp"

ConnectionManager::ConnectionManager(std::string access_token) {
  this->access_token = access_token;
  this->url = "";
};

static size_t writeCallback(void *contents, size_t size, size_t nmemb,
                            void *userp) {
  size_t totalSize = size * nmemb;
  std::string *str = (std::string *)userp;
  str->append((char *)contents, totalSize);
  return totalSize;
}

int ConnectionManager::send_request() {
  CURL *curl = curl_easy_init();
  if (!curl) {
    logger.log(ERROR, "Impossible de creer curl");
    return 1;
  }

  std::string response;
  std::string authHeader = "Authorization: Bearer " + this->access_token;

  struct curl_slist *headers = nullptr;
  logger.log(DEBUG, this->access_token);
  headers = curl_slist_append(headers, authHeader.c_str());

  curl_easy_setopt(curl, CURLOPT_URL, this->url.c_str());
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

  CURLcode res = curl_easy_perform(curl);

  if (res != CURLE_OK) {
    std::cerr << "Erreur cURL : " << curl_easy_strerror(res) << std::endl;
  } else {
    logger.log(DEBUG, "url = " + url);
    logger.log(DEBUG, "response = " + response);
    std::ofstream outfile("strava_response.json");
    if (!outfile) {
      std::cerr << "Erreur : impossible de créer strava_response.json\n";
    } else {
      outfile << response;
      outfile.close();
      std::cout << "Réponse écrite dans strava_response.json" << std::endl;
    }
  }

  curl_slist_free_all(headers);
  curl_easy_cleanup(curl);

  return 0;
}

void log(ofstream output_file, std::string to_print) {}

void ConnectionManager::execute() {
  std::ifstream command_file("../command.json", std::ifstream::binary);

  Json::Value command;
  Json::CharReaderBuilder readerBuilder;
  std::string errors;

  bool parsingSuccessful =
      Json::parseFromStream(readerBuilder, command_file, &command, &errors);
  if (!parsingSuccessful) {
    std::cerr << "Erreur de parsing JSON : " << errors << std::endl;
    return;
  }

  if (command.isMember("activities")) {
    this->url = "https://www.strava.com/api/v3/activities"; // On veut récupérer
                                                            // une activitée
    if (command["activities"]["id"] !=
        "") // On veut récupérer quelque chose sur une activités précise
    {
      this->url.append("/");
      this->url = this->url.append(command["activities"]["id"].asString());

      if (command["activities"]["comments"] !=
          "") // On veut récupérer les commentaires
      {
        this->url.append("/comments");
      } else if (command["activities"]["kudos"] != "") {
        this->url.append("/kudos");
      } else if (command["activities"]["laps"] != "") {
        this->url.append("/laps");
      }
    }
    send_request();
  }

  if (command.isMember("athlete")) {
    this->url = "https://www.strava.com/api/v3/athlete"; // On veut récupérer
                                                         // une activitée
    if (command["athlete"]["activities"] != "") {
      this->url.append("/activities");
    }
    send_request();
  }
}
