#ifndef LOGGER_H
#define LOGGER_H

#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>

using namespace std;


enum LogLevel { DEBUG, INFO, WARNING, ERROR, CRITICAL };

class Logger {
    ofstream logFile; // File stream for the log file
public:
    Logger(const string& filename);
    ~Logger();
    void log(LogLevel level, const string& message);
};

extern Logger logger;

#endif