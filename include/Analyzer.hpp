#pragma once

#include <Logger.hpp>
#include <Python.h>
#include <algorithm>
#include <ctime>
#include <fstream>
#include <iostream>
#include <json/json.h>
#include <limits>
#include <pylifecycle.h>
#include <pythonrun.h>
#include <string>
#include <tuple>

enum graph_type { DEFAULT, PER_TYPE };

class Analyzer {
private:
  vector<std::string> name_filter;

  std::tuple<int, int> distance;
  std::tuple<int, int> duree;
  std::tuple<int, int> d_plus;

  vector<std::string> comments;

  std::ifstream file;

  vector<Json::Value> filtered;

  void sorter();

public:
  Analyzer() {
    this->distance = {-1, -1};
    this->duree = {-1, -1};
    this->d_plus = {-1, -1};
    this->name_filter = vector<std::string>();
    this->filtered = vector<Json::Value>();
    file.open("command.json");
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
  void draw_graph(graph_type type);
  void debug();

  friend std::ostream &operator<<(std::ostream &out, const Analyzer &other);
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
    std::cerr << "Erreur JSON - set up: " << errs << std::endl;
    return 1;
  }

  this->distance = {root["filtre"]["distance"][0].asInt(),
                    root["filtre"]["distance"][1].asInt()};
  this->d_plus = {root["filtre"]["d+"][0].asInt(),
                  root["filtre"]["d+"][1].asInt()};
  this->duree = {root["filtre"]["duree"][0].asInt(),
                 root["filtre"]["duree"][1].asInt()};

  for (auto &it : root["filtre"]["nom"]) {
    name_filter.push_back(it.asString());
  }

  return 0;
}

int Analyzer::extract() {
  std::ifstream input("strava_response.json");

  if (!input.is_open()) {
    std::cerr << "Impossible d’ouvrir le fichier JSON !" << std::endl;
    return 1;
  }

  Json::Value root;
  Json::CharReaderBuilder builder;
  JSONCPP_STRING errs;

  if (!parseFromStream(builder, input, &root, &errs)) {
    std::cerr << "Erreur JSON - extract : " << errs << std::endl;
    return 1;
  }

  bool isOk = true;

  for (auto &run : root) {
    isOk = true;
    std::string name = run["name"].asString();
    int distance_run = run["distance"].asInt();
    int elevation = run["total_elevation_gain"].asInt();

    int distance_min, distance_max, duree_min, duree_max, d_plus_min,
        d_plus_max;
    distance_min = std::get<0>(this->distance);
    distance_max = std::get<1>(this->distance);
    duree_min = std::get<0>(this->duree);
    duree_max = std::get<1>(this->duree);
    d_plus_min = std::get<0>(this->d_plus);
    d_plus_max = std::get<1>(this->d_plus);

    if (distance_min != -1 && (run["distance"].asInt()) < distance_min) {
      isOk = false;
    } else if (distance_max != -1 && (run["distance"].asInt()) > distance_max) {
      isOk = false;
    } else if (duree_max != -1 && (run["elapsed_time"].asInt()) > duree_max) {
      isOk = false;
    } else if (duree_min != -1 && (run["elapsed_time"].asInt()) < duree_min) {
      isOk = false;
    } else if (d_plus_min != -1 && d_plus_min > elevation) {
      isOk = false;
    } else if (d_plus_max != -1 && d_plus_max < elevation) {
      isOk = false;
    } else if (run["type"] != "Run") {
      isOk = false;
    }

    if (isOk) {
      for (auto &it : this->name_filter) {
        if (name.find(it) != std::string::npos) {
          this->filtered.push_back(run);
        }
      }
    }
  }
  this->sorter();
  return 1;
}

// Tri sur le nom des données
void Analyzer::sorter() {

  std::sort(this->filtered.begin(), this->filtered.end(),
            [](const Json::Value &lha, const Json::Value &rha) {
              return lha["name"].asString() < rha["name"].asString();
            });
}

void Analyzer::debug() {
  ofstream filtered_run("output/output.json");
  for (auto &run : this->filtered) {
    filtered_run << run;
  }
}

ostream &operator<<(ostream &out, const Analyzer &other) {
  out << "Name filter :" << endl;
  for (auto &it : other.name_filter) {
    out << "\t- " << it << endl;
  }

  out << "Distance filter : \n\t- [" << std::get<0>(other.distance) << ";"
      << std::get<1>(other.distance) << "]" << endl;
  out << "Dénivelé filter : \n\t- [" << std::get<0>(other.d_plus) << ";"
      << std::get<1>(other.d_plus) << "]" << endl;
  out << "Duree filter : \n\t- [" << std::get<0>(other.duree) << ";"
      << std::get<1>(other.duree) << "]" << endl;

  return out;
}

double speed_to_pace(const double v) { return (1000.0 / v) / 60.0; }

void Analyzer::draw_graph(graph_type type = DEFAULT) {

  if (type == DEFAULT) {
    std::ostringstream ss_speed;
    std::ostringstream ss_bpm;
    std::ostringstream ss_x;

    // x = RUN y = PACE & BPM
    vector<unsigned int> x;
    unsigned int i = 0u;

    for (auto &it : this->filtered) {
      if (it.isMember("average_heartrate")) {
        ss_bpm << it["average_heartrate"].asInt();
      } else {
        ss_bpm << 0;
      }
      if (it.isMember("average_speed")) {
        ss_speed << speed_to_pace(it["average_speed"].asDouble());
      } else {
        ss_speed << 0.f;
      }
      ss_x << i;

      if (i + 1 < this->filtered.size()) {
        ss_speed << ",";
        ss_bpm << ",";
        ss_x << ",";
      }

      i++;
    }
    string speed_list = ss_speed.str();
    string bpm_list = ss_bpm.str();
    string x_list = ss_x.str();

    std::string file_to_execute = "src/graphics/graphique_pace.py";
    std::string commande = "python3 ";
    std::string argument = ss_x.str() + " " + ss_speed.str();

    system((commande + file_to_execute + " " + argument).c_str());
  } else if (type == PER_TYPE) {

    std::ostringstream x_list, pace_vma, pace_ef, bpm_vma, bpm_ef, other;
    int i = 1;

    for (auto &it : this->filtered) {
      std::string name = it["name"].asString();
      if (name.find("EF") != std::string::npos) {
        // On a une EF
        pace_ef << speed_to_pace(it["average_speed"].asDouble());
        x_list << i;

        pace_ef << ",";
        x_list << ",";
        i++;

      } else if (name.find("VMA") != std::string::npos) {
        // VMA
        pace_vma << speed_to_pace(it["average_speed"].asDouble());
        x_list << i;

        pace_vma << ",";
        x_list << ",";
        i++;
      }
    }

    string arg1 = x_list.str();
    string arg2 = pace_vma.str();
    string arg3 = pace_ef.str();

    // Dirty code coming :
    if (!arg1.empty()) {
      arg1.pop_back();
    }

    if (!arg2.empty()) {
      arg2.pop_back();
    }

    if (!arg3.empty()) {
      arg3.pop_back();
    }

    cout << "arg1 = " << arg1 << endl;
    cout << "arg2 = " << arg2 << endl;
    cout << "arg3 = " << arg3 << endl;

    std::string file_to_execute = "src/graphics/graphique_splited.py";
    std::string commande = "python3 ";
    std::string argument = arg1 + " " + arg2 + " " + arg3;

    system((commande + file_to_execute + " " + argument).c_str());
  }
}
