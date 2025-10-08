cmake_minimum_required(VERSION 3.23)
project(id_proxy_gen CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

FILE(GLOB SRCFILES 
    main.cpp
)

find_package(Boost REQUIRED COMPONENTS algorithm property_tree)
add_executable(id_proxy_gen ${SRCFILES})
target_link_libraries(id_proxy_gen PRIVATE Boost::algorithm Boost::property_tree)

IF(MSVC)
    target_compile_options(id_proxy_gen PRIVATE /MT)
else()
  target_link_libraries(id_proxy_gen PRIVATE stdc++_libbacktrace)
  target_compile_options(id_proxy_gen PRIVATE -g)
endif()

#copy executable to current path
add_custom_command(TARGET id_proxy_gen POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:id_proxy_gen> ${CMAKE_CURRENT_SOURCE_DIR}/../id_proxy_gen.exe
)