# DataCollector/CMakeLists.txt

cmake_minimum_required(VERSION 3.15)
project(DataCollector CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. 의존성 패키지 찾기
# Vcpkg가 제공하는 정확한 패키지 이름을 사용하여 찾습니다.
find_package(CURL CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

# SQLite3는 Vcpkg에서 'unofficial-sqlite3' 이름으로 제공되므로 이를 사용합니다.
find_package(unofficial-sqlite3 CONFIG REQUIRED) 

# 2. 실행 파일 정의
# data_collector.cpp 파일을 컴파일하여 data_collector 실행 파일을 정의합니다.
add_executable(data_collector main.cpp)

# 3. 라이브러리 연결 (모든 의존성을 data_collector 실행 파일에 연결)
target_link_libraries(data_collector PRIVATE
    CURL::libcurl
    nlohmann_json::nlohmann_json
    
    # 💡 SQLite3 연결: 'unofficial-sqlite3' 패키지가 제공하는 타겟을 사용합니다.
    unofficial::sqlite3::sqlite3 
)