cmake_minimum_required(VERSION 3.15)

# 프로젝트 이름을 명확하게 지정
project(AlgoTeamProject CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. 의존성 패키지 찾기
# SQLite3는 필수입니다 (DB 읽기용)
find_package(unofficial-sqlite3 CONFIG REQUIRED)

# 2. 실행 파일 정의
# 소스 파일이 main.cpp라고 가정 (다를 경우 수정 필요)
add_executable(AlgoTeamProject main.cpp)

# 3. 라이브러리 연결
target_link_libraries(AlgoTeamProject PRIVATE
    unofficial::sqlite3::sqlite3  # DB 접근을 위해 필수 연결
    # nlohmann_json::nlohmann_json
    # CURL::libcurl
)

# (참고) 실행 시 DB 파일을 찾을 수 있도록 작업 디렉토리 설정이 중요합니다.
# VS Code의 launch.json에서 "cwd"를 설정하거나,
# DataCollector가 생성한 youtube_data.db를 이 프로젝트의 실행 위치로 복사해야 합니다.