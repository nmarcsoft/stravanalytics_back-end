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

    std::tuple<int, int> distance;
    std::tuple<int, int> duree;
    std::tuple<int, int> d_plus;

    std::tuple<time_t, time_t> date;
    vector<std::string> comments;

    std::ifstream file;

    vector<Json::Value> filtered;

    void sorter();

public:
    Analyzer()
    {
        this->distance = {-1, -1};
        this->duree = {-1, -1};
        this->d_plus = {-1, -1};
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
    i++;
    if (dateStr == "")
    {
        if (i == 2)
            return std::numeric_limits<time_t>::max();
        return 0;
    }

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
    
    this->distance = {root["filtre"]["distance"][0].asInt(), root["filtre"]["distance"][1].asInt()};
    this->d_plus = {root["filtre"]["d+"][0].asInt(), root["filtre"]["d+"][1].asInt()};
    this->duree = {root["filtre"]["duree"][0].asInt(), root["filtre"]["duree"][1].asInt()};

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
        int elevation = run["total_elevation_gain"].asInt();

        int distance_min, distance_max, duree_min, duree_max, d_plus_min, d_plus_max;
        distance_min = std::get<0>(this->distance);
        distance_max = std::get<1>(this->distance);
        duree_min = std::get<0>(this->duree);
        duree_max = std::get<1>(this->duree);
        d_plus_min = std::get<0>(this->d_plus);
        d_plus_max = std::get<1>(this->d_plus);

        if (distance_min != -1 && (run["distance"].asInt() / 1000) < distance_min)
        {
            isOk = false;
        }
        else if (distance_max != -1 && (run["distance"].asInt() / 1000) > distance_max)
        {
            isOk = false;
        } else if (duree_max != -1 && (run["elapsed_time"].asInt() / 60) > duree_max)
        {
            isOk = false;
        } else if (duree_min != -1 && (run["elapsed_time"].asInt() / 60) < duree_min)
        {
            isOk = false;
        } else if (time < std::get<0>(this->date))
        {
            if (time > std::get<1>(this->date))
                isOk = false;
        } else if (d_plus_min != -1 && d_plus_min > elevation)
        {
            isOk = false;
        } else if (d_plus_max != -1 && d_plus_max < elevation)
        {
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
    
    out << "Distance filter : \n\t- [" << std::get<0>(other.distance) << ";" << std::get<1>(other.distance) << "]" << endl; 
    out << "Dénivelé filter : \n\t- [" << std::get<0>(other.d_plus) << ";" << std::get<1>(other.d_plus) << "]" << endl; 
    out << "Duree filter : \n\t- [" << std::get<0>(other.duree) << ";" << std::get<1>(other.duree) << "]" << endl; 
    

    return out;
}

void Analyzer::draw_graph()
{
    
}