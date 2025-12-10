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
  headers = curl_slist_append(headers, authHeader.c_str());

  logger.log(DEBUG, "url" + url);

  curl_easy_setopt(curl, CURLOPT_URL, this->url.c_str());
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

  CURLcode res = curl_easy_perform(curl);

  if (res != CURLE_OK) {
    std::cerr << "Erreur cURL : " << curl_easy_strerror(res) << std::endl;
  } else {
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

time_t string_date_to_timestamp(const std::string &date) {
  std::tm tm = {};
  std::istringstream ss(date);

  ss >> std::get_time(&tm, "%d/%m/%Y");
  if (ss.fail()) {
    throw std::runtime_error("Format invalide: " + date);
  }

  time_t toReturn = std::mktime(&tm);
  return toReturn;
}

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
    this->url = "https://www.strava.com/api/v3/activities";
    if (command["activities"]["id"] != "") {
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
    this->url.append("?per_page=200");

    std::string before, after;

    if (command.isMember("filtre")) {
      if (command["filtre"].isMember("date")) {
        before = command["filtre"]["date"][1].asString();
        after = command["filtre"]["date"][0].asString();
      }
    }

    if (before != "") {
      std::stringstream converter_before;
      converter_before << string_date_to_timestamp(before);
      this->url.append("&before=" + converter_before.str());
    }

    if (after != "") {
      std::stringstream converter_after;
      converter_after << string_date_to_timestamp(after);
      this->url.append("&after=" + converter_after.str());
    }

    send_request();
  }

  if (command.isMember("athlete")) {
    this->url = "https://www.strava.com/api/v3/athlete";
    if (command["athlete"]["activities"] != "") {
      this->url.append("/activities");
    }
    send_request();
  }
}
