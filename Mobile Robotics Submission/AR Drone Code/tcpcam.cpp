/*

*/

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <string>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <ctime>

using namespace cv;
using namespace std;

cv::Mat input;

int main(int argc, char** argv){
   time_t tstart, tend; // for keeping time
   int no = 0;
   char fileName[30];
   char url[]="tcp://192.168.1.1:5555";
   
   if(argc>1){
      no = atoi(argv[1]);
   }
   
   sprintf(fileName,"camCapture%u.jpg",no);
   
   std::cout<<"Opening Cam, Please Wait ...\n";
   VideoCapture cap(url);
   if(!cap.isOpened()){
      cout<<"Error Opening Camera!!!\n";
      return -1;
   }
   std::cout<<"Cam Opened\n";

   
   bool bSuccess = true;
   
   tstart = time(0);
   
   for(int i=0;i<100;i++) cap.read(input);
   
   //~ for(int i=0;i<100;i++){
   unsigned int i=0;
   while(1){i++;
      bSuccess = cap.read(input);
      if(bSuccess){
            std::cout<<"Pass "<<i<<std::endl;
      }else{
            std::cout<<"Fail"<<std::endl;
      }
      sprintf(fileName,"camCapture%u.jpg",i);
      cv::imwrite(fileName, input);
      
     int c = waitKey(1);
     if( (char)c == 27 )
         break;
   }
   tend = time(0);
   double framePerSec = (double) i/difftime(tend, tstart);
   cout << "It took "<< difftime(tend, tstart) <<" second(s)."<< endl;
   cout << "Frames per sec "<< framePerSec<< endl;
   return 0;
}
