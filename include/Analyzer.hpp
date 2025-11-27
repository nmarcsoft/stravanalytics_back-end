#pragma once

#include <iostream>
#include <string>
#include <Logger.hpp>
#include <fstream>
#include <json/json.h>


class Analyzer {
private:
    vector<std::string> name_filter;

    unsigned int distance_min;
    unsigned int distance_max;
    unsigned int duree;
    
    int d_plus;

public:
    Analyzer() : distance_min(0), distance_max(0),
        duree(0),
        d_plus(0) 
    {
        name_filter = vector<std::string>();
    }

    int extract();
    void draw_graph();

    friend std::ostream& operator<<(std::ostream& out, const Analyzer other);

};

int Analyzer::extract() {
    std::ifstream file("../command.json");
    if (!file.is_open()) {
        std::cerr << "Impossible d’ouvrir le fichier JSON !" << std::endl;
        return 1;
    }

    Json::Value root;
    Json::CharReaderBuilder builder;
    JSONCPP_STRING errs;

    if (!parseFromStream(builder, file, &root, &errs)) {
        std::cerr << "Erreur JSON : " << errs << std::endl;
        return 1;
    }

    this->distance_min = root["filtre"]["distance_min"].asUInt();
    this->distance_max = root["filtre"]["distance_max"].asUInt64();
    this->d_plus = root["filtre"]["d+"].asInt();
    this->duree = root["filtre"]["duree"].asUInt64();
    
    for (auto &it : root["filtre"]["nom"])
    {
        name_filter.push_back(it.asString());
    }

    return 0;
}

ostream& operator<< (ostream& out, const Analyzer other)
{
    out << "Name filter :" << endl;
    for (auto &it : other.name_filter)
    {
        out << "\t- " << it << endl;
    }
    
    out << "Distance filter : \n\t- [" << other.distance_min << ";" << other.distance_max << "]" << endl; 
    out << "Déniveler filter : \n\t- " << other.d_plus << endl;
    out << "Durée filter \n\t- " << other.duree << endl;

    return out;
}