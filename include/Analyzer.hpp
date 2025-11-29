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
        if (i == 0)
            return 0;
        else
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

    //this->date = { (dateToTimestamp(root["filtre"][""][0].asString())), (dateToTimestamp(root["filtre"]["nom"][1].asString())) };

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
        }
        
        if (isOk)
        {
            for (auto & it : this->name_filter)
            {
                if ( name.find(it) != std::string::npos)
                {
                    this->filtered.push_back(it);
                }
            }
        }
    }

    return 1;
}

void Analyzer::debug()
{
    for (auto &it : this->filtered)
    {
        cout << it << endl;
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