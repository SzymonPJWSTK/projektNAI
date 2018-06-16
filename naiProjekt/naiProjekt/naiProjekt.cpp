// naiProjekt.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

void DrawLine(VideoCapture camera, int imgwidth, int imgheight, int loRange[], int hiRange[])
{
	vector < Point > path;

	int x2 = 0;
	int	y2 = 0;

	while (waitKey(1) != 27)
	{
		Mat frame, imgOrg;
		camera >> frame;
		flip(frame, frame, 1);
		imgOrg = frame.clone();

		cvtColor(frame, frame, COLOR_BGR2HSV);
		inRange(frame, Scalar(loRange[0], loRange[1], loRange[2]), Scalar(hiRange[0], hiRange[1], hiRange[2]), frame);

		morphologyEx(frame, frame, CV_MOP_CLOSE, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));

		vector < vector < Point > > contours; //kontury obrazu
		findContours(frame, contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);

		Moments oMoments = moments(frame);

		double dM01 = oMoments.m01;
		double dM10 = oMoments.m10;
		double dArea = oMoments.m00;

		//obiekt jakiś sensownych rozmiarów (a nie dziurka w nosie)
		if (dArea > 100000)
		{
			//wyznaczanie środka przedmiotu pojawiającego się w coknie frame
			int x1 = dM10 / dArea;
			int y1 = dM01 / dArea;

			path.push_back({ x1, y1 });

			//potrzebne będzie jakieś wygładzanie tych lini bo od punktów się zaroi
			circle(imgOrg, Point(x1, y1), 7, Scalar(255, 0, 0), 1); //narysowanie niebieskiego kółka na środku śledzonego przedmiotu
		}
		polylines(imgOrg, { path }, false, Scalar(0, 255, 0), 2);


		imshow("ustawienia", frame);
		imshow("kamera", imgOrg);
	}
}

int main()
{
	VideoCapture camera(0);

	int loRange[3] = { 170,150,60 };
	int hiRange[3] = { 179,255,255 };

	namedWindow("ustawienia", CV_WINDOW_AUTOSIZE);
	createTrackbar("loRange0", "ustawienia", &(loRange[0]), 255);
	createTrackbar("loRange1", "ustawienia", &(loRange[1]), 255);
	createTrackbar("loRange2", "ustawienia", &(loRange[2]), 255);
	createTrackbar("hiRange0", "ustawienia", &(hiRange[0]), 255);
	createTrackbar("hiRange1", "ustawienia", &(hiRange[1]), 255);
	createTrackbar("hiRange2", "ustawienia", &(hiRange[2]), 255);

	DrawLine(camera, 512, 382, loRange, hiRange);

	return 0;
}
