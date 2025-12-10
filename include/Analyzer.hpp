#pragma once

#include <Logger.hpp>
#include <Python.h>
#include <ctime>
#include <fstream>
#include <iostream>
#include <json/json.h>
#include <limits>
#include <string>
#include <tuple>

enum graph_type { DEFAULT };

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

    if (distance_min != -1 && (run["distance"].asInt() / 1000) < distance_min) {
      isOk = false;
    } else if (distance_max != -1 &&
               (run["distance"].asInt() / 1000) > distance_max) {
      isOk = false;
    } else if (duree_max != -1 &&
               (run["elapsed_time"].asInt() / 60) > duree_max) {
      isOk = false;
    } else if (duree_min != -1 &&
               (run["elapsed_time"].asInt() / 60) < duree_min) {
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

// TODO sort the data
void Analyzer::sorter() {}

void Analyzer::debug() {
  ofstream filtered_run("output.json");
  for (auto &it : this->filtered) {
    cout << it << endl;
    filtered_run << it << endl;
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
  Py_Initialize();

  if (type == DEFAULT) {
    std::ostringstream ss_speed;
    std::ostringstream ss_bpm;
    std::ostringstream ss_x;

    // x = RUN y = PACE & BPM
    vector<unsigned int> x;
    unsigned int i = 0u;

    vector<unsigned int> bpm;
    vector<double> speed;
    ss_speed << "[";
    ss_bpm << "[";
    ss_x << "[";
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
        ss_speed << ", ";
        ss_bpm << ", ";
        ss_x << ", ";
      }

      i++;
    }
    ss_speed << "]";
    ss_bpm << "]";
    ss_x << "]";

    string speed_list = ss_speed.str();
    string bpm_list = ss_bpm.str();
    string x_list = ss_x.str();

    std::stringstream script;
    script << "import matplotlib.pyplot as plt\n"
              "x = "
           << x_list
           << "\n"
              "pace = "
           << speed_list
           << "\n"
              "plt.figure(figsize=(10,5))\n"
              "plt.plot(x, pace, marker='o')\n"
              "plt.xlabel('Numéro du footing')\n"
              "plt.ylabel('Allure (min/km)')\n"
              "plt.title('Allure par footing')\n"
              "plt.grid(True)\n"
              "plt.gca().invert_yaxis()\n"
              "plt.savefig('allure_par_footing.pdf')\n";

    PyRun_SimpleString(script.str().c_str());

    Py_Finalize();

    std::cout << "Graphique généré : allure_par_footing.pdf\n";
  }
}
