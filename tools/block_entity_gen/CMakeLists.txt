cmake_minimum_required(VERSION 3.23)
project(block_entity_gen CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_BUILD_TYPE RelWithDebInfo)

FILE(GLOB SRCFILES 
    main.cpp
)

find_package(Boost REQUIRED COMPONENTS algorithm property_tree)
add_executable(block_entity_gen ${SRCFILES})
target_link_libraries(block_entity_gen PRIVATE Boost::algorithm Boost::property_tree)

IF(MSVC)
    target_compile_options(block_entity_gen PRIVATE /MTd)
else()
  target_link_libraries(block_entity_gen PRIVATE stdc++_libbacktrace)
  target_compile_options(block_entity_gen PRIVATE -g)
endif()

#copy executable to current path
add_custom_command(TARGET block_entity_gen POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:block_entity_gen> ${CMAKE_CURRENT_SOURCE_DIR}/../block_entity_gen.exe
)