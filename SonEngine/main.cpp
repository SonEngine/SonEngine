#include "VideoApp.h"
//#include "SimpleApp.h"

int main()
{
	//Core::SimpleApp app(200, 200);
	Core::VideoApp app(1280, 720);

	if (app.Initialize())
	{
		return app.Run();
	}
	return -1;
}