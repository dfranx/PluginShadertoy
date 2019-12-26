cmake_minimum_required(VERSION 3.1)
project(Shadertoy)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ./bin)

# source code
set(SOURCES
	dllmain.cpp
	Shadertoy.cpp

# libraries
	libs/json11/json11.cpp
	libs/pugixml/src/pugixml.cpp
	libs/imgui/imgui_draw.cpp
	libs/imgui/imgui_widgets.cpp
	libs/imgui/imgui.cpp
)

# cmake toolchain 
if(CMAKE_TOOLCHAIN_FILE)
    include(${CMAKE_TOOLCHAIN_FILE})
endif(CMAKE_TOOLCHAIN_FILE)

# openssl
find_package(OpenSSL REQUIRED)

# create executable
add_library(Shadertoy SHARED ${SOURCES})

set_target_properties(Shadertoy PROPERTIES OUTPUT_NAME "plugin")
set_target_properties(Shadertoy PROPERTIES PREFIX "")

# include directories
target_include_directories(Shadertoy PRIVATE ${OPENSSL_INCLUDE_DIR} libs inc)

target_link_libraries(Shadertoy ${OPENSSL_LIBRARIES})

if (NOT MSVC)
	target_compile_options(Shadertoy PRIVATE -Wno-narrowing)
endif()