cmake_minimum_required(VERSION 3.20)
project(mariadb_jev LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(CURL REQUIRED IMPORTED_TARGET libcurl)
pkg_check_modules(JANSSON REQUIRED IMPORTED_TARGET jansson)
find_path(MARIADB_INCLUDE_DIR mysql.h
  HINTS /usr/include/mariadb /usr/include/mysql
        /usr/local/opt/mariadb-connector-c/include/mariadb
        /opt/homebrew/opt/mariadb-connector-c/include/mariadb)
if(NOT MARIADB_INCLUDE_DIR)
  message(FATAL_ERROR "MariaDB development headers (mysql.h) are required")
endif()

add_library(jev_engine STATIC src/engine.cpp)
target_include_directories(jev_engine PUBLIC src)
target_link_libraries(jev_engine PUBLIC PkgConfig::CURL PkgConfig::JANSSON)
target_compile_options(jev_engine PRIVATE -Wall -Wextra -Wpedantic)

add_library(mariadb_jev MODULE src/udf.cpp)
target_include_directories(mariadb_jev PRIVATE ${MARIADB_INCLUDE_DIR})
target_link_libraries(mariadb_jev PRIVATE jev_engine)
set_target_properties(mariadb_jev PROPERTIES PREFIX "" SUFFIX ".so")
target_compile_options(mariadb_jev PRIVATE -Wall -Wextra -Wpedantic)

enable_testing()
add_executable(engine_test tests/engine_test.cpp)
target_link_libraries(engine_test PRIVATE jev_engine)
add_test(NAME engine COMMAND ${CMAKE_COMMAND} -E env
  JEV_API_KEY=test-key JEV_API_URL=http://127.0.0.1:18765/v1/systemone
  python3 ${CMAKE_SOURCE_DIR}/tests/run_mock.py $<TARGET_FILE:engine_test>)

add_executable(live_smoke tests/live_smoke.cpp)
target_link_libraries(live_smoke PRIVATE jev_engine)
