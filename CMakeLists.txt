cmake_minimum_required(VERSION 3.10.0)
project(strava VERSION 0.1.0 LANGUAGES C CXX)

find_package(CURL REQUIRED)

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(/usr/include)

add_executable(strava main.cpp src/Logger.cpp src/csvParser.cpp src/Connector.cpp src/StravaOAuth.cpp)
target_link_libraries(strava PRIVATE CURL::libcurl jsoncpp)

