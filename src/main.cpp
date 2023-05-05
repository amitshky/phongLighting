#include "core/core.h"
#include "core/logger.h"
#include "core/application.h"

int main()
{
	Logger::Init();

	Application* app = Application::Create("Phong Lighting");
	app->Run();
	delete app;
}