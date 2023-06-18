cmake_minimum_required(VERSION 3.24)

set(CMAKE_CXX_STANDARD 17)
project(
	phongLighting
	LANGUAGES CXX
)


# GLFW
option(GLFW_BUILD_EXAMPLES "Build the GLFW example programs" OFF)
option(GLFW_BUILD_TESTS "Build the GLFW test programs" OFF)
option(GLFW_BUILD_DOCS "Build the GLFW documentation" OFF)
option(GLFW_INSTALL "Generate installation target" OFF)
option(GLFW_DOCUMENT_INTERNALS "Include internals in documentation" OFF)

# ImGui
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DVK_PROTOTYPES")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_PROTOTYPES")

add_subdirectory(src)
add_subdirectory(lib)


if(UNIX AND NOT APPLE)
	set(LINUX TRUE)
endif()

if(MSVC)
	target_compile_options(${PROJECT_NAME} PUBLIC "/W4;/WX;/analyze;/MP;")
elseif(GNU OR Clang)
	set(GCC_CLANG_COMPILE_OPTIONS "-Wall;-Werror;-Wextra;-Wpedantic;-Wconversion;-Wshadow;")
	set(GCC_CLANG_COMPILE_OPTIONS_DEBUG "-O0;-g;")
	set(GCC_CLANG_COMPILE_OPTIONS_RELEASE "-O3;")

	target_compile_options(${PROJECT_NAME} PUBLIC ${GCC_CLANG_COMPILE_OPTIONS})
	target_compile_options(${PROJECT_NAME} PUBLIC $<$<CONFIG:Debug>:${GCC_CLANG_COMPILE_OPTIONS_DEBUG}>)
	target_compile_options(${PROJECT_NAME} PUBLIC $<$<CONFIG:Release>:${GCC_CLANG_COMPILE_OPTIONS_RELEASE}>)

	if(LINUX)
		target_link_libraries(${PROJECT_NAME} pthreads)
	endif()
endif()


find_package(Vulkan REQUIRED)


target_include_directories(
	${PROJECT_NAME}
	PUBLIC
	"src/"
	"lib/"
	"lib/glm/"
	"lib/glfw/include/"
	"lib/fmt/include/"
	"lib/spdlog/include/"
	"lib/imgui/"
	${Vulkan_INCLUDE_DIR}
)

target_link_libraries(
	${PROJECT_NAME}
	glfw
	spdlog::spdlog
	# imgui
	${Vulkan_LIBRARY}
)
