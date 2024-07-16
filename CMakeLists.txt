cmake_minimum_required(VERSION 3.23)
set(CMAKE_POLICY_DEFAULT_CMP0091 NEW)
project(CraftedCart CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(Boost_NO_WARN_NEW_VERSIONS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(MSVC)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  add_definitions(/MP)
  add_definitions(/Zc:preprocessor)
else(MSVC)
  add_compile_options(-m64)
  add_compile_options(-g)
  set(Boost_USE_STATIC_LIBS ON)
  set(Boost_USE_STATIC_RUNTIME ON)
endif(MSVC)
set(ENABLE_BOOST_PREPROCESSOR ON)


set(COMMANDLINE_LIBTYPE STATIC)
set(COMMANDLINE_SHARED_LIBS OFF)

add_subdirectory("src/library/fast_task")
add_subdirectory("src/library/commandline")

set(CC_src_INCLUDE_DIR "src")
set(CC_INCLUDE_DIR ${CC_src_INCLUDE_DIR})


FILE(GLOB SRCFILES 
        src/protocolHelper/*/*.cpp
        src/protocolHelper/*/*/*.cpp
        src/protocolHelper/*/*/*/*.cpp
        src/library/ENBT/enbt.cpp
        src/base_objects/*.cpp 
        src/storage/*.cpp 
        src/api/*.cpp 
        src/util/*.cpp 
        src/plugin/*.cpp 
        src/build_in_plugins/*.cpp 
        src/*.cpp 
)


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
      commandline::commandline
      fast_task
    )
