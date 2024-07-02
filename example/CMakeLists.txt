cmake_minimum_required(VERSION 3.13)
project(pntr_app_sfx_example
    DESCRIPTION "Simple example with pntr_app_sfx"
    HOMEPAGE_URL "https://github.com/konsumer/pntr_app_sfx"
    VERSION 0.0.1
    LANGUAGES C
)

# could be PNTR_APP_SDL, PNTR_APP_CLI, PNTR_APP_RAYLIB, PNTR_APP_LIBRETRO
# everything is setup for PNTR_APP_RAYLIB, though
add_definitions(-DPNTR_APP_RAYLIB)

# build for web
if (EMSCRIPTEN)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -sUSE_GLFW=3 -sASYNCIFY")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -sASSERTIONS=1 -sWASM=1 --shell-file ${CMAKE_SOURCE_DIR}/shell.html")
  set(CMAKE_EXECUTABLE_SUFFIX ".html")
  add_definitions(-DPLATFORM=Web)
  add_definitions(-DPNTR_APP_WEB)
endif ()

cmake_policy(SET CMP0135 NEW)

include(FetchContent)

FetchContent_Declare(pntr
  URL https://github.com/RobLoach/pntr/archive/refs/heads/master.zip
)
FetchContent_MakeAvailable(pntr)

FetchContent_Declare(pntr_app
  URL https://github.com/RobLoach/pntr_app/archive/refs/heads/master.zip
)
FetchContent_MakeAvailable(pntr_app)

FetchContent_Declare(raylib
  URL https://github.com/raysan5/raylib/archive/refs/heads/master.zip
)
FetchContent_MakeAvailable(raylib)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE)

add_executable(pntr_app_sfx_example pntr_app_sfx_example.c)
target_link_libraries(pntr_app_sfx_example pntr pntr_app raylib)
target_include_directories(pntr_app_sfx_example PRIVATE "${CMAKE_CURRENT_LIST_DIR}/.." "${raylib_SOURCE_DIR}/src")

