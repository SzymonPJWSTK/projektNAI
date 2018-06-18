// naiProjekt.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include <opencv2/opencv.hpp>
#include <string>


using namespace cv;
using namespace std;

const char* image_window = "Source_Image";
const char* result_window = "Result_window";

void DrawLine(VideoCapture camera, int imgwidth, int imgheight, int loRange[], int hiRange[])
{
	vector < Point > path, smoothedPath;
	Mat imgOrg;

	while (waitKey(1) != 27)
	{
		Mat frame;
		camera >> frame;
		flip(frame, frame, 1);
		imgOrg = frame.clone();

		cvtColor(frame, frame, COLOR_BGR2HSV);
		inRange(frame, Scalar(loRange[0], loRange[1], loRange[2]), Scalar(hiRange[0], hiRange[1], hiRange[2]), frame);

		morphologyEx(frame, frame, CV_MOP_CLOSE, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));

		Moments oMoments = moments(frame);

		double dM01 = oMoments.m01;
		double dM10 = oMoments.m10;
		double dArea = oMoments.m00;

		//obiekt jakiś sensownych rozmiarów (a nie dziurka w nosie)
		if (dArea > 500000)
		{
			//wyznaczanie środka przedmiotu pojawiającego się w coknie frame
			int x1 = dM10 / dArea;
			int y1 = dM01 / dArea;

			path.push_back({ x1, y1 });															//zapamiętanie nowych punktów


			vector <Point2f> pathSmooth;
			approxPolyDP(vector<Point2f>(path.begin(), path.end()), pathSmooth, 70, false);		// wygładzenie lini

			smoothedPath.clear();
			for (auto &point : pathSmooth)														//kopia wygładzonej lini do wektora punktów (żeby później wyświetlić na jego podstawie linie)
				smoothedPath.push_back({ (int)point.x, (int)point.y });

			circle(imgOrg, Point(x1, y1), 7, Scalar(255, 0, 0), 1); //narysowanie niebieskiego kółka na środku śledzonego przedmiotu

			if (waitKey(1) == 32)
				path.clear();

			if (pathSmooth.size() >= 6)
			{
				vector <Point> itr(pathSmooth.end() - 6, pathSmooth.end());
				int conditions = 0;

				if (itr[1].x - itr[0].x > 0 && itr[1].y - itr[0].y > 0)
				{
					conditions++;
					cout << "jeden" << endl;
				}

				if (itr[2].x - itr[1].x > 0 && itr[1].y - itr[2].y > 0)
				{
					conditions++;
					cout << "dwa";
				}

				if (itr[2].x - itr[3].x > 0 && itr[3].y - itr[2].y > 0)
				{
					conditions++;
					cout << "trzy";
				}

				if ((::abs(itr[4].x - itr[3].x) > 0 || ::abs(itr[4].x - itr[3].x) == 0) && (::abs(itr[4].y - itr[3].y) > 0 || itr[4].y - itr[3].y == 0))
				{
					conditions++;
					cout << "cztery";
				}

				if (itr[4].x - itr[5].x > 0 && itr[4].y - itr[5].y > 0)
				{
					conditions++;
					cout << "piec";
				}

				if (conditions == 5)
				{
					cout << "Jest penti" << endl;
					path.clear();
				}
			}
		}

			polylines(imgOrg, { smoothedPath }, false, Scalar(0, 255, 0), 2);
			imshow("ustawienia", frame);
			imshow("kamera", imgOrg);
	}

	
}


int main()
{
	VideoCapture camera(0);

	int loRange[3] = { 0,168,94};
	int hiRange[3] = { 255,255,255 };

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
