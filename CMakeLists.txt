cmake_minimum_required(VERSION 3.10)
project(CasualQuantBot)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Deps
find_package(Eigen3 3.3 REQUIRED NO_MODULE)
find_package(Boost REQUIRED COMPONENTS system)
find_package(OpenSSL REQUIRED)
find_package(nlohmann_json 3.2.0 REQUIRED)

# Includes
include_directories(
    src
    ${EIGEN3_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIR}
)

# Collect sources
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Build target
add_executable(CasualQuantBot ${SOURCES} "src/main.cpp")

# Link libs
target_link_libraries(CasualQuantBot
    Eigen3::Eigen
    Boost::system
    OpenSSL::SSL
    OpenSSL::Crypto
    nlohmann_json::nlohmann_json
)