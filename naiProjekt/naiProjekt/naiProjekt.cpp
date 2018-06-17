// naiProjekt.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;


void DrawLine(VideoCapture camera, int imgwidth, int imgheight, int loRange[], int hiRange[])
{
	vector < Point > path, smoothedPath;

	while (waitKey(1) != 27)
	{
		Mat frame, imgOrg;
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
			approxPolyDP(vector<Point2f>(path.begin(), path.end()), pathSmooth, 40, false);		// wygładzenie lini

			smoothedPath.clear();
			for (auto &point : pathSmooth)														//kopia wygładzonej lini do wektora punktów (żeby później wyświetlić na jego podstawie linie)
				smoothedPath.push_back({ (int)point.x, (int)point.y });

			circle(imgOrg, Point(x1, y1), 7, Scalar(255, 0, 0), 1); //narysowanie niebieskiego kółka na środku śledzonego przedmiotu
			polylines(imgOrg, { smoothedPath }, false, Scalar(0, 255, 0), 2);

			//rozpoznawanie kształtu
			if (smoothedPath.size() >= 4) {
				vector < Point > itr(smoothedPath.end() - 4, smoothedPath.end());
				int conditions = 0;
				double factor = (::abs(itr[0].x - itr[1].x) + ::abs(itr[0].y - itr[1].y)) * 2 / 3;


				if ((::abs(itr[0].x - itr[1].x) > factor) && (::abs(itr[0].y - itr[1].y) < factor)) {
					conditions++;
				}
				if ((::abs(itr[1].x - itr[2].x) > factor) && (::abs(itr[1].y - itr[2].y) > factor)) {
					conditions++;
				}
				if ((::abs(itr[2].x - itr[3].x) > factor) && (::abs(itr[2].y - itr[3].y) < factor)) {
					conditions++;
				}

				if (conditions == 3) {
					cout << "Jest Z!!!" << endl;

				//	polylines(imgOrg, { smoothedPath }, false, Scalar(0, 255, 0), 2);
					Mat objectArray;

					objectArray = imgOrg.clone();

					for (int i = 0; i < objectArray.rows; i++)
					{
						for (int j = 0; j < objectArray.row(i).cols; j++)
						{
							Vec3b color = objectArray.at<Vec3b>(Point(j, i));

							if (!(color[0] == 0 && color[1] == 255 && color[2] == 0))
							{
								color[0] = 0;
								color[1] = 0;
								color[2] = 0;
								objectArray.at<Vec3b>(Point(j, i)) = color;
							}
						}
					}


					imwrite("test.jpg", objectArray);
					path.clear();
				}
			}

		}

			

			//imshow("linia", objectArray);
			imshow("ustawienia", frame);
			imshow("kamera", imgOrg);
		
	}
}

int main()
{
	VideoCapture camera(0);

	int loRange[3] = { 0,192,65 };
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
