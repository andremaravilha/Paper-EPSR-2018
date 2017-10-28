cmake_minimum_required(VERSION 3.8)

# ==============================================================================
# Project info

project(maneuvers-scheduling)


# ==============================================================================
# C++ standard and settings

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)


# ==============================================================================
# External dependencies

# Gurobi (path)
if (NOT DEFINED GUROBI_PATH)
    set(GUROBI_PATH "/opt/gurobi")
endif()

# Gurobi (libraries)
if (NOT DEFINED GUROBI_LIBRARY)
    set(GUROBI_LIBRARY gurobi75)
endif()


# ==============================================================================
# Paths to search for headers

include_directories(
        ${PROJECT_SOURCE_DIR}/lib/
        ${GUROBI_PATH}/include/)


# ==============================================================================
# Paths to search for libraries

link_directories(
        ${GUROBI_PATH}/lib/)


# ==============================================================================
# Libraries to link

set(GUROBI_LIBS gurobi_c++ ${GUROBI_LIBRARY})
set(OTHER_LIBS  pthread m)


# ==============================================================================
# Source files

set(SOURCE_FILES
        src/main.cpp
        src/problem.cpp src/problem.h
        src/algorithm.h
        src/neighborhood.cpp src/neighborhood.h
        src/local_search.cpp src/local_search.h
        src/heur_greedy.cpp src/heur_greedy.h
        src/heur_ils.cpp src/heur_ils.h
        src/mip_precedence.cpp src/mip_precedence.h
        src/mip_linear_ordering.cpp src/mip_linear_ordering.h
        src/utils.cpp src/utils.h)


# ==============================================================================
# Targets

add_executable(schd ${SOURCE_FILES})
target_link_libraries(schd ${GUROBI_LIBS} ${OTHER_LIBS})