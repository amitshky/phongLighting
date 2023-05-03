#include "core/core.h"
#include "core/application.h"


int main()
{
	Application* app = Application::Create("Phong Lighting");
	app->Run();
	delete app;
}