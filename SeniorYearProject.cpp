//objectTrackingTutorial.cpp

//Written by  Kyle Hounslow 2013

//Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software")
//, to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
//and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

//The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
//IN THE SOFTWARE.

#include <sstream>
#include <string>
#include <iostream>
#include <cmath>
#include <opencv\highgui.h>
#include <opencv\cv.h>
#include "mraa.hpp"
#include <unistd.h>
#include "l298.h"


using namespace std;
using namespace cv;
//initial min and max HSV filter values.
//these will be changed using trackbars
int H_MIN = 12;
int H_MAX = 169;
int S_MIN = 79;
int S_MAX = 254;
int V_MIN = 44;
int V_MAX = 256;
//default capture width and height
const int FRAME_WIDTH = 320;
const int FRAME_HEIGHT = 240;
//max number of objects to be detected in frame
const int MAX_NUM_OBJECTS=50;
//minimum and maximum object area
const int MIN_OBJECT_AREA = 20*20;
const int MAX_OBJECT_AREA = FRAME_HEIGHT*FRAME_WIDTH/1.5;


void findDistance(double a, double &distance){
	double r;
	double f=986.8783;
	double diameter=6.6;
	r=sqrt(a/3.1415);
	distance=(diameter*f)/(2*r);
	cout<<distance<<endl;
}

void morphOps(Mat &thresh){

	//create structuring element that will be used to "dilate" and "erode" image.
	//the element chosen here is a 3px by 3px rectangle

	Mat erodeElement = getStructuringElement( MORPH_RECT,Size(3,3));
    //dilate with larger element so make sure object is nicely visible
	Mat dilateElement = getStructuringElement( MORPH_RECT,Size(8,8));

	erode(thresh,thresh,erodeElement);
	erode(thresh,thresh,erodeElement);


	dilate(thresh,thresh,dilateElement);
	dilate(thresh,thresh,dilateElement);

}


void trackFilteredObject(int &x, int &y, Mat threshold, bool &objectFound, double &distance){

	Mat temp;
	threshold.copyTo(temp);
	//these two vectors needed for output of findContours
	vector< vector<Point> > contours;
	vector<Vec4i> hierarchy;
	//find contours of filtered image using openCV findContours function
	findContours(temp,contours,hierarchy,CV_RETR_CCOMP,CV_CHAIN_APPROX_SIMPLE );
	//use moments method to find our filtered object
	double refArea = 0;
	if (hierarchy.size() > 0) {
		int numObjects = hierarchy.size();
        //if number of objects greater than MAX_NUM_OBJECTS we have a noisy filter
        if(numObjects<MAX_NUM_OBJECTS){
			for (int index = 0; index >= 0; index = hierarchy[index][0]) {

				Moments moment = moments((cv::Mat)contours[index]);
				double area = moment.m00;

				//if the area is less than 20 px by 20px then it is probably just noise
				//if the area is the same as the 3/2 of the image size, probably just a bad filter
				//we only want the object with the largest area so we safe a reference area each
				//iteration and compare it to the area in the next iteration.
                if(area>MIN_OBJECT_AREA && area<MAX_OBJECT_AREA && area>refArea){
					x = moment.m10/area;
					y = moment.m01/area;
					objectFound = true;
					refArea = area;
					findDistance(area,distance);
				}else objectFound = false;

			}


			//let user know you found an object
			if(objectFound ==true){
				cout<<"Tracking Object"<<endl;
				//draw object location on screen
				}

		}else cout<<"TOO MUCH NOISE! ADJUST FILTER";
	}
}


