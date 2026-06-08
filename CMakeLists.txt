cmake_minimum_required(VERSION 3.10)
project(SNN)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)


if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O3")
endif()


set(CORE_SOURCES snn_core/snn_core.cpp)
set(LOADER_SOURCES data_loader/data_loader.cpp)


add_executable(${PROJECT_NAME} 
    main.cpp 
    ${CORE_SOURCES} 
    ${LOADER_SOURCES}
)


target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/snn_core
    ${CMAKE_CURRENT_SOURCE_DIR}/data_loader
)