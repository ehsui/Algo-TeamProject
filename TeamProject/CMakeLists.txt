add_executable(TeamProject main.cpp)

find_package(CURL CONFIG REQUIRED)
target_link_libraries(TeamProject PRIVATE CURL::libcurl)
