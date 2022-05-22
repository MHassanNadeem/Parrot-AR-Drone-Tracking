/* Simple Code to Test OpenCV on AR Drone*/

#include <cv.h>
#include <highgui.h>

using namespace cv;

int main( int argc, char** argv ){
 char* imageName = argv[1];

 Mat image;

 if( argc != 2){
   printf( "ERROR: Please specify Image name in argument\n " );
   return -1;
 }
 
 image = imread( imageName, 1 );
 
  if(!image.data ){
   printf( "ERROR: No image\n " );
   return -1;
 }

 Mat gray_image;
 cvtColor( image, gray_image, CV_BGR2GRAY );

 imwrite( "Gray_Image.jpg", gray_image );

 return 0;
}
