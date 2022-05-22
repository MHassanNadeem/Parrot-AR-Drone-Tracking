#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <string>
#include <stdio.h>
#include <iostream>

using namespace cv;

cv::Mat input;

// /dev/video0
int main(int argc, char** argv){
   int no = 0;
   char fileName[20];
   char streamName[20];
   
   if(argc>1){
      no = atoi(argv[1]);
   }
   
   sprintf(fileName,"camCapture%u.jpg",no);
   sprintf(streamName,"video%u",no);
   
   std::cout<<"Opening Cam #"<<no<<" -> "<<streamName<<std::endl;
   
   VideoCapture cap(no);
   for(int i=0;i<100;i++){
      bool bSuccess = cap.read(input);
      if(bSuccess) break;
   }

   cv::imwrite(fileName, input);
   return 0;
}
