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

	ui/imGuiOverlay.cpp

	utils/utils.cpp

	../lib/imgui/imgui.cpp
	../lib/imgui/imgui_demo.cpp
	../lib/imgui/imgui_draw.cpp
	../lib/imgui/imgui_tables.cpp
	../lib/imgui/imgui_widgets.cpp
	../lib/imgui/backends/imgui_impl_glfw.cpp
	../lib/imgui/backends/imgui_impl_vulkan.cpp
)
