#include "Analyzer.hpp"

// Global structure defined in the Analyzer.hpp file
Analyzer analyzer;

/*! \brief Made to set_up the data in the Analyzer Class
 *         Should be used once, before using the Analyzer class
 *
 * @param None
 */
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

/*! \brief Made to sort the data from strava_response.json
 *        This function will parse it, and register the usefull data in
 * Analyzer->filtered
 * To know how to sort, you need to set the file 'command.json'
 *
 * @param None
 */
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

/*! \brief Made to sort the data in Analyzer->filtered
 *        You need to fill Analyzer->filtered before, using the
 * Analyzer::extract() funtion
 *
 * @param None
 */
void Analyzer::sorter() {
  // TODO sort the data
  std::sort(this->filtered.begin(), this->filtered.end(),
            [](const Json::Value &lha, const Json::Value &rha) {
              return lha["name"].asString() < rha["name"].asString();
            });
}

/*! \brief Made to write to the file 'output/output.json' the data amoung the
 * Analyzer->filtered
 *
 * @param None
 */
void Analyzer::debug() {
  ofstream filtered_run("output/output.json");
  for (auto &run : this->filtered) {
    filtered_run << run;
  }
}

/*! \brief Overloading of the write function to log intelligent data to
 * log the filter of the analyzer
 *
 * @param None
 */
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

/*! \brief Function to convert the speed to a min/km unit
 *
 * @param double v : The input speed to convert
 * @return double : The speed converted
 */
double Analyzer::speed_to_pace(const double v) {
  double toReturn = (1000.0 / v) / 60;
  cout << "Calcule allure du back-end :" << toReturn << endl;
  return toReturn;
}
