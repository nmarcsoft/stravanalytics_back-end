#include "Connector.hpp"

ConnectionManager::ConnectionManager(std::string access_token)
{
    this->access_token = access_token;
    this->url = "https://www.strava.com/api/v3/athlete/activities";
};

size_t writeCallback(void* contents, size_t size, size_t nmemb, std::string* userData)
{
    userData->append((char*)contents, size * nmemb);
    return size * nmemb;
}

int ConnectionManager::test_connection()
{
    CURL* curl = curl_easy_init();
    if (!curl) {
        logger.log(ERROR, "Impossible de creer curl");
        return 1;
    }
    
    std::string response;
    std::string authHeader = "Authorization: Bearer " + this->access_token;

    struct curl_slist* headers = nullptr;
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
        std::cout << "Réponse Strava :" << std::endl;
        std::cout << response << std::endl;
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    return 0;
}