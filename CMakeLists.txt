cmake_minimum_required(VERSION 3.25)
set(CMAKE_POLICY_DEFAULT_CMP0091 NEW)
project(CopperServer CXX)
set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(Boost_NO_WARN_NEW_VERSIONS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)




add_subdirectory(tools)
message(STATUS "Building resources")
FILE(GLOB RESOURCE_FILES CONFIGURE_DEPENDS 
  resources/*/*.* 
  resources/*/*/*.* 
  resources/*/*/*/*.* 
  resources/*/*/*/*/*.* 
  resources/*/*/*/*/*/*.*
)
list(FILTER RESOURCE_FILES EXCLUDE REGEX ".*/resources/include.hpp")
list(FILTER RESOURCE_FILES EXCLUDE REGEX ".*/data/.*")
build_resource_merge_json("copper_server" "${CMAKE_CURRENT_SOURCE_DIR}/resources/data" "out/built_resources")
build_resources_json("copper_server" "out/built_resources" "${RESOURCE_FILES}" RESOURCE_FILES_include RESOURCE_FILES_build)
create_resources_header("resources" "${RESOURCE_FILES_include}")





if(MSVC)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  add_definitions(/MP)
  add_definitions(/GF)
  add_definitions(/bigobj)
  add_definitions(/utf-8)
  add_definitions(/Zc:preprocessor)
  add_compile_options(-DPLATFORM_WINDOWS=1)
else(MSVC)
  add_compile_options(-m64)
  add_compile_options(-g)
  set(Boost_USE_STATIC_LIBS ON)
  set(Boost_USE_STATIC_RUNTIME ON)
  add_compile_options(-DPLATFORM_LINUX=1)
endif(MSVC)
set(ENABLE_BOOST_PREPROCESSOR ON)


set(COMMANDLINE_LIBTYPE STATIC)
set(COMMANDLINE_SHARED_LIBS OFF)

find_package(Boost REQUIRED COMPONENTS context json bimap iostreams)
find_package(ZLIB REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(utf8cpp CONFIG REQUIRED)

add_subdirectory("library/fast_task")
add_subdirectory("library/commandline")

set(CC_src_INCLUDE_DIR ".")
set(CC_INCLUDE_DIR ${CC_src_INCLUDE_DIR})

FILE(GLOB SRCFILES 
        src/*.cpp 
        src/*/*.cpp 
        src/*/*/*.cpp 
        src/*/*/*/*.cpp
        src/*/*/*/*/*.cpp
        library/enbt/*.cpp
)

include_directories(${Boost_INCLUDE_DIRS} ${CC_INCLUDE_DIR} ${ZLIB_INCLUDE_DIRS} ${OPENSSL_INCLUDE_DIR} ${utf8cpp_INCLUDE_DIRS})


IF(MSVC)
  SET(CMAKE_CXX_FLAGS "/EHsc")
ENDIF(MSVC)
  add_executable(CopperServer ${SRCFILES} ${RESOURCE_FILES_build})
  set_property(TARGET CopperServer PROPERTY
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  target_link_libraries(CopperServer
    PRIVATE
      ${Boost_LIBRARIES}
      ZLIB::ZLIB
      OpenSSL::SSL
      utf8::cpp
      commandline
      fast_task
    )
