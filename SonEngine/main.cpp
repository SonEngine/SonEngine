#include "VideoApp.h"
#include "SimpleApp.h"
#include "MultiThreadApp.h"


int main()
{
	//Core::SimpleApp app(1280, 720);
	//Core::MultiThreadApp app(256 * 2 + 100, 256 * 2, 100);
	//Core::MultiThreadApp app(1280 , 720, 0);
	//Core::MultiThreadApp app(1280, 720, 0);
	Core::MultiThreadApp app(1920 , 1080, 0);
	//Core::MultiThreadApp app(448 +200, 448, 200);
	//Core::VideoApp app(1280, 720);
	
	if (app.Initialize())
	{
		return app.Run();
	}
	return -1;
}