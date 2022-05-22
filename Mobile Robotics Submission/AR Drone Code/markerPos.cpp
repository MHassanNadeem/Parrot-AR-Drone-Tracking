#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <boost/thread.hpp>
#include <boost/date_time.hpp>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <iostream>
#include <string>
#include <math.h>

#include "tictoc.h"

//~ #define SHOW_IMAGE 1

using namespace cv;
using namespace std;

#define PORT 5556

int thresh = 50, N = 11;
#ifdef SHOW_IMAGE
const char* wndname = "Square Detection Demo";
#endif

// helper function:
// finds a cosine of angle between vectors
// from pt0->pt1 and from pt0->pt2
double angle( Point pt1, Point pt2, Point pt0 ){
    double dx1 = pt1.x - pt0.x;
    double dy1 = pt1.y - pt0.y;
    double dx2 = pt2.x - pt0.x;
    double dy2 = pt2.y - pt0.y;
    return (dx1*dx2 + dy1*dy2)/sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
}


// returns sequence of squares detected on the image.
// the sequence is stored in the specified memory storage
void findSquares( const Mat& image, vector<vector<Point> >& squares ){
    squares.clear();
    
    Mat pyr, timg, gray0(image.size(), CV_8U), gray;
    
    // down-scale and upscale the image to filter out the noise
    //~ pyrDown(image, pyr, Size(image.cols/2, image.rows/2));
    //~ pyrUp(pyr, timg, image.size());
    timg = image;
    vector<vector<Point> > contours;
    
        cv::cvtColor(timg, gray0, CV_BGR2GRAY);
        //~ imshow("gray", gray0);
        
        // try several threshold levels
        for( int l = 0; l < N; l++ )
        {
            // hack: use Canny instead of zero threshold level.
            // Canny helps to catch squares with gradient shading
            if( l == 0 )
            {
                // apply Canny. Take the upper threshold from slider
                // and set the lower to 0 (which forces edges merging)
                Canny(gray0, gray, 0, thresh, 5);
                //~ imshow("canny", gray);
                // dilate canny output to remove potential
                // holes between edge segments
                dilate(gray, gray, Mat(), Point(-1,-1));
            }
            else
            {
                // apply threshold if l!=0:
                //     tgray(x,y) = gray(x,y) < (l+1)*255/N ? 255 : 0
                gray = gray0 >= (l+1)*255/N;
            }

            // find contours and store them all as a list
            findContours(gray, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);

            vector<Point> approx;
            
            // test each contour
            for( size_t i = 0; i < contours.size(); i++ )
            {
                // approximate contour with accuracy proportional
                // to the contour perimeter
                approxPolyDP(Mat(contours[i]), approx, arcLength(Mat(contours[i]), true)*0.02, true);
                
                // square contours should have 4 vertices after approximation
                // relatively large area (to filter out noisy contours)
                // and be convex.
                // Note: absolute value of an area is used because
                // area may be positive or negative - in accordance with the
                // contour orientation
                //~ cout<<"AREA = "<<fabs(contourArea(Mat(approx)))<<endl;
                if( approx.size() == 4 &&
                    fabs(contourArea(Mat(approx))) > 100 && fabs(contourArea(Mat(approx))) < 100000 &&
                    isContourConvex(Mat(approx)) )
                {
                    double maxCosine = 0;

                    for( int j = 2; j < 5; j++ )
                    {
                        // find the maximum cosine of the angle between joint edges
                        double cosine = fabs(angle(approx[j%4], approx[j-2], approx[j-1]));
                        maxCosine = MAX(maxCosine, cosine);
                    }

                    // if cosines of all angles are small
                    // (all angles are ~90 degree) then write quandrange
                    // vertices to resultant sequence
                    if( maxCosine < 0.3 )
                        squares.push_back(approx);
                }
            }
        }
}

double getDist(const Point& p1, const Point& p2){
	double dist = sqrt(pow(p1.x - p2.x,2) + pow(p1.y -p2.y,2));
	
	if(dist < 0) dist = -dist;
	return dist;
}

