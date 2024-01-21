cmake_minimum_required(VERSION 3.20)
project(CraftedCart CXX)
set(CMAKE_CXX_STANDARD 23)
set(Boost_NO_WARN_NEW_VERSIONS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CC_src_INCLUDE_DIR "src")
set(CC_INCLUDE_DIR ${art_src_INCLUDE_DIR})

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
        src/library/*.cpp
        src/base_objects/*.cpp 
        src/*.cpp 
)

find_package(Boost REQUIRED COMPONENTS context)
find_package(utf8cpp CONFIG REQUIRED)

IF(MSVC)
  SET(CMAKE_CXX_FLAGS "/EHsc")
ENDIF(MSVC)

  add_executable(CraftedCart ${SRCFILES})
  set_property(TARGET CraftedCart PROPERTY
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  target_link_libraries(CraftedCart
    PRIVATE
      ${Boost_LIBRARIES}
      utf8::cpp
    )