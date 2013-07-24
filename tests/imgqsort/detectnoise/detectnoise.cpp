#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

using namespace cv;
using namespace std;

/// Global variable
Mat src, src_gray;
Mat dst, detected_edges;

int edgeThresh = 1;
int lowThreshold=0.035;   //given in research paper
int ratio = 3;
int kernel_size = 3;
char* window_name = "Edge Map";

void CannyThreshold(int, void*)
{
  /// Reduce noise with a kernel 3x3
  blur( src_gray, detected_edges, Size(3,3) );

  /// Canny detector
  Canny( detected_edges, detected_edges, lowThreshold, lowThreshold*ratio, kernel_size );

  /// Using Canny's output as a mask, we display our result
  dst = Scalar::all(0);

  src.copyTo( dst, detected_edges);
  imshow( window_name, dst );
 }


/** @function main */
int main( int argc, char** argv )
{
  // Load an image
  src = imread( argv[1] );

  if( !src.data )
  { return -1; }

  // Create a matrix of the same type and size as src (for dst)
  dst.create( src.size(), src.type() );

  // Convert the image to grayscale
  cvtColor( src, src_gray, CV_BGR2GRAY );

   // Apply Canny Edge Detector to get the edges
  CannyThreshold(0, 0);

    double average=mean(detected_edges)[0];
    double maxval;
    int *maxIdx=(int* )malloc(sizeof(detected_edges));
    
    // To find the maximum edge intensity value
    
    minMaxIdx(detected_edges, 0, &maxval, 0, maxIdx);

    double blurresult=average/maxval;
    cout<<"The average of the edge intensity is "<<average<<std::endl;
    cout<<"The maximum of the edge intensity is "<<maxval<<std::endl;
    cout<<"The result of the edge intensity is "<<blurresult<<std::endl;

  /// Wait until user exit program by pressing a key
  waitKey(0);

  return 0;
  }
