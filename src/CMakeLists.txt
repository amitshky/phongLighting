add_executable(
	${PROJECT_NAME}

	main.cpp

	core/core.cpp
	core/application.cpp
	core/input.cpp
	core/window.cpp
	core/logger.cpp

	renderer/renderer.cpp
	renderer/vulkanContext.cpp
	renderer/device.cpp
	renderer/commandPool.cpp
	renderer/commandBuffer.cpp
	renderer/swapchain.cpp
	renderer/vertexBuffer.cpp
	renderer/indexBuffer.cpp
	renderer/uniformBuffer.cpp
	renderer/descriptor.cpp
	renderer/texture.cpp
	renderer/shader.cpp
	renderer/pipeline.cpp
	renderer/camera.cpp

	editor/ubo.cpp

	utils/utils.cpp
)
