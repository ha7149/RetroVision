cmake_minimum_required(VERSION 3.20)
project(RetroVision CXX)

# Force modern language features
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Locate Package Configurations via vcpkg Toolchain
find_package(SDL3 CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(FFmpeg REQUIRED COMPONENTS avcodec avformat swscale swresample)

# 2. Declare Window Client Executable Target
add_executable(RetroVision 
	src/main.cpp
	src/ConfigManager.cpp
	src/Scheduler.cpp
	src/Decoder.cpp
	src/Renderer.cpp
	src/Filter.cpp
)

# 3. Explicitly include header directories
target_include_directories(RetroVision PRIVATE 
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${FFMPEG_INCLUDE_DIRS}
)

# 4. Explicitly link FFmpeg's library directiories (Production safegaurd)
target_link_directories(RetroVision PRIVATE ${FFMPEG_LIBRARY_DIRS})

# 3. Explicitly Bind Headers and Binary Linkages
target_link_libraries(RetroVision
	PRIVATE
		SDL3::SDL3
		nlohmann_json::nlohmann_json
		${FFMPEG_LIBRARIES}
		opengl32
		gdi32
		ws2_32
		winmm
		imm32
		version
		ole32
		oleaut32
		setupapi
		user32
)

if(NOT WIN32 OR MINGW)
	message(STATUS ">>> DETECTED MSYS2/MINGW: LINKING m.lib <<<")
	target_link_libraries(RetroVision PRIVATE m)
else()
	message(STATUS ">>> DETECTED NATIVE MSVC: SKIPPING m.lib <<<")
endif()


