#pragma once

#include <Logger.hpp>
#include <Python.h>
#include <algorithm>
#include <cstdlib>
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

  vector<Json::Value> get_filtered() { return this->filtered; }

  friend std::ostream &operator<<(std::ostream &out, const Analyzer &other);
};

extern Analyzer analyzer;
