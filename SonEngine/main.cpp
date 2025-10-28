#include "VideoApp.h"
#include "SimpleApp.h"
#include "MultiThreadApp.h"


int main()
{
	Core::SimpleApp app(1280, 720);
	//Core::MultiThreadApp app(1280, 720);
	//Core::VideoApp app(1280, 720);
	
	if (app.Initialize())
	{
		return app.Run();
	}
	return -1;
}