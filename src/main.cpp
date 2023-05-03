#include "core/core.h"
#include "core/application.h"


int main()
{
	Application* app = new Application{ "Phong Lighting" };
	app->Run();
	delete app;
}