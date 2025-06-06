#include "SimpleApp.h"

int main()
{
	Core::SimpleApp app(1920, 1080);

	if (app.Initialize())
	{
		return app.Run();
	}
	return -1;
}