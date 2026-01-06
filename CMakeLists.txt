cmake_minimum_required(VERSION 3.10.0)
project(strava VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)

find_package(CURL REQUIRED)
find_package(OpenSSL REQUIRED)

add_compile_options(-g)

file(GLOB JSONCPP_SOURCES
    external/jsoncpp/src/lib_json/*.cpp
)

add_executable(strava
    main.cpp
    src/Logger.cpp
    src/Connector.cpp
    src/Analyzer.cpp
    src/StravaAuthController.cpp
    ${JSONCPP_SOURCES}
)

# Includes projet
target_include_directories(strava PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/external/httplib
    ${PROJECT_SOURCE_DIR}/external/jsoncpp/include
)


# Options httplib
target_compile_definitions(strava PRIVATE CPPHTTPLIB_OPENSSL_SUPPORT)

# Libs système
target_link_libraries(strava PRIVATE
    CURL::libcurl
    OpenSSL::SSL
    OpenSSL::Crypto
    pthread
)
