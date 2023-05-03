cmake_minimum_required(VERSION 3.24)

set(CMAKE_CXX_STANDARD 17)
project(
	phongLighting
	LANGUAGES CXX
)


add_subdirectory(src)
add_subdirectory(lib)


if(UNIX AND NOT APPLE)
	set(LINUX TRUE)
endif()

if(MSVC)
	target_compile_options(${PROJECT_NAME} PUBLIC "/W4")
elseif(GNU OR Clang)
	set(GCC_CLANG_COMPILE_OPTIONS "-wall;-Werror;-wextra;-wpedantic;-wconversion;-wshadow;")
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
	${Vulkan_INCLUDE_DIR}
)

target_link_libraries(
	${PROJECT_NAME}
	glfw
	fmt::fmt
	${Vulkan_LIBRARY}
)
