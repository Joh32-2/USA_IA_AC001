cmake_minimum_required(VERSION 3.5)

project(USA_IA_ACO01 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenMP REQUIRED)

add_executable(USA_IA_ACO01 main.cpp)

if(OpenMP_CXX_FOUND)
    target_link_libraries(USA_IA_ACO01 PRIVATE OpenMP::OpenMP_CXX)
endif()

include(GNUInstallDirs)
install(TARGETS USA_IA_ACO01
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
