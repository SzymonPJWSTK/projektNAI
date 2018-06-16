// naiProjekt.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

int main()
{
	VideoCapture camera(0);
	
	while (waitKey(1) != 27)
	{
		Mat filter;
		camera >> filter;
		flip(filter, filter, 1);
		resize(filter, filter, { 512, 382 });
		imshow("obraz", filter);
	}

    return 0;
}

