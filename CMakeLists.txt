cmake_minimum_required(VERSION 3.10.0)
project(strava VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)

find_package(CURL REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
find_package(Python3 COMPONENTS NumPy REQUIRED)

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(/usr/include)
include_directories(${PROJECT_SOURCE_DIR}/lib/matplotlib-cpp)

add_executable(strava main.cpp src/Logger.cpp src/csvParser.cpp src/Connector.cpp src/StravaOAuth.cpp)
target_link_libraries(strava PRIVATE CURL::libcurl jsoncpp OpenSSL::SSL OpenSSL::Crypto Python3::Python Python3::NumPy)
target_compile_definitions(strava PRIVATE CPPHTTPLIB_OPENSSL_SUPPORT)
