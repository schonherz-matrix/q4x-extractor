cmake_minimum_required(VERSION 3.16)

project(
        q4x-extractor
        DESCRIPTION "Extracts qp4, qpr, mp3/ogg(if exists) files from q4x"
        HOMEPAGE_URL "kszk.bme.hu"
        LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(
        Qt6
        COMPONENTS Core
        REQUIRED)

add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Core)
