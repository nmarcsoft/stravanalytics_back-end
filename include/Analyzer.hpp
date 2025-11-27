#pragma once

#include <iostream>
#include <string>
#include <Logger.hpp>
#include <fstream>
#include <json/json.h>


class Analyzer {
private:
    vector<std::string> name_filter;

    int distance_min;
    int distance_max;
    int duree_max;
    int duree_min;
    int d_plus;

    std::ifstream file;

public:
    Analyzer() : distance_min(-1), distance_max(-1),
        duree_min(-1), duree_max(-1),
        d_plus(-1) 
    {
        name_filter = vector<std::string>();
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

    friend std::ostream& operator<<(std::ostream& out, const Analyzer& other);

};

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

    this->distance_min = root["filtre"]["distance_min"].asUInt();
    this->distance_max = root["filtre"]["distance_max"].asUInt64();
    this->d_plus = root["filtre"]["d+"].asInt();
    this->duree_min = root["filtre"]["duree_min"].asUInt64();
    this->duree_max = root["filtre"]["duree_max"].asUInt64();
    
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

    vector<Json::Value> filtered();
    bool isOk = true;

    for (auto &run : root)
    {
        isOk = true;
        std::string name = run["name"].asString();
        if (this->distance_min != -1 && (run["distance"].asInt() / 1000) < this->distance_min)
        {
            isOk = false;
        }
        else if (this->distance_max != -1 && (run["distance"].asInt() / 1000) > this->distance_max)
        {
            isOk = false;
        } else if (this->duree_max != -1 && run["elapsed_time"].asInt() / 60 > this->duree_max)
        {
            isOk = false;
        } else if (this->duree_min != -1 && run["elapsed_time"].asInt() / 60 < this->duree_min)
        {
            isOk = false;
        }
        
        if (isOk)
        {
            for (auto & it : this->name_filter)
            {
                if ( name.find(it) )
                {
                    filtered().push_back(it);
                }
            }
        }
    }

    return 1;
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