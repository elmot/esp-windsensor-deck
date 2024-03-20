cmake_minimum_required(VERSION 3.28)
project(test CXX)

set(CMAKE_CXX_STANDARD 11)

add_executable(test-nmea-parser main.cpp ../esp32-deck-display/main/nmea-parser.cpp)
target_compile_definitions(test-nmea-parser PRIVATE NMEA_WIND_PARSER_TEST)
target_include_directories(test-nmea-parser PRIVATE ../esp32-deck-display/main)