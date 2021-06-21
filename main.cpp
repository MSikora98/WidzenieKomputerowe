#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<opencv2/core/core.hpp>
#include<opencv2/opencv.hpp>

#include<iostream>
#include<conio.h>
#include<vector>
#include<math.h>
#include <windows.h>
#include <winbase.h>
#include <time.h>

// Maciej Sikora 260965
/////////////////////////////////
//Klawiszologia:
//Spacja - opcjonalna aktualizacja tła.
///////////////////////////////////

using namespace cv;
using namespace std;

class SuspiciousObject {
	public:
		Point objPosition;
		Point ownerPosition;
		time_t stopMovement;
		bool isNew;
		bool haveOwner;
		bool isStatic;
};

int simplifiedDistance(Point first, Point second) {
	int xDist = abs(first.x - second.x);
	int yDist = abs(first.y - second.y);
	if (xDist > yDist) {
		return xDist;
	}
	else {
		return yDist;
	}
}

void AlarmOn(Mat img, double width){
	putText(img, "ALARM!!!", Point(width - 200, 55), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(0, 0, 255));
}

void showCurrentTime(SYSTEMTIME time, double height, Mat& img){
	GetLocalTime(&time);
	string dateAndTime = to_string(time.wDay) + ":" + to_string(time.wMonth) + ":" + to_string(time.wYear) + " " + to_string(time.wHour) + ":" + to_string(time.wMinute) + ":" + to_string(time.wSecond);
	putText(img, dateAndTime, Point(10, height), FONT_HERSHEY_SIMPLEX, 0.6, Scalar(0, 255, 0));	
}

int toAlarmTime(time_t startTime) {
	int showSec = 0;
	time_t currentTime;
	time(&currentTime);
	showSec = difftime(currentTime, startTime);
	return showSec;
}

void showSizeDetection(double height, Mat img, int minS, int maxS){
	line(img, Point(10, height - 30), Point(maxS + 10, height - 30), Scalar(255, 0, 0), 3);
	line(img, Point(minS + 10, height - 25), Point(minS + 10, height - 35), Scalar(255, 0, 0), 3);
	line(img, Point(maxS + 10, height - 20), Point(maxS + 10, height - 40), Scalar(255, 0, 0), 3);
}

void showCenter(Mat img, Point centerPoint)
{
	string coordsToString = "x = " + to_string(centerPoint.x) + ", y = " + to_string(centerPoint.y);
	line(img, Point(centerPoint.x - 15, centerPoint.y), Point(centerPoint.x + 15, centerPoint.y), Scalar(255, 0, 0));
	line(img, Point(centerPoint.x, centerPoint.y - 15), Point(centerPoint.x, centerPoint.y + 15), Scalar(255, 0, 0));
	putText(img, coordsToString, Point(centerPoint.x, centerPoint.y + 20), FONT_HERSHEY_SIMPLEX, 0.3, Scalar(0, 0, 255));
}

void drawRectangle(vector<Point> contour, Mat img, int minS, int maxS, vector<SuspiciousObject>& susObjects, vector<Point>& centers){
	Rect borderRect = boundingRect(contour);
	SuspiciousObject newSuspiciousObject;
	
	if((minS <= borderRect.width) && (minS <= borderRect.height)){
		Point centerOfRect = Point ((borderRect.width / 2) + borderRect.tl().x, (borderRect.height / 2) + borderRect.tl().y);
		showCenter(img, centerOfRect);
		if((maxS >= borderRect.width) && (maxS >= borderRect.height)){
			
			rectangle(img, borderRect, Scalar(255, 0, 0), 2);
			newSuspiciousObject.objPosition = Point(centerOfRect);
			newSuspiciousObject.isNew = true;
			newSuspiciousObject.isStatic = false;
			time(&newSuspiciousObject.stopMovement);
			susObjects.push_back(newSuspiciousObject);

		}
		else{
			rectangle(img, borderRect, Scalar(0, 255, 0));
			centers.push_back(centerOfRect);
		}
	}
}

