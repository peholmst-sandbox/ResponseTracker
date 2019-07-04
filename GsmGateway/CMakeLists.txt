project(GsmGateway)
find_package(Boost 1.70.0 REQUIRED COMPONENTS system)
include_directories(${Base_SOURCE_DIR})
include_directories(${Boost_INCLUDE_DIRS})
add_executable(GsmGateway main.cpp)
target_link_libraries(GsmGateway ${Boost_LIBRARIES})

