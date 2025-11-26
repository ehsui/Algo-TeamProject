cmake_minimum_required(VERSION 3.15)
project(DataCollector CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. 의존성 패키지 찾기
find_package(CURL CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

# ★ 중요: SQLite는 vcpkg에서 제공하는 'unofficial' 이름을 써야만 찾을 수 있습니다.
find_package(unofficial-sqlite3 CONFIG REQUIRED)

# 2. 실행 파일 정의 (이름: data_collector)
add_executable(data_collector main.cpp)

# 3. 라이브러리 연결
target_link_libraries(data_collector PRIVATE
    CURL::libcurl
    nlohmann_json::nlohmann_json
    # ★ 중요: 라이브러리 연결 이름도 unofficial 버전을 사용합니다.
    unofficial::sqlite3::sqlite3
)

# 4. 빌드 후 config.json 복사
# (이전에 수정한 대로 타겟 이름을 'data_collector'로 맞춤)
add_custom_command(TARGET data_collector POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/config.json"
        "$<TARGET_FILE_DIR:data_collector>/config.json"
)