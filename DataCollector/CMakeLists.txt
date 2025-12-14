cmake_minimum_required(VERSION 3.15)
project(YouTubeDataCollector CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(unofficial-sqlite3 CONFIG REQUIRED)
find_package(CURL CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

add_executable(YouTubeDataCollector main.cpp)

target_link_libraries(YouTubeDataCollector PRIVATE
    unofficial::sqlite3::sqlite3
    CURL::libcurl
    nlohmann_json::nlohmann_json
)

# Copy config.txt to build folder
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/config.txt
    ${CMAKE_CURRENT_BINARY_DIR}/config.txt
    COPYONLY
)

# Create data output directory
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../data)