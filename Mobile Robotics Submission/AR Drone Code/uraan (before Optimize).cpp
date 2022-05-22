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

#define SHOW_IMAGE 1
//~ #define DO_TAKE_OFF 1
//~ #define DO_MOVE 1

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
                  cout<<"Distance = "<< getDist(getCenter(squares[i]),getCenter(squares[j])) <<endl;
                  squaresFiltered.push_back(squares[i]); // Smaller Square
                  squaresFiltered.push_back(squares[j]); // Larger Square
                  return true;
               }
            }
         }
   }
   
   return false;
}

struct sockaddr_in receiver_addr;
int sock_fd;
bool openSocket(){
   //~ char line[] = "AT*LED=1,0,1073741824,4\r"; // Blinks LED
   sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
   receiver_addr.sin_family = AF_INET;
	 if( inet_aton( "192.168.1.1",  &receiver_addr.sin_addr )== 0) {
	   close(sock_fd);
		return false;
	 }
   receiver_addr.sin_port = htons( PORT );
   return true;
}

void sendAT(const char* line){
   sendto(sock_fd, line, strlen(line), 0,(struct sockaddr*)&receiver_addr,sizeof(receiver_addr));
}



void watchdogReset(){
   /*
   The drone does not receive any traffic for more that 50ms; it will then set its ARDRONE_COM_WATCHDOG_MASK
   bit in the ardrone_state field (2nd field) of the navdata packet. To exit this mode, the client
   must send the AT Command AT*COMWDG.
   • The drone does not receive any traffic for more than 2000ms; it will then stop all com-
   munication with the client, and internally set the ARDRONE_COM_LOST_MASK bit in its state
   variable. The client must then reinitialize the network communication with the drone.
   */
   boost::posix_time::milliseconds workTime(500);
   const char buffer[] = "AT*COMWDG=1\r";
   while(1){
      sendAT(buffer);
      boost::this_thread::sleep(workTime);
   }
}

struct cmd_vel_struct{
   float moveLR; // Move Left Right
   float UD; // Up Down
   float FB; // Forward Backwards
   float rotateLR; // Rotation Left Right
} cmd_vel;

struct error_struct{
   int moveLR; // Move Left Right
   int UD; // Up Down
   int FB; // Forward Backwards
   int rotateLR; // Rotation Left Right
} tError;

/* Encodes 32-bit float in 32-bin int */
uint32_t encodeFloat(float f){
   return *(uint32_t *)(&f);
}

/* Decodes 32-bit float from 32-bin int */
float decodeFloat(uint32_t i){
   return *(float *)(&i);
}

template <typename T>
void constrain(T& var, T min, T max){
   if(var>max){
      var = max;
      return;
   }
   if(var<min){
      var = min;
      return;
   }
}

void atLoop(){
   boost::posix_time::milliseconds workTime(30);
   char temp[100];
   while(1){
      constrain(cmd_vel.moveLR, -1.0f, 1.0f);
      constrain(cmd_vel.FB, -1.0f, 1.0f);
      constrain(cmd_vel.UD, -1.0f, 1.0f);
      constrain(cmd_vel.rotateLR, -1.0f, 1.0f);
      
      sprintf(temp,"AT*PCMD=1,1,%u,%u,%u,%u\r", encodeFloat(cmd_vel.moveLR), encodeFloat(cmd_vel.FB), encodeFloat(cmd_vel.UD), encodeFloat(cmd_vel.rotateLR));
      sendAT(temp);
      boost::this_thread::sleep(workTime);
   }
}


float speed = 0.01;

float kp=0.001 / 4;


int main () {
   #ifdef SHOW_IMAGE
   namedWindow( wndname, 1 );
   #endif
   
   if(openSocket()){
      printf("Connection Established Successfully\n");
   }else{
      printf("Failed establish connection\n");
      return -1;
   }
   
   //~ Initiate cmd_vel to zero
   cmd_vel.FB=cmd_vel.UD=cmd_vel.moveLR=cmd_vel.rotateLR=0;
   
   //~ Initiate error to zero;
   tError.FB=tError.UD=tError.moveLR=tError.rotateLR=0;
   
   printf("Starting WatchDog Resetting Thread\n");
   boost::thread watchDogResetThread(watchdogReset);
   
   #ifdef DO_MOVE
      printf("Starting AT Loop Thread\n");
      boost::thread atLoopThread(atLoop);
   #endif
   
    vector<vector<Point> > squares;
   
   //~ Set Trim
   printf("Setting Trim\n");
   sendAT("AT*FTRIM=1\r");
   
   #ifdef DO_TAKE_OFF
      //~ Take Off
      printf("Taking Off\n");
      for(int i=0;i<50;i++){
         sendAT("AT*REF=1,290718208\r");
         usleep(100*1000); // 100ms
      }
   #endif
    
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
        printf("camCount = %u\n", camCount);
        if(image.empty() || (camCount++)%30!=0) continue; // Capture every third frame
        
   findSquares(image, squares);
   vector<vector<Point> > gSquares;
   
   if(filterSquares(squares, gSquares)){ // Marker Found
         Point pp = getCenter(gSquares[0]); Point *p = &pp;
         float area = fabs(contourArea(Mat(gSquares[0])));
         //~ cout<<"AREA: "<<area<<endl;
         
         //~ Calculating the errors
         tError.FB = (area - 600) / 2;
         tError.rotateLR = (p->x - WIDTH/2);
         tError.UD = (HEIGHT/2 - p->y);
         
         //~ Forward-Backward
         if(abs(tError.FB) < 100){
            cmd_vel.FB = 0;
         }else{
            if(tError.FB > 0) // Back
               cmd_vel.FB = kp * tError.FB / 5;
            else // Forward
               cmd_vel.FB = kp * tError.FB * 1.75;
         }
         cout<<"FB = "<<cmd_vel.FB<<endl;
         
         //~ Left-Right
         if(abs(tError.rotateLR) < 50){
            cmd_vel.rotateLR = 0;
         }else{
               cmd_vel.rotateLR = kp * tError.rotateLR * 10;
         }  
         cout<<"LR = "<<cmd_vel.rotateLR<<endl;
         cout<<"LR Dis: "<<tError.rotateLR<<endl;
         
         //~ Up-Down
         if(abs(tError.UD) < 25){
            cmd_vel.UD = 0;
         }else{
            cmd_vel.UD = kp * tError.UD * 20;
         }

   }else{ // Marker not found
         //~ Reset velocities to zero
         cmd_vel.FB=cmd_vel.UD=cmd_vel.moveLR=cmd_vel.rotateLR=0;
         //~ sendAT("AT*LED=1,2,1073741824,0\r");
         printf("..");
   }
      #ifdef SHOW_IMAGE
         drawSquares2(image, gSquares);
         waitKey(1);
      #endif
   }// end while(1)

   printf("Closing Socket\n");
   printf("Bye Bye\n");
   close(sock_fd);
   return 0;
}
