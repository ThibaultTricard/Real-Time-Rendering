cmake_minimum_required(VERSION 3.10)
project(realtimerendering)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
    tinyobjloader
    GIT_REPOSITORY https://github.com/tinyobjloader/tinyobjloader.git
    GIT_TAG v2.0.0rc13
)
FetchContent_MakeAvailable(tinyobjloader)

set(LAVACAKE_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    LavaCake
    GIT_REPOSITORY https://github.com/ThibaultTricard/LavaCake.git
    GIT_TAG b9a0b4c
)
FetchContent_MakeAvailable(LavaCake)

# Find required dependencies
find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${Vulkan_INCLUDE_DIRS})

# TP1 - Calcul GPU (headless)
add_executable(TP1_exercice1 "src/TP1/exercice1.cpp")
target_link_libraries(TP1_exercice1 PRIVATE LavaCake::LavaCake)
target_compile_definitions(TP1_exercice1 PRIVATE PROJECT_ROOT="${CMAKE_SOURCE_DIR}/")

#add_executable(TP1_exercice2 "src/TP1/exercice2.cpp")
#target_link_libraries(TP1_exercice2 PRIVATE LavaCake::LavaCake)
#target_compile_definitions(TP1_exercice2 PRIVATE PROJECT_ROOT="${CMAKE_SOURCE_DIR}/")

#add_executable(TP1_exercice3 "src/TP1/exercice3.cpp")
#target_link_libraries(TP1_exercice3 PRIVATE LavaCake::LavaCake)
#target_compile_definitions(TP1_exercice3 PRIVATE PROJECT_ROOT="${CMAKE_SOURCE_DIR}/")

# TP2 - Rendu graphique
#add_executable(TP2_exercice1 "src/TP2/exercice1.cpp")
#target_link_libraries(TP2_exercice1 PRIVATE LavaCake::LavaCake glfw)
#target_compile_definitions(TP2_exercice1 PRIVATE PROJECT_ROOT="${CMAKE_SOURCE_DIR}/")

#add_executable(TP2_exercice2 "src/TP2/exercice2.cpp")
#target_link_libraries(TP2_exercice2 PRIVATE LavaCake::LavaCake glfw)
#target_compile_definitions(TP2_exercice2 PRIVATE PROJECT_ROOT="${CMAKE_SOURCE_DIR}/")

#add_executable(TP2_exercice3 "src/TP2/exercice3.cpp")
#target_link_libraries(TP2_exercice3 PRIVATE LavaCake::LavaCake glfw glm::glm)
#target_compile_definitions(TP2_exercice3 PRIVATE PROJECT_ROOT="${CMAKE_SOURCE_DIR}/")

#add_executable(TP2_exercice4 "src/TP2/exercice4.cpp")
#target_link_libraries(TP2_exercice4 PRIVATE LavaCake::LavaCake glfw glm::glm)
#target_compile_definitions(TP2_exercice4 PRIVATE PROJECT_ROOT="${CMAKE_SOURCE_DIR}/")

# TP3 - Normales
#add_executable(TP3_exercice1 "src/TP3/exercice1.cpp")
#target_link_libraries(TP3_exercice1 PRIVATE LavaCake::LavaCake glfw glm::glm tinyobjloader)
#target_include_directories(TP3_exercice1 PRIVATE ${CMAKE_SOURCE_DIR}/src)
#target_compile_definitions(TP3_exercice1 PRIVATE PROJECT_ROOT="${CMAKE_SOURCE_DIR}/")

#add_executable(TP3_exercice2 "src/TP3/exercice2.cpp")
#target_link_libraries(TP3_exercice2 PRIVATE LavaCake::LavaCake glfw glm::glm tinyobjloader)
#target_include_directories(TP3_exercice2 PRIVATE ${CMAKE_SOURCE_DIR}/src)
#target_compile_definitions(TP3_exercice2 PRIVATE PROJECT_ROOT="${CMAKE_SOURCE_DIR}/")

#add_executable(TP3_exercice3 "src/TP3/exercice3.cpp")
#target_link_libraries(TP3_exercice3 PRIVATE LavaCake::LavaCake glfw glm::glm tinyobjloader)
#target_include_directories(TP3_exercice3 PRIVATE ${CMAKE_SOURCE_DIR}/src)
#target_compile_definitions(TP3_exercice3 PRIVATE PROJECT_ROOT="${CMAKE_SOURCE_DIR}/")
