cmake_minimum_required(VERSION 3.20)
project(CraftedCart CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(Boost_NO_WARN_NEW_VERSIONS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_subdirectory("src/library/fast_task")

set(CC_src_INCLUDE_DIR "src")
set(CC_INCLUDE_DIR ${CC_src_INCLUDE_DIR})

if(MSVC)
  add_definitions(/MP)
  add_definitions(/Zc:preprocessor)
  string(REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
  string(REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})
  string(REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})
  string(REPLACE "/Gv" "/Gr" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS_RELEASE})
  string(REPLACE "/Gv" "/Gr" CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})
  string(REPLACE "/Gv" "/Gr" CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})
else(MSVC)
  add_compile_options(-m64)
  add_compile_options(-g)
  set(Boost_USE_STATIC_LIBS ON)
  set(Boost_USE_STATIC_RUNTIME ON)
endif(MSVC)
set(ENABLE_BOOST_PREPROCESSOR ON)

FILE(GLOB SRCFILES 
        src/protocolHelper/*/*.cpp
        src/protocolHelper/*/*/*.cpp
        src/library/ENBT/enbt.cpp
        src/base_objects/*.cpp 
        src/storage/*.cpp 
        src/api/*.cpp 
        src/plugin/*.cpp 
        src/*.cpp 
)

find_library(commandline_LIB commandline)

find_package(Boost REQUIRED COMPONENTS context json bimap)
find_package(ZLIB REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(utf8cpp CONFIG REQUIRED)

include_directories(${Boost_INCLUDE_DIRS} ${CC_INCLUDE_DIR}${ZLIB_INCLUDE_DIRS} ${OPENSSL_INCLUDE_DIR} ${utf8cpp_INCLUDE_DIRS})

IF(MSVC)
  SET(CMAKE_CXX_FLAGS "/EHsc")
ENDIF(MSVC)

  add_executable(CraftedCart ${SRCFILES})
  set_property(TARGET CraftedCart PROPERTY
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  target_link_libraries(CraftedCart
    PRIVATE
      ${Boost_LIBRARIES}
      ZLIB::ZLIB
      OpenSSL::SSL
      utf8::cpp
      ${commandline_LIB}
      fast_task
    )