Point getCenter(const vector<Point> &square){
   Moments mu;
   mu = moments( square, false );
     
   Point2f mc(mu.m10/mu.m00 , mu.m01/mu.m00);
   
   //~ Change point2f to point and return
   return Point(mc.x,mc.y);
}

// the function draws all the squares in the image
void drawSquares( Mat& image, const vector<vector<Point> >& squares ){
    for( size_t i = 0; i < squares.size(); i++ )
    {
        const Point* p = &squares[i][0];
        int n = (int)squares[i].size();
        polylines(image, &p, &n, 1, true, Scalar(0,255,0), 3, CV_AA);
    }

   #ifdef SHOW_IMAGE
       imshow(wndname, image);
   #endif
}

void drawSquares2( Mat& image, const vector<vector<Point> >& squares ){
    for( size_t i = 0; i < squares.size(); i++ )
    {
        const Point* p = &squares[i][0];
        int n = (int)squares[i].size();
        polylines(image, &p, &n, 1, true, Scalar(0,0,255), 3, CV_AA);
        circle( image, getCenter(squares[i]), 4, Scalar(0,0,255), -1, 8, 0 );
    }

   #ifdef SHOW_IMAGE
    imshow(wndname, image);
    #endif
}

bool filterSquares(const vector<vector<Point> >& squares, vector<vector<Point> >& squaresFiltered){
   // Calculate Areas of the Squares
   vector<float> areas;
   for(int i=0; i<squares.size(); i++){
      areas.push_back(fabs(contourArea(Mat(squares[i]))));
   }
   
   // Iterate and find good squares
   for(int i=0; i<areas.size(); i++){
         for(int j=i+1; j<areas.size(); j++){
            float ratio = areas[j]/areas[i];
            if(ratio>5 && ratio<7){
               if(getDist(getCenter(squares[i]),getCenter(squares[j])) < 25){
                  //~ cout<<"Distance = "<< getDist(getCenter(squares[i]),getCenter(squares[j])) <<endl;
                  squaresFiltered.push_back(squares[i]); // Smaller Square
                  squaresFiltered.push_back(squares[j]); // Larger Square
                  return true;
               }
            }
         }
   }
   
   return false;
}

int main () {
   #ifdef SHOW_IMAGE
   namedWindow( wndname, 1 );
   #endif
   
    vector<vector<Point> > squares;
    
   printf("Opening Cam, Please Wait ...\n");
   VideoCapture cap("tcp://192.168.1.1:5555");
   //~ VideoCapture cap(0);
   if(!cap.isOpened()){
      cout<<"Error Opening Camera!!!\n";
      return -1;
   }
   cout<<"Cam Opened\n";

   Mat image;
   
   const int WIDTH  = cap.get(CV_CAP_PROP_FRAME_WIDTH);
   const int HEIGHT = cap.get(CV_CAP_PROP_FRAME_HEIGHT);
   
   cout<<"Video is "<<WIDTH<<" * "<<HEIGHT<<endl;
   
   for(int i=0;i<100;i++) cap.read(image);
   
   cout<<"Entering While Loop"<<endl;
   unsigned int camCount = 0;
   while(1){
        cap.read(image);
        
        if(image.empty() || (camCount++)%50!=0) continue; // Skip frames
        //~ printf("camCount = %u\n", camCount);
        
   tic();
   findSquares(image, squares);
   cout<<"findSquare "<<endl;toc();
   vector<vector<Point> > gSquares;
   
   if(filterSquares(squares, gSquares)){ // Marker Found
         Point p = getCenter(gSquares[0]);
         cout<<"Marker At: "<<p.x<<", "<<p.y<<endl;
         

   }else{ // Marker not found
         cout<<".."<<endl;
   }
      #ifdef SHOW_IMAGE
         drawSquares2(image, gSquares);
         waitKey(1);
      #endif
   }// end while(1)

   return 0;
}
