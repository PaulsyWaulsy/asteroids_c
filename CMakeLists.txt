cmake_minimum_required(VERSION 3.10)

# Set project name and versionk0304
project(asteroids)

# Find SDL2 package
find_package(SDL2 REQUIRED)

# Add source files
file(GLOB SOURCES src/*c)

# Add the executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Add compile options for the target
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)

# Link SDL2 libraries
target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES} -lm)