vector<SuspiciousObject> updateSuspicious(vector<SuspiciousObject> oldSus, vector<SuspiciousObject> newSus, vector<Point> possibleOwners){
	vector<SuspiciousObject> temp;
	Point owner;
	int oldDistance;
	SuspiciousObject newSuspiciousObject;
	temp.clear();
	bool found;
	for( size_t i = 0; i < newSus.size(); i++ ){
		found = false;
		oldDistance = 100000;
		for( size_t j = 0; (j < oldSus.size()) && (!found); j++ ){
			if(simplifiedDistance(newSus[i].objPosition, oldSus[j].objPosition) <= 1){
				newSuspiciousObject.objPosition = newSus[i].objPosition;
				newSuspiciousObject.ownerPosition = oldSus[j].ownerPosition;
				newSuspiciousObject.stopMovement = oldSus[j].stopMovement;
				newSuspiciousObject.haveOwner = oldSus[j].haveOwner;
				newSuspiciousObject.isNew = false;
				newSuspiciousObject.isStatic = true;
				temp.push_back(newSuspiciousObject);
				found = true;
			}
			else if(simplifiedDistance(newSus[i].objPosition, oldSus[j].objPosition) < oldDistance){
				newSuspiciousObject.objPosition = newSus[i].objPosition;
				newSuspiciousObject.ownerPosition = oldSus[j].ownerPosition;
				newSuspiciousObject.stopMovement = oldSus[j].stopMovement;
				newSuspiciousObject.haveOwner = oldSus[j].haveOwner;
				oldDistance = simplifiedDistance(newSus[i].objPosition, oldSus[j].objPosition);
			}
		}
		if(oldDistance < 10 && !found){
			newSuspiciousObject.isNew = false;
			newSuspiciousObject.isStatic = false;
			time(&newSuspiciousObject.stopMovement);
			temp.push_back(newSuspiciousObject);
			found = true;
		}
		if(!found){
			newSuspiciousObject.objPosition = newSus[i].objPosition;
			newSuspiciousObject.ownerPosition = newSus[i].ownerPosition;
			newSuspiciousObject.stopMovement = newSus[i].stopMovement;
			newSuspiciousObject.haveOwner = newSus[i].haveOwner;
			newSuspiciousObject.isNew = true;
			newSuspiciousObject.isStatic = false;
			time(&newSuspiciousObject.stopMovement);
			temp.push_back(newSuspiciousObject);
		}
	}
	for (size_t i = 0; i < temp.size(); i++) {
		if (temp[i].isNew) {
			if (possibleOwners.size() > 0) {
				owner = Point(possibleOwners[0]);
				oldDistance = simplifiedDistance(temp[i].objPosition, owner);
				for (size_t j = 1; j < possibleOwners.size(); j++) {
					if (simplifiedDistance(temp[i].objPosition, possibleOwners[j]) < oldDistance) {
						owner = Point(possibleOwners[j]);
						oldDistance = simplifiedDistance(temp[i].objPosition, owner);
					}
				}
				if (oldDistance < 30) {
					temp[i].ownerPosition = Point(owner);
					temp[i].haveOwner = true;
				}
				else {
					temp[i].ownerPosition = Point(temp[i].objPosition);
					temp[i].haveOwner = false;
				}
			}
			else {
				temp[i].ownerPosition = Point(temp[i].objPosition);
				temp[i].haveOwner = false;
			}
		}
		else {
			if (possibleOwners.size() > 0) {
				owner = Point(possibleOwners[0]);
				oldDistance = simplifiedDistance(temp[i].ownerPosition, owner);
				for (size_t j = 1; j < possibleOwners.size(); j++) {
					if (simplifiedDistance(temp[i].ownerPosition, possibleOwners[j]) < oldDistance) {
						owner = Point(possibleOwners[j]);
						oldDistance = simplifiedDistance(temp[i].ownerPosition, owner);
					}
				}
				if (simplifiedDistance(temp[i].ownerPosition, owner) < 30) {
					temp[i].ownerPosition = Point(owner);
					temp[i].haveOwner = true;
				}
				else {
					temp[i].haveOwner = false;
				}
			}
			else {
				temp[i].haveOwner = false;
			}
		}
	}
	return temp;
}

void showOwnerRelations(Mat img, vector<SuspiciousObject> susp){
	for( size_t i = 0; i < susp.size(); i++ ){
		if(susp[i].haveOwner && susp[i].isStatic){
			line(img, susp[i].objPosition, susp[i].ownerPosition, Scalar(0, 0, 255));
		}
	}
}

void showIdleTime(Mat img, vector<SuspiciousObject> susp, double width){
	int showSec;
	int showMin;
	
	for( size_t i = 0; i < susp.size(); i++ ){
		if(susp[i].isStatic){
			showSec = 0;
			showMin = 0;
			showSec = toAlarmTime(susp[i].stopMovement);
			while(showSec > 59){
				showSec = showSec - 60;
				showMin = showMin + 1;
			}
			if(showSec > 5 && !susp[i].haveOwner){
				AlarmOn(img, width);
			}
			string idleTime = to_string(showMin) + ":" + to_string(showSec);
			putText(img, idleTime, susp[i].objPosition, FONT_HERSHEY_SIMPLEX, 0.3, Scalar(0, 255, 0));
		}
	}
}