int main(int argc, char* argv[])
{	// create a GPIO object from MRAA using pin 7
	mraa::Gpio* d_pin = new mraa::Gpio(7);
	//create motors
	upm::L298* motor1 = new upm::L298(3, 6, 8);
	upm::L298* motor2 = new upm::L298(5, 4, 9);
	//some boolean variables for different functionality within this
	//program
	bool motion = false;
	bool object=false;
	//Matrix to store each frame of the webcam feed
	Mat cameraFeed;
	//matrix storage for HSV image
	Mat HSV;
	//matrix storage for binary threshold image
	Mat threshold;
	//x and y values for the location of the object
	int x=0, y=0;
	int a=0, b=0;
	double distance=0;
	//video capture object to acquire webcam feed
	VideoCapture capture;
	//open capture object at location zero (default location for webcam)
	capture.open(0);
	//set height and width of capture frame
	capture.set(CV_CAP_PROP_FRAME_WIDTH,FRAME_WIDTH);
	capture.set(CV_CAP_PROP_FRAME_HEIGHT,FRAME_HEIGHT);
	//start an infinite loop where webcam feed is copied to cameraFeed matrix
	//all of our operations will be performed within this loop
	for(;;){
		//store image to matrix
		capture.read(cameraFeed);
		//convert frame from BGR to HSV colorspace
		cvtColor(cameraFeed,HSV,COLOR_BGR2HSV);
		//filter HSV image between values and store filtered image to
		//threshold matrix
		inRange(HSV,Scalar(H_MIN,S_MIN,V_MIN),Scalar(H_MAX,S_MAX,V_MAX),threshold);
		//perform morphological operations on thresholded image to eliminate noise
		//and emphasize the filtered object(s)
		morphOps(threshold);
		//pass in thresholded frame to our object tracking function
		//this function will return the x and y coordinates of the
		//filtered object
		trackFilteredObject(x,y,threshold,object,distance);
		cout<<x<<endl;


		if(object==true){
		if(x<a+2 && x>a-2 && y<b+2 && y>b-2){
			a=x;
			b=y;
			motion=true;}
		else{
			a=x;
			b=y;
			motion=false;}
		}else {motion=false;}

		cout<<object<<endl;
		cout<<motion<<endl;
		cout<<a<<endl<<b<<endl<<endl;

		if(motion==true && object==false){
			int pin_value = d_pin->read();
			int i=0;
			if(x<=280 && pin_value!=0){
				while(x<280 && pin_value!=0){
					//store image to matrix
					capture.read(cameraFeed);
					//convert frame from BGR to HSV colorspace
					cvtColor(cameraFeed,HSV,COLOR_BGR2HSV);
					//filter HSV image between values and store filtered image to
					//threshold matrix
					inRange(HSV,Scalar(H_MIN,S_MIN,V_MIN),Scalar(H_MAX,S_MAX,V_MAX),threshold);
					//perform morphological operations on thresholded image to eliminate noise
					//and emphasize the filtered object(s)
					morphOps(threshold);
					//pass in thresholded frame to our object tracking function
					//this function will return the x and y coordinates of the
					//filtered object
					pin_value = d_pin->read();
					motor1->setSpeed(35);
					motor2->setSpeed(35);
					motor1->setDirection(upm::L298::DIR_CW);
					motor2->setDirection(upm::L298::DIR_CCW);
					motor1->enable(true);
					motor2->enable(true);
					trackFilteredObject(x,y,threshold,object,distance);
					cout<<x<<endl;
					i++;
					cout<<"The Ball is on the left."<<endl;
					if(i>=5){
						if(object==false){
							i=0;
							break;
						}else i=0;
					}
				}
				motor1->setDirection(upm::L298::DIR_NONE); // fast stop
				motor2->setDirection(upm::L298::DIR_NONE); // fast stop
				motor1->setSpeed(0);
				motor2->setSpeed(0);
				motor1->enable(false);
				motor2->enable(false);
				sleep(3);
				}

			else if(x>=330){
				while(x>=330 && pin_value!=0){
					//store image to matrix
					capture.read(cameraFeed);
					//convert frame from BGR to HSV colorspace
					cvtColor(cameraFeed,HSV,COLOR_BGR2HSV);
					//filter HSV image between values and store filtered image to
					//threshold matrix
					inRange(HSV,Scalar(H_MIN,S_MIN,V_MIN),Scalar(H_MAX,S_MAX,V_MAX),threshold);
					//perform morphological operations on thresholded image to eliminate noise
					//and emphasize the filtered object(s)
					morphOps(threshold);
					//pass in thresholded frame to our object tracking function
					//this function will return the x and y coordinates of the
					//filtered object
					pin_value = d_pin->read();
					motor1->setSpeed(35);
					motor2->setSpeed(35);
					motor1->setDirection(upm::L298::DIR_CCW);
					motor2->setDirection(upm::L298::DIR_CW);
					motor1->enable(true);
					motor2->enable(true);
					trackFilteredObject(x,y,threshold,object,distance);
					i++;
					cout<<x<<endl;
					cout<<"The Ball is on the right."<<endl;
					if(i>=5){
						if(object==false){
							i=0;
							break;
						}
					}else i=0;
				}
				motor1->setDirection(upm::L298::DIR_NONE); // fast stop
				motor2->setDirection(upm::L298::DIR_NONE); // fast stop
				motor1->setSpeed(0);
				motor2->setSpeed(0);
				motor1->enable(false);
				motor2->enable(false);
				sleep(3);
			}
			else if(x>=280 && x<=330){
					while(x>=280 && x<=330 && pin_value!=0){
					//store image to matrix
					capture.read(cameraFeed);
					//convert frame from BGR to HSV colorspace
					cvtColor(cameraFeed,HSV,COLOR_BGR2HSV);
					//filter HSV image between values and store filtered image to
					//threshold matrix
					inRange(HSV,Scalar(H_MIN,S_MIN,V_MIN),Scalar(H_MAX,S_MAX,V_MAX),threshold);
					//perform morphological operations on thresholded image to eliminate noise
					//and emphasize the filtered object(s)
					morphOps(threshold);
					//pass in thresholded frame to our object tracking function
					//this function will return the x and y coordinates of the
					//filtered object
					pin_value = d_pin->read();
					motor1->setSpeed(40);
					motor2->setSpeed(40);
					motor1->setDirection(upm::L298::DIR_CW);
					motor2->setDirection(upm::L298::DIR_CW);
					motor1->enable(true);
					motor2->enable(true);
					trackFilteredObject(x,y,threshold,object,distance);
					cout<<x<<endl;
					i++;
					cout<<"Go forward"<<endl;
					if(i>=5){
						if(object==false){
							i=0;
							break;
						}
					}else i=0;
				}
				motor1->setDirection(upm::L298::DIR_NONE); // fast stop
				motor2->setDirection(upm::L298::DIR_NONE); // fast stop
				motor1->setSpeed(0);
				motor2->setSpeed(0);
				motor1->enable(false);
				motor2->enable(false);
				sleep(3);
			}


		}



		//delay 30ms so that screen can refresh.
		//image will not appear without this waitKey() command
		usleep(500000);
		waitKey(500);

	}

	delete motor1;
	delete motor2;


	return 0;
}
