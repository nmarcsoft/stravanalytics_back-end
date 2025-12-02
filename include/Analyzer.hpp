#pragma once

#include <iostream>
#include <string>
#include <Logger.hpp>
#include <fstream>
#include <json/json.h>
#include <tuple>
#include <ctime>
#include <limits>

class Analyzer {
private:
    vector<std::string> name_filter;

    int distance_min;
    int distance_max;
    int duree_max;
    int duree_min;
    int d_plus;

    std::tuple<time_t, time_t> date;
    vector<std::string> comments;

    std::ifstream file;

    vector<Json::Value> filtered;

    void sorter();

public:
    Analyzer()
    {
        this->distance_min = -1;
        this->distance_max = -1;
        this->duree_min = -1;
        this->duree_max = -1;
        this->d_plus = -1;
        this->date = {0, std::numeric_limits<time_t>::max()};
        this->name_filter = vector<std::string>();
        this->filtered = vector<Json::Value>();
        file.open("../command.json");
        if (!file.is_open()) {
            throw std::runtime_error("Impossible d'ouvrir le fichier : command.json");
        }
    }

    ~Analyzer() {
        if (file.is_open()) {
            file.close();
        }
    }

    int set_up();
    int extract();
    void draw_graph();
    void debug();

    

    friend std::ostream& operator<<(std::ostream& out, const Analyzer& other);

};

static int i = 0;
time_t dateToTimestamp(const std::string& dateStr) {

    if (dateStr == "")
    {
        if (i == 1)
            return 0;
        return std::numeric_limits<time_t>::max();
    }
    i++;
    std::tm tm = {};
    std::istringstream ss(dateStr);

    ss >> std::get_time(&tm, "%d/%m/%Y");

    if (ss.fail()) {
        throw std::runtime_error("Format de date invalide");
    }

    tm.tm_hour = 0;
    tm.tm_min  = 0;
    tm.tm_sec  = 0;

    return std::mktime(&tm);
}

time_t iso8601_to_timestamp(const std::string& iso8601)
{
    std::tm tm = {};
    std::istringstream ss(iso8601);

    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    if (ss.fail()) {
        throw std::runtime_error("Erreur de parsing de la date !");
    }

    // Convertit en timestamp UTC
    return timegm(&tm);  // IMPORTANT : timegm() = gmtime inverse
}

int Analyzer::set_up() {
    
    if (!this->file.is_open()) {
        std::cerr << "Impossible d’ouvrir le fichier JSON !" << std::endl;
        return 1;
    }

    Json::Value root;
    Json::CharReaderBuilder builder;
    JSONCPP_STRING errs;

    if (!parseFromStream(builder, this->file, &root, &errs)) {
        std::cerr << "Erreur JSON : " << errs << std::endl;
        return 1;
    }

    this->distance_min = root["filtre"]["distance_min"].asInt();
    this->distance_max = root["filtre"]["distance_max"].asInt();
    this->d_plus = root["filtre"]["d+"].asInt();
    this->duree_min = root["filtre"]["duree_min"].asInt();
    this->duree_max = root["filtre"]["duree_max"].asInt();

    time_t date_min = dateToTimestamp(root["filtre"]["date"][0].asString());
    time_t date_max = dateToTimestamp(root["filtre"]["date"][1].asString());

    this->date = { date_min, date_max };

    for (auto &it : root["filtre"]["nom"])
    {
        name_filter.push_back(it.asString());
    }

    return 0;
}

int Analyzer::extract()
{
    std::ifstream input("strava_response.json");

    if (!input.is_open()) {
        std::cerr << "Impossible d’ouvrir le fichier JSON !" << std::endl;
        return 1;
    }

    Json::Value root;
    Json::CharReaderBuilder builder;
    JSONCPP_STRING errs;

    if (!parseFromStream(builder, input, &root, &errs)) {
        std::cerr << "Erreur JSON : " << errs << std::endl;
        return 1;
    }

    
    bool isOk = true;

    for (auto &run : root)
    {
        isOk = true;
        std::string name = run["name"].asString();
        int distance_run = run["distance"].asInt();
        time_t time = iso8601_to_timestamp(run["start_date"].asString());

        if (this->distance_min != -1 && (run["distance"].asInt() / 1000) < this->distance_min)
        {
            isOk = false;
        }
        else if (this->distance_max != -1 && (run["distance"].asInt() / 1000) > this->distance_max)
        {
            isOk = false;
        } else if (this->duree_max != -1 && (run["elapsed_time"].asInt() / 60) > this->duree_max)
        {
            isOk = false;
        } else if (this->duree_min != -1 && (run["elapsed_time"].asInt() / 60) < this->duree_min)
        {
            isOk = false;
        } else if (time < std::get<0>(this->date))
        {
            if (time > std::get<1>(this->date))
                isOk = false;
        }
        
        if (isOk)
        {
            for (auto & it : this->name_filter)
            {
                if ( name.find(it) != std::string::npos)
                {
                    cout << "Ajout du footing avec une date de : " << time << \
                    "comparé à [" << std::get<0>(this->date) << " : " << std::get<1>(this->date) << "]" << endl;
                    this->filtered.push_back(run);
                }
            }
        }
    }
    this->sorter();
    return 1;
}

void Analyzer::sorter()
{
    std::sort(this->filtered.begin(), this->filtered.end(),
    [](const Json::Value &a, const Json::Value &b) {
        return a["name"].asString() < b["name"].asString();
    }
);

}

void Analyzer::debug()
{
    ofstream filtered_run("output.json");
    for (auto &it : this->filtered)
    {
        cout << it << endl;
        filtered_run << it << endl;
    }
}

ostream& operator<< (ostream& out, const Analyzer& other)
{
    out << "Name filter :" << endl;
    for (auto &it : other.name_filter)
    {
        out << "\t- " << it << endl;
    }
    
    out << "Distance filter : \n\t- [" << other.distance_min << ";" << other.distance_max << "]" << endl; 
    out << "Déniveler filter : \n\t- " << other.d_plus << endl;
    out << "Duree filter : \n\t- [" << other.duree_min << ";" << other.duree_max << "]" << endl; 
    

    return out;
}

void Analyzer::draw_graph()
{
    
}