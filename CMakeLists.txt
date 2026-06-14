cmake_minimum_required(VERSION 3.24)
project(VeloFX-SDK LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT WIN32)
    message(FATAL_ERROR "VeloFX SDK currently only supports Windows (MSVC x64). Support for Linux/macOS is planned for future releases.")
endif()

find_package(CURL REQUIRED)

add_library(velofx STATIC IMPORTED GLOBAL)

set_target_properties(velofx PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

set_target_properties(velofx PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/lib/windows/velofx.lib"
)


if(TARGET CURL::libcurl)
    target_link_libraries(velofx INTERFACE CURL::libcurl)
else()
    target_link_libraries(velofx INTERFACE libcurl)
endif()