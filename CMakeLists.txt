cmake_minimum_required(VERSION 3.16.3)
cmake_policy(SET CMP0091 NEW)

project(mhr_charm_and_item_box_editor)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	set(CMAKE_CXX_FLAGS_DEBUG "/DEBUG")
	set(CMAKE_CXX_FLAGS_RELEASE "/O2")
	set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
else ()
	set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wno-unused-parameter -Wpedantic -no-pie")
	set(CMAKE_CXX_FLAGS_DEBUG "-g")
	set(CMAKE_CXX_FLAGS_RELEASE "-O3 -flto")
endif()

file(GLOB sources "${PROJECT_SOURCE_DIR}/src/*")
add_executable(${PROJECT_NAME} WIN32 ${sources})
add_subdirectory(${PROJECT_SOURCE_DIR}/Agro)
target_include_directories(${PROJECT_NAME} PUBLIC Agro/Agro/include)
target_link_libraries(${PROJECT_NAME} Agro)
set_target_properties(${PROJECT_NAME} PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}"
	RUNTIME_OUTPUT_DIRECTORY_DEBUG "${PROJECT_SOURCE_DIR}"
	RUNTIME_OUTPUT_DIRECTORY_RELEASE "${PROJECT_SOURCE_DIR}"
)
