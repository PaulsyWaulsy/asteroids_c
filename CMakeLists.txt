cmake_minimum_required(VERSION 3.10)

# Set project name
project(asteroids)

# Find SDL2 and SDL_mixer
find_package(SDL2 REQUIRED)
find_package(SDL2_mixer REQUIRED)

# Add source files
file(GLOB SOURCES src/*.c)

# Add the executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Add compile options
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)

# Link SDL2 and SDL_mixer
target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES} SDL2_mixer -lm)