//
int main() {
	Mat image;

	VideoCapture cap;

	SYSTEMTIME clock;
	GetLocalTime(&clock);
    

	//cap.open("LeftBag.mpg");
	//cap.open("LeftBag_AtChair.mpg");
	cap.open("LeftBag_PickedUp.mpg");
	//cap.open("LeftBox.mpg");

	//LeftBag.mpg
	//LeftBag_AtChair.mpg
	//LeftBag_PickedUp.mpg
	//LeftBox.mpg
	
	namedWindow("Obraz zrodlowy", WINDOW_AUTOSIZE);
	namedWindow("Suwaki", WINDOW_AUTOSIZE);
	
	resizeWindow("Suwaki", 1000, 300);

	cap.set(CAP_PROP_FRAME_WIDTH, 640);
	cap.set(CAP_PROP_FRAME_HEIGHT, 480);

	double dWidth = cap.get(cv::CAP_PROP_FRAME_WIDTH);
	double dHeight = cap.get(cv::CAP_PROP_FRAME_HEIGHT);

	unsigned char k = 0;
	int minObjectSize = 5;
	int maxObjectSize = 12;
	int trsh = 32;

	createTrackbar("Min. wlk.", "Suwaki", &minObjectSize, 300);
	createTrackbar("Max wlk.", "Suwaki", &maxObjectSize, 1000);
	createTrackbar("Threshold", "Suwaki", &trsh, 255);


	if (!cap.isOpened()) {
		return 0;
	}

	cap >> image;
	
	Mat imageClone;
	Mat grayImage;
	
	Mat backgroundImage;
	Mat backgroundGray;
	Mat backgroundBoundaries;
	
	
	Mat diffImage;
	Mat blurDiffImage;
	Mat threshImage;
	
	int currentFrame = 0;
	
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	
	vector<SuspiciousObject> suspiciousObjects;
	vector<Point> centerPoints;
	
	vector<SuspiciousObject> newSusObjects;
	vector<Point> newCenterPoints;

	vector<SuspiciousObject> tempor;


	bool firstFrame = true;

	while (cap.read(image)) {
		
		if(firstFrame){
			backgroundImage = imread("background.png", IMREAD_COLOR);
			cvtColor(backgroundImage, backgroundGray, COLOR_BGR2GRAY );
			backgroundBoundaries = imread("backgroundMask.png", IMREAD_GRAYSCALE);
			threshold(backgroundBoundaries, backgroundBoundaries, 1, 255, THRESH_BINARY);
			firstFrame = false;
		}
		
		k = waitKey(1);
		if (k == 27) {
			break;
		}
		if (k == 32) {
			backgroundImage = image.clone();
			cvtColor(backgroundImage, backgroundGray, COLOR_BGR2GRAY );
		}
		if (k == 112){
		}
		if (k == 115){
		}

		if (currentFrame >= cap.get(CAP_PROP_FRAME_COUNT) - 10) {
			currentFrame = 0;
			cap.set(CAP_PROP_POS_FRAMES, 0);
		}

		
		imageClone = image.clone();
		cvtColor(imageClone, grayImage, COLOR_BGR2GRAY);
		
		diffImage = image.clone();
		absdiff(grayImage, backgroundGray, diffImage);
		
		GaussianBlur(diffImage, blurDiffImage, Size(3, 3), 3);

		threshold(blurDiffImage, threshImage, trsh, 255, THRESH_BINARY);
		
		dilate(threshImage, threshImage, (10, 10), Point(-1, -1), 5);
		erode(threshImage, threshImage, (10, 10), Point(-1, -1), 5);

		erode(threshImage, threshImage, (10, 10), Point(-1, -1), 5);
		dilate(threshImage, threshImage, (10, 10), Point(-1, -1), 5);
		
		bitwise_and(threshImage, backgroundBoundaries, threshImage);
		
		findContours(threshImage, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_NONE);
		newSusObjects.clear();
		newCenterPoints.clear();
		for( size_t i = 0; i < contours.size(); i++ )
		{
			drawRectangle(contours[i], imageClone, minObjectSize, maxObjectSize, newSusObjects, newCenterPoints);
		}
		centerPoints = newCenterPoints;
		tempor = updateSuspicious(suspiciousObjects, newSusObjects, newCenterPoints);
		suspiciousObjects = tempor;
		showOwnerRelations(imageClone, suspiciousObjects);
	
		showCurrentTime(clock, 20, imageClone);
		showSizeDetection(dHeight, imageClone, minObjectSize, maxObjectSize);

		showIdleTime(imageClone, suspiciousObjects, dWidth);
		
		imshow("Obraz zrodlowy", imageClone);
		//imshow("BinarnyRuchu", threshImage);
		//imshow("Tlo", backgroundImage);

		waitKey(15);

		currentFrame += 1;
	}
	destroyAllWindows();
	cap.release();
	return 0;


}