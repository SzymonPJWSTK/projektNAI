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

void detectAndDisplay(Mat frame);

String face_cascade_name = "lbpcascade_frontalface.xml";
String eyes_cascade_name = "haarcascade_eye_tree_eyeglasses.xml";
CascadeClassifier face_cascade;
CascadeClassifier eyes_cascade;
string window_name = "Capture - Face detection";
RNG rng(12345);
Mat glasses;


void PutMask(Mat mask, Mat frame, vector<Point> points)
{
	
}

void detectAndDisplay(Mat frame)
{
	std::vector<Rect> faces;
	Mat frame_gray;

	cvtColor(frame, frame_gray, CV_BGR2GRAY);
	equalizeHist(frame_gray, frame_gray);

	//-- Detect faces
	face_cascade.detectMultiScale(frame_gray, faces, 1.1, 2, 0, Size(12, 12)); //| CV_HAAR_SCALE_IMAGE, Size(30, 30));

	if (faces.size() > 0)
	{
		Point center(faces[0].x + faces[0].width*0.5, faces[0].y + faces[0].height*0.5);
		ellipse(frame, center, Size(faces[0].width*0.5, faces[0].height*0.5), 0, 0, 360, Scalar(255, 0, 255), 4, 8, 0);

		Mat faceROI = frame_gray(faces[0]);
		std::vector<Rect> eyes;

		//-- In each face, detect eyes
		vector <Point> eyesPosition;
		eyes_cascade.detectMultiScale(faceROI, eyes, 1.1, 2, 0 | CV_HAAR_SCALE_IMAGE, Size(30, 30));

		for (size_t j = 0; j < eyes.size(); j++)
		{
			Point center(faces[0].x + eyes[j].x + eyes[j].width*0.5, faces[0].y + eyes[j].y + eyes[j].height*0.5);
			int radius = cvRound((eyes[j].width + eyes[j].height)*0.25);
			circle(frame, center, radius, Scalar(255, 0, 0), 4, 8, 0);
			eyesPosition.push_back(center);
		}

		PutMask(glasses.clone(), frame, eyesPosition);
	}

	//-- Show what you got
	flip(frame, frame, 1);
	imshow(window_name, frame);
}

void ChangeBackground(VideoCapture camera) 
{
	Mat frame;

	while (waitKey(1) != 27)
	{
		camera >> frame;
		detectAndDisplay(frame);
	}
}

bool Condition(vector <Point2f> pathSmooth, vector <Point> & path)
{
	vector <Point> itr(pathSmooth.end() - 6, pathSmooth.end());
	int conditions = 0;

	if (itr[1].x - itr[0].x > 0 && itr[1].y - itr[0].y > 0)
		conditions++;

	if (itr[2].x - itr[1].x > 0 && itr[1].y - itr[2].y > 0)
		conditions++;

	if (itr[2].x - itr[3].x > 0 && itr[3].y - itr[2].y > 0)
		conditions++;

	if ((::abs(itr[4].x - itr[3].x) > 0 || ::abs(itr[4].x - itr[3].x) == 0) && (::abs(itr[4].y - itr[3].y) > 0 || itr[4].y - itr[3].y == 0))
		conditions++;

	if (itr[4].x - itr[5].x > 0 && itr[4].y - itr[5].y > 0)
		conditions++;

	if (conditions == 5)
	{
		path.clear();
		return true;
	}

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
			approxPolyDP(vector<Point2f>(path.begin(), path.end()), pathSmooth, 70, false);		// wygładzenie lini

			smoothedPath.clear();
			for (auto &point : pathSmooth)														//kopia wygładzonej lini do wektora punktów (żeby później wyświetlić na jego podstawie linie)
				smoothedPath.push_back({ (int)point.x, (int)point.y });

			circle(imgOrg, Point(x1, y1), 7, Scalar(255, 0, 0), 1); //narysowanie niebieskiego kółka na środku śledzonego przedmiotu

			if (waitKey(1) == 32)
				path.clear();

			if (pathSmooth.size() >= 6)
			{
				if (Condition(pathSmooth, path))
					return true;
			}
		}

			polylines(imgOrg, { smoothedPath }, false, Scalar(0, 255, 0), 2);
			imshow("ustawienia", frame);
			imshow("kamera", imgOrg);
	}

	return false;
}

int main()
{
	VideoCapture camera(0);

	if (!face_cascade.load(face_cascade_name)) { printf("--(!)Error loading\n"); return -1; };
	if (!eyes_cascade.load(eyes_cascade_name)) { printf("--(!)Error loading\n"); return -1; };
	glasses = imread( "Devil.png", -1 );

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
		ChangeBackground(camera);

	return 0;
}