cmake_minimum_required(VERSION 3.31)
project(CopperServer CXX)
set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(Boost_NO_WARN_NEW_VERSIONS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(tools)
message(STATUS "Building resources")
FILE(GLOB REFLECTION_FILES CONFIGURE_DEPENDS
  src/api/packets.hpp
  src/base_objects/component.hpp
  src/base_objects/dye_color.hpp
  src/base_objects/packets_help.hpp
  src/util/calculations.hpp
)
FILE(GLOB RESOURCE_FILES CONFIGURE_DEPENDS
  resources/*/*.* 
  resources/*/*/*.* 
  resources/*/*/*/*.* 
  resources/*/*/*/*/*.* 
  resources/*/*/*/*/*/*.*
)
list(FILTER RESOURCE_FILES EXCLUDE REGEX ".*/resources/include.hpp")
list(FILTER RESOURCE_FILES EXCLUDE REGEX ".*/data/.*")
build_resource_merge_json("copper_server" "${CMAKE_CURRENT_SOURCE_DIR}/resources/data" "out/built_resources" compressed)
build_resources("copper_server" "out/built_resources" "${RESOURCE_FILES}" RESOURCE_FILES_include RESOURCE_FILES_build compressed)
create_resources_header("resources" "${RESOURCE_FILES_include}")
create_reflect_map("${CMAKE_CURRENT_SOURCE_DIR}/src/util/reflect_fns.hpp" "${REFLECTION_FILES}")


IF(MSVC)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>") #I hate using this for library/commandline. Its just breaks, COMMANDLINE_LIBTYPE and COMMANDLINE_SHARED_LIBS are just not working as excepted
  string(REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
  string(REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})
  string(REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})
ENDIF(MSVC)

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
  src/*/*/*/*/*/*.cpp
  src/*/*/*/*/*/*/*.cpp
  library/enbt/*.cpp
)

include_directories(${Boost_INCLUDE_DIRS} ${CC_INCLUDE_DIR} ${ZLIB_INCLUDE_DIRS} ${OPENSSL_INCLUDE_DIR} ${utf8cpp_INCLUDE_DIRS})


IF(MSVC)
  SET(CMAKE_CXX_FLAGS "/EHa")
  add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
ENDIF(MSVC)
add_executable(CopperServer ${SRCFILES} ${RESOURCE_FILES_build})
IF(MSVC)
  target_compile_options(CopperServer PRIVATE /utf-8)
  target_compile_options(CopperServer PUBLIC /wd4100)
  target_compile_options(CopperServer PUBLIC /wd4505)
  target_compile_options(CopperServer PUBLIC /wd4324)
  target_compile_options(CopperServer PUBLIC /wd4717)
  target_compile_options(CopperServer PUBLIC /sdl)
  target_compile_options(CopperServer PUBLIC /bigobj)
  target_compile_options(CopperServer PRIVATE /utf-8)
  target_compile_options(CopperServer PUBLIC /GF)
  add_compile_definitions(_DISABLE_VECTOR_ANNOTATION)
  add_compile_definitions(_DISABLE_STRING_ANNOTATION)
  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(CopperServer PUBLIC /MTd)
  else()
    target_compile_options(CopperServer PUBLIC /MT)
  endif()
  target_compile_options(CopperServer PUBLIC /W4)
  target_compile_options(CopperServer PUBLIC /WX)

  target_compile_options(CopperServer PUBLIC /permissive-)
ENDIF(MSVC)
IF(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  target_compile_options(CopperServer PUBLIC -Wno-unused-function)
  
  IF(MSVC)
    target_compile_options(CopperServer PUBLIC /clang:-ftemplate-backtrace-limit=0)
  ENDIF(MSVC)
ENDIF()

target_link_libraries(CopperServer PRIVATE
  ${Boost_LIBRARIES}
  ZLIB::ZLIB
  OpenSSL::SSL
  utf8::cpp
  commandline
  fast_task
)
