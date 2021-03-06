/* This records a gesture and saves it in a file "recorded" by default otherwise with the argument passed*/

#include "opencv2/opencv.hpp"
#include "cv.h"
#include <opencv2/imgproc/imgproc.hpp>   //opencv
#include <iostream>
#include <stdio.h>      //std
#include <unistd.h> 
#include <time.h>       //time
#include <string.h>
#include <sstream>	//string concatenation
#include <dirent.h>    //count files

#define FILTER 46,76,88,255,114,255//60,90,88,255,114,255    //HSV filter args
#define MILS_BEFORE_GEST 100	       // Wait time before gesture starts recording
#define GEST_TIME 1 		       //Time for which a gesture is recorded

using namespace std;
using namespace cv;
  
VideoCapture video(0);
Mat orig, filt, cent,cropped,rescaled;
Point prev=Point(-1,-1);
int flag =0; //whether to start gesture recording

void clear(Mat &img)
{
  for(int row=0;row<img.rows;++row)
  {
    for(int col=0;col<img.cols;++col)
    {
      img.at<Vec3b>(row,col)=Vec3b(0,0,0);
    }
  }
}
void filter(int h1,int h2,int s1,int s2,int v1,int v2)  //filter the coloured part 
{
  cvtColor(orig,orig,CV_RGB2HSV);
  blur(orig,orig,Size(10,10),Point(-1,-1));
  for(int i=0;i<orig.rows;++i)
  {
    for(int j=0;j<orig.cols;++j)
    {
      if(!(orig.at<Vec3b>(i,j).val[0]>h1 && orig.at<Vec3b>(i,j).val[0]<h2 && orig.at<Vec3b>(i,j).val[1]>s1 && orig.at<Vec3b>(i,j).val[1]<s2 && orig.at<Vec3b>(i,j).val[2]>v1 && orig.at<Vec3b>(i,j).val[2]<v2))
      {
	filt.at<Vec3b>(i,j)=Vec3b(0,0,0);
      }
      else
      {
	filt.at<Vec3b>(i,j)=Vec3b(255,255,255);
      }
    }
  }
  blur(filt,filt,Size(10,10),Point(-1,-1));
}

void rescale()
{
  int row_max,row_min,col_max,col_min;
  col_min=cent.cols;
  col_max=row_max=0;
  row_min=cent.rows;

  for(int i=0;i<cent.rows;++i)
  {
    for(int j=0;j<cent.cols;++j)
    {
      if(cent.at<Vec3b>(i,j)==Vec3b(255,255,255))
      {
	if(i>row_max)row_max=i;
	if(i<row_min)row_min=i;
	if(j>col_max)col_max=j;
	if(j<col_min)col_min=j;
      }
    }
  }
  Rect roi=cvRect(col_min,row_min,col_max-col_min,row_max-row_min);
  cropped=cent(roi).clone();
  resize(cropped,rescaled,Size(0,0),32.0/(cropped.cols),24.0/(cropped.rows),CV_INTER_AREA);
}

void centroid()                //plot the centroid and join with prev 
{
  int row=0,col=0,count=0;
  for(int i=0;i<filt.rows;++i)
  {
    for(int j=0;j<filt.cols;++j)
    {
      if(filt.at<Vec3b>(i,j)==Vec3b(255,255,255))
      {
	count++;row+=i;col+=j;
      }
    }
  }
  if(count!=0)
  {
    row/=count;col/=count;
    if(prev!=Point(-1,-1))
    {
      line(cent,Point(col,row),prev,Scalar(255,255,255),2,4);
    }
    else
    {
      flag=1;
    }
    prev.x=col;prev.y=row;
  }
  else
  {
    prev=Point(-1,-1);
  }
}
void rec_gest_image()   	//Records the actual gesture
{
  clock_t t;
  prev=Point(-1,-1);clear(cent);
  t=clock();
  while((float)(clock()-t)/CLOCKS_PER_SEC<GEST_TIME)	//Time for which gesture is recorded
  {
    video >>orig;
    filter(FILTER);
    centroid();
  }
  rescale();
}

int rec_gest() //Record the gesture
{
  prev=Point(-1,-1);
  flag =0;
  if(!video.isOpened())
    return -1;
  video >>orig;
  filt=orig.clone();
  clear(filt);
  cent=filt.clone();
  while(1)
  {
    video >>orig;
    filter(FILTER);
    centroid();
    if(flag)
    {
      usleep(MILS_BEFORE_GEST*1000);
      rec_gest_image();
      break;
    }
  }
  return 0;
}
