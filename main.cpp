#include <easyx.h>
#include <iostream>
#include "common.h"
using namespace std;

int main()
{
	initgraph(WINDOW_WIDTH, WINDOW_HEIGHT);

	ExMessage msg;
	
	BeginBatchDraw();

	while (running)
	{
		DWORD start_time = GetTickCount();

		while (peekmessage(&msg))
		{
		
		}

		cleardevice();
		FlushBatchDraw();

		DWORD end_time = GetTickCount();
		DWORD delta_time = end_time - start_time;
		if (delta_time < 1000 / 144)
		{
			Sleep(1000 / 144 - delta_time);
		}
	}

	EndBatchDraw();

	return 0;
}