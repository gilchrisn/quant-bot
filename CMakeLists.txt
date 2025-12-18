cmake_minimum_required(VERSION 3.10)
project(CasualQuantBot)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Dependencies
# 1. Eigen3 (For Linear Algebra/Math)
find_package(Eigen3 3.3 REQUIRED NO_MODULE)

# 2. Boost (For Networking - System, Asio, Beast, SSL)
# Note: Ensure you have installed boost: sudo apt-get install libboost-all-dev
find_package(Boost REQUIRED COMPONENTS system)

# 3. OpenSSL (Required by Boost.Beast for HTTPS/WSS)
find_package(OpenSSL REQUIRED)

# 4. nlohmann_json (Standard JSON library)
find_package(nlohmann_json 3.2.0 REQUIRED)

# Include Directories
include_directories(
    src
    ${EIGEN3_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIR}
)

# Source Files
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Executable
add_executable(CasualQuantBot ${SOURCES} "src/main.cpp")

# Link Libraries
target_link_libraries(CasualQuantBot
    Eigen3::Eigen
    Boost::system
    OpenSSL::SSL
    OpenSSL::Crypto
    nlohmann_json::nlohmann_json
)