cmake_minimum_required(VERSION 3.23)
project(reflect_map CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

FILE(GLOB SRCFILES 
    main.cpp
)

add_executable(reflect_map ${SRCFILES})
target_compile_options(reflect_map PRIVATE /MT)

#copy executable to current path
add_custom_command(TARGET reflect_map POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:reflect_map> ${CMAKE_CURRENT_SOURCE_DIR}/../reflect_map.exe
)