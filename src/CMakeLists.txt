add_executable(
	${PROJECT_NAME}
	main.cpp

	core/application.cpp
	core/input.cpp
	core/window.cpp
	core/logger.cpp

	renderer/renderer.cpp
	renderer/vulkanContext.cpp
	renderer/device.cpp

	utils/utils.cpp
)
