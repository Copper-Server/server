cmake_minimum_required(VERSION 3.23)
project(binary_embed CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

FILE(GLOB SRCFILES 
    main.cpp
    ../../library/enbt/enbt.cpp
)

find_package(Boost REQUIRED COMPONENTS iostreams)
find_package(ZLIB REQUIRED)
include_directories(${Boost_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIRS})


add_executable(binary_embed ${SRCFILES})
target_compile_options(binary_embed PRIVATE /MT)
target_link_libraries(binary_embed
    PRIVATE
      Boost::iostreams
      ZLIB::ZLIB
    )

#copy executable to current path
add_custom_command(TARGET binary_embed POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:binary_embed> ${CMAKE_CURRENT_SOURCE_DIR}/../binary_embed.exe
)