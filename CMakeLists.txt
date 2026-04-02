cmake_minimum_required(VERSION 3.16)
project(FileManager VERSION 1.0 LANGUAGES CXX)
 
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
 
# Release build по умолчанию если не указано
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
 
# Оптимизации
if(MSVC)
    add_compile_options(/W3 /O2 /utf-8)
else()
    add_compile_options(-Wall -Wextra -O2)
endif()
 
set(SOURCES
    main.cpp
    logger.cpp
    fileops.cpp
    search.cpp
    ui.cpp
)
 
add_executable(fm ${SOURCES})
 
# Линковка с потоками
find_package(Threads REQUIRED)
target_link_libraries(fm Threads::Threads)
 
# Windows: нужна stdc++fs на старых MinGW
if(MINGW)
    target_link_libraries(fm stdc++fs)
endif()
 
# Иконка/манифест Windows (опционально)
if(WIN32)
    target_compile_definitions(fm PRIVATE WIN32_LEAN_AND_MEAN)
    target_link_libraries(fm)
endif()
 
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")