cmake_minimum_required(VERSION 3.24)

set(CMAKE_CXX_STANDARD 17)
project(
	phongLighting
	LANGUAGES CXX
)

# project options
option(USE_PRE_BUILT_LIB "Use pre-built libraries or custom build them" ON)

# GLFW options
option(GLFW_BUILD_EXAMPLES "Build the GLFW example programs" OFF)
option(GLFW_BUILD_TESTS "Build the GLFW test programs" OFF)
option(GLFW_BUILD_DOCS "Build the GLFW documentation" OFF)
option(GLFW_INSTALL "Generate installation target" OFF)
option(GLFW_DOCUMENT_INTERNALS "Include internals in documentation" OFF)

# Assimp options
option(BUILD_SHARED_LIBS "Build package with shared libraries." OFF)
option(ASSIMP_BUILD_FRAMEWORK "Build package as Mac OS X Framework bundle." OFF)
option(ASSIMP_DOUBLE_PRECISION "Set to ON to enable double precision processing" OFF)
option(ASSIMP_OPT_BUILD_PACKAGES "Set to ON to generate CPack configuration files and packaging targets" OFF)
option(ASSIMP_ANDROID_JNIIOSYSTEM "Android JNI IOSystem support is active" OFF)
option(ASSIMP_NO_EXPORT "Disable Assimp's export functionality." OFF)
option(ASSIMP_BUILD_ZLIB "Build your own zlib" OFF)
option(ASSIMP_BUILD_ASSIMP_TOOLS "If the supplementary tools for Assimp are built in addition to the library." OFF)
option(ASSIMP_BUILD_SAMPLES "If the official samples are built as well (need Glu)." OFF)
option(ASSIMP_BUILD_TESTS "If the test suite for Assimp is built in addition to the library." OFF)
option(ASSIMP_COVERALLS "Enable this to measure test coverage." OFF)
option(ASSIMP_INSTALL "Disable this if you want to use assimp as a submodule." ON)
option(ASSIMP_WARNINGS_AS_ERRORS "Treat all warnings as errors." OFF)
option(ASSIMP_ASAN "Enable AddressSanitizer." OFF)
option(ASSIMP_UBSAN "Enable Undefined Behavior sanitizer." OFF)
option(ASSIMP_BUILD_DOCS "Build documentation using Doxygen." OFF)
option(ASSIMP_INJECT_DEBUG_POSTFIX "Inject debug postfix in .a/.so/.dll lib names" ON)
option(ASSIMP_IGNORE_GIT_HASH "Don't call git to get the hash." OFF)


add_subdirectory(src)
if(NOT ${USE_PRE_BUILT_LIB})
	add_subdirectory(lib)
endif()


if(UNIX AND NOT APPLE)
	set(LINUX TRUE)
endif()

if(MSVC)
	target_compile_options(${PROJECT_NAME} PUBLIC "/W4;/analyze;/MP;")
elseif(GNU OR Clang)
	set(GCC_CLANG_COMPILE_OPTIONS "-Wall;-Wextra;-Wpedantic;-Wconversion;-Wshadow;")
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
	"lib/assimp/include/"

	# includes for binaries
	"binaries/assimp/include/"
	"binaries/assimp/contrib/zlib"

	${Vulkan_INCLUDE_DIR}
)

set(
	DEBUG_BUILD_LIB
	"${CMAKE_SOURCE_DIR}/binaries/glfw/glfw3.lib"
	"${CMAKE_SOURCE_DIR}/binaries/imgui/imgui.lib"
	"${CMAKE_SOURCE_DIR}/binaries/spdlog/spdlogd.lib"
	"${CMAKE_SOURCE_DIR}/binaries/assimp/assimpd.lib"
	"${CMAKE_SOURCE_DIR}/binaries/assimp/contrib/zlib/zlibstaticd.lib"
)
set(
	RELEASE_BUILD_LIB
	"${CMAKE_SOURCE_DIR}/binaries/glfw/glfw3.lib"
	"${CMAKE_SOURCE_DIR}/binaries/imgui/imgui.lib"
	"${CMAKE_SOURCE_DIR}/binaries/spdlog/spdlog.lib"
	"${CMAKE_SOURCE_DIR}/binaries/assimp/assimp.lib"
	"${CMAKE_SOURCE_DIR}/binaries/assimp/contrib/zlib/zlibstatic.lib"
)


if(CMAKE_BUILD_TYPE MATCHES Debug AND ${USE_PRE_BUILT_LIB})
	set(BUILD_LIB ${DEBUG_BUILD_LIB})
elseif(CMAKE_BUILD_TYPE MATCHES Release AND ${USE_PRE_BUILT_LIB})
	set(BUILD_LIB ${RELEASE_BUILD_LIB})
else()
	set(
		BUILD_LIB

		glfw
		spdlog::spdlog
		assimp
		imgui
	)
endif()

target_link_libraries(
	${PROJECT_NAME}
	${BUILD_LIB}
	${Vulkan_LIBRARY}
)
