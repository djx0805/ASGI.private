// test.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "GITest.h"
#include <iostream>


int main()
{
	GITest window;
	//
	window.Init(L"triangle test", false, 800, 600);
	window.Run();
	int a;
	std::cin >> a;
	return 0;
    return 0;
}

