cmake_minimum_required(VERSION 3.13)
project(pntr_app_sfx_example
    DESCRIPTION "Simple example with pntr_app_sfx"
    HOMEPAGE_URL "https://github.com/konsumer/pntr_app_sfx"
    VERSION 0.0.1
    LANGUAGES C
)

cmake_policy(SET CMP0135 NEW)

include(FetchContent)

if (EMSCRIPTEN)
  # build for web, with pure-web layer
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -sUSE_GLFW=3 -sASYNCIFY")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -sASSERTIONS=1 -sWASM=1 --shell-file ${CMAKE_SOURCE_DIR}/shell.html")
  set(CMAKE_EXECUTABLE_SUFFIX ".html")
  add_definitions(-DPNTR_APP_WEB)
else()
  # build for raylib
  add_definitions(-DPNTR_APP_RAYLIB)
  FetchContent_Declare(raylib
    URL https://github.com/raysan5/raylib/archive/refs/heads/master.zip
  )
  FetchContent_MakeAvailable(raylib)
endif ()

FetchContent_Declare(pntr
  URL https://github.com/RobLoach/pntr/archive/refs/heads/master.zip
)
FetchContent_MakeAvailable(pntr)

FetchContent_Declare(pntr_app
  URL https://github.com/RobLoach/pntr_app/archive/refs/heads/master.zip
)
FetchContent_MakeAvailable(pntr_app)

FetchContent_Declare(pntr_nuklear
  URL https://github.com/RobLoach/pntr_nuklear/archive/refs/heads/master.zip
)
FetchContent_MakeAvailable(pntr_nuklear)

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE)

add_executable(pntr_app_sfx_example pntr_app_sfx_example.c)
if (EMSCRIPTEN)
  target_link_libraries(pntr_app_sfx_example pntr pntr_app)
else()
  target_link_libraries(pntr_app_sfx_example pntr pntr_app raylib)
endif ()
target_include_directories(pntr_app_sfx_example PRIVATE "${CMAKE_CURRENT_LIST_DIR}/.." "${raylib_SOURCE_DIR}/src")

add_executable(pntr_app_sfx_gen pntr_app_sfx_gen.c)
if (EMSCRIPTEN)
  target_link_libraries(pntr_app_sfx_gen pntr pntr_app pntr_nuklear)
else()
  target_link_libraries(pntr_app_sfx_gen pntr pntr_app pntr_nuklear raylib)
endif ()
target_include_directories(pntr_app_sfx_gen PRIVATE "${CMAKE_CURRENT_LIST_DIR}/.." "${raylib_SOURCE_DIR}/src")
