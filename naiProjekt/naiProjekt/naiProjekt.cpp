// naiProjekt.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <string>
#include <iostream>
#include <opencv2/opencv.hpp>
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/objdetect.hpp"

using namespace cv;
using namespace std;

double min_face_size = 20;
double max_face_size = 200;
Mat mask;
Mat pieklo;

Mat ChangeFacenBackground(Mat frame, Point center, Size faceSize)
{
	Mat resizedMask, resizedFlame;
	resize(mask, resizedMask, faceSize);
	resize(pieklo, resizedFlame, Size(frame.cols, frame.rows));

	Rect maskPosition(center.x - faceSize.width / 2, center.y - faceSize.width / 2, faceSize.width, faceSize.width);

	resizedMask.copyTo(resizedFlame(maskPosition));

	return resizedFlame;
}

void GoToHell(VideoCapture camera)
{
	Mat frame;
	CascadeClassifier face_cascade("lbpcascade_frontalface.xml");

	while (waitKey(1) != 27)
	{
		camera >> frame;

		std::vector<Rect> faces;

		face_cascade.detectMultiScale(frame, faces, 1.1, 2, 0 | CV_HAAR_SCALE_IMAGE, Size(min_face_size, min_face_size), Size(max_face_size, max_face_size));

		for (int i = 0; i < faces.size(); i++)
		{   
			min_face_size = faces[i].width*0.7;
			max_face_size = faces[i].width*1.5;
			Point center(faces[i].x + faces[i].width*0.5, faces[i].y + faces[i].height*0.5);
			frame = ChangeFacenBackground(frame, center, Size(faces[i].width, faces[i].height));
		}
	
		flip(frame, frame, 1);
		imshow("Piekło", frame);
	}
}

bool Condition(vector <Point2f> pathSmooth, vector <Point> & path)
{
	vector <Point> itr(pathSmooth.end() - 6, pathSmooth.end());
	int conditions = 0;

	if (itr[1].x - itr[0].x > 0 && itr[1].y - itr[0].y > 0)
	{
		cout << "jeden" << endl;
		conditions++;
	}

	if (itr[2].x - itr[1].x > 0 && ::abs(itr[0].y - itr[2].y) <= 50)
	{
		cout << "dwa" << endl;
		conditions++;
	}

	if (itr[2].x - itr[3].x > 0 && itr[3].y - itr[2].y > 0)
	{
		cout << "trzy" << endl;
		conditions++;
	}

	if ((::abs(itr[4].x - itr[3].x) > 0  && (::abs(itr[4].y - itr[3].y) <= 50)))
	{
		cout << "cztery" << endl;
		conditions++;

	}

	if (::abs(itr[5].x - itr[0].x) <= 5 && ::abs(itr[5].y - itr[0].y) <= 5)
	{
		cout << "piec" << endl;
		conditions++;
	}

	if (conditions == 5)
	{
		path.clear();
		return true;
	}

	return false;
}

bool ConditionZ(vector <Point2f> pathSmooth, vector <Point> & path) 
{
	vector <Point> itr(pathSmooth.end() - 4, pathSmooth.end());
	int conditions = 0;

	if (::abs(itr[0].x - itr[1].x) > 5 && (::abs(itr[0].y - itr[1].y) < 5)) {
		cout << "JedenZ";
		conditions++;
	}
	if ((::abs(itr[0].x - itr[2].x) < 50) && (::abs(itr[1].y - itr[2].y) > 5)) {
		cout << "DwaZ";

		conditions++;
	}
	if (::abs(itr[2].x - itr[3].x) > 5 && (::abs(itr[2].y - itr[3].y) < 10)) {
		cout << "TrzyZ";

		conditions++;
	}


	if (conditions == 3)
		return true;

	return false;
}

bool DrawLine(VideoCapture camera, int imgwidth, int imgheight, int loRange[], int hiRange[])
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
			approxPolyDP(vector<Point2f>(path.begin(), path.end()), pathSmooth, 60, false);		// wygładzenie lini

			smoothedPath.clear();
			for (auto &point : pathSmooth)														//kopia wygładzonej lini do wektora punktów (żeby później wyświetlić na jego podstawie linie)
				smoothedPath.push_back({ (int)point.x, (int)point.y });

			circle(imgOrg, Point(x1, y1), 7, Scalar(255, 0, 0), 1); //narysowanie niebieskiego kółka na środku śledzonego przedmiotu

			if (waitKey(1) == 32)
				path.clear();

			
			if (pathSmooth.size() >= 4 && pathSmooth.size() < 6)
			{
				if (ConditionZ(pathSmooth, path))
					return false;
			}

			if (pathSmooth.size() >= 6)
			{
				if (Condition(pathSmooth, path))
				{
					destroyWindow("kamera");
					destroyWindow("ustawienia");
					return true;
				}
			}
		}

			polylines(imgOrg, { smoothedPath }, false, Scalar(0, 0, 255), 2);
			imshow("ustawienia", frame);
			imshow("kamera", imgOrg);
	}

	return false;
}

int main()
{
	VideoCapture camera(0);
	mask = imread("Devil.png");
	pieklo = imread("piekielko.jpg");

	int loRange[3] = { 0,168,94};
	int hiRange[3] = { 255,255,255 };

	namedWindow("ustawienia", CV_WINDOW_AUTOSIZE);
	createTrackbar("loRange0", "ustawienia", &(loRange[0]), 255);
	createTrackbar("loRange1", "ustawienia", &(loRange[1]), 255);
	createTrackbar("loRange2", "ustawienia", &(loRange[2]), 255);
	createTrackbar("hiRange0", "ustawienia", &(hiRange[0]), 255);
	createTrackbar("hiRange1", "ustawienia", &(hiRange[1]), 255);
	createTrackbar("hiRange2", "ustawienia", &(hiRange[2]), 255);
	
	bool isPentagramShown = DrawLine(camera, 512, 382, loRange, hiRange);

	if (isPentagramShown)
		GoToHell(camera);

	return 0;
}