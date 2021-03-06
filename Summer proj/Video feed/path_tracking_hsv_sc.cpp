#include "opencv2/opencv.hpp"
#include "cv.h"
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include "opencv2/imgproc/imgproc.hpp"
#define MILS_BEFORE_GEST 400
#define GEST_TIME 1
#define SUDDEN_JUMP 100000

using namespace std;
using namespace cv;

Mat frame,img,img_scl,img_crop,img_flip;
int switch_value=1,clear_value;
int flag=0,gest_flag=0;
time_t t1,t2;

Point prev=Point(-1,-1);

void filter(int h1,int h2,int s1,int s2,int v1,int v2)
{
  
  for(int i=0;i<frame.rows;++i)
  {
    for(int j=0;j<frame.cols;++j)
    {
      if(!(frame.at<Vec3b>(i,j).val[0]>h1 && frame.at<Vec3b>(i,j).val[0]<h2 && frame.at<Vec3b>(i,j).val[1]>s1 && frame.at<Vec3b>(i,j).val[1]<s2 && frame.at<Vec3b>(i,j).val[2]>v1 && frame.at<Vec3b>(i,j).val[2]<v2))
      {
	frame.at<Vec3b>(i,j)=Vec3b(0,0,0);
      }
      else
      {
	frame.at<Vec3b>(i,j)=Vec3b(255,255,255);
      }
    }
  }
  blur(frame,frame,Size(10,10),Point(-1,-1));
  //Mat element=getStructuringElement(MORPH_RECT,Size(3,3),Point(-1,-1));
  //dilate(frame,frame,element);
}
void scale()
{
  int x_max,x_min,y_max,y_min,to_add;
  x_min=img_flip.cols;
  x_max=y_max=0;
  y_min=img_flip.rows;

  for(int i=0;i<img_flip.rows;++i)
  {
    for(int j=0;j<img_flip.cols;++j)
    {
      if(img_flip.at<Vec3b>(i,j)==Vec3b(255,255,255))
      {
	if(i>x_max)x_max=i;
	if(i<x_min)x_min=i;
	if(j>y_max)y_max=j;
	if(j<y_min)y_min=j;
      }
    }
  }
  if((x_max-x_min)/3.0>(y_max-y_min)/4.0)
  {
    to_add=((x_max-x_min)/3.0-(y_max-y_min)/4.0)*4;
    if(y_min-to_add/2<0)
    {
      y_max=y_max+(to_add-y_min);
      y_min=0;
      
    }
    else if(y_max+to_add/2>img_flip.cols)
    {
      y_min=y_min-(to_add-(img_flip.cols-y_max));
      y_max=img_flip.cols;      
    }
    else
    {
      y_min-=to_add/2;
      y_max+=to_add/2;
    }
  }
  else
  {
    to_add=((y_max-y_min)/4.0-(x_max-x_min)/3.0)*3;
    if(x_min-to_add/2<0)
    {
      x_max=x_max+(to_add-x_min);
      x_min=0;      
    }
    else if(x_max+to_add/2>img_flip.rows)
    {
      x_min=x_min-(to_add-(img_flip.rows-x_max));
      x_max=img_flip.rows;     
    }
    else
    {
      x_min-=to_add/2;
      x_max+=to_add/2;
    }
  }
  Rect rect=cvRect(y_min,x_min,y_max-y_min,x_max-x_min);
  img_crop=img_flip(rect).clone();
  resize(img_crop,img_scl,Size(0,0),64.0/(img_crop.cols),48.0/(img_crop.rows),CV_INTER_AREA);
  imshow("img",img_scl);
  imwrite("lol.png",img_scl);
}

void switch_callback( int position ) {
  if(position ==0)
  {
    prev=Point(-1,-1);
  }
  for(int i=0;i<frame.rows;++i)
  {
    for(int j=0;j<frame.cols;++j)
    {
      if(img.at<Vec3b>(i,j)==Vec3b(255,0,0))
      {
	img.at<Vec3b>(i,j)=Vec3b(0,0,0);
      }
    }
  }
}

void clear()
{
  for(int i=0;i<frame.rows;++i)
  {
    for(int j=0;j<frame.cols;++j)
    {
      if(img.at<Vec3b>(i,j)!=Vec3b(255,255,255))
      {
	img.at<Vec3b>(i,j)=Vec3b(0,0,0);
      }
    }
  }
}

void clear_callback(int position)
{
   for(int i=0;i<frame.rows;++i)
  {
    for(int j=0;j<frame.cols;++j)
    {
	img.at<Vec3b>(i,j)=Vec3b(0,0,0); 
    }
  }
  prev=Point(-1,-1);
}

void out_edge()
{
 int x=0, y=0,count=0;
 for(int i=0;i<frame.rows;++i)
  {
    for(int j=0;j<frame.cols;++j)
    {
      if(frame.at<Vec3b>(i,j).val[0]==255)
      {
	x+=j;y+=i;++count;
      }
    }
  }
 if(count!=0 && switch_value)
 {
   x/=count;y/=count;
   if(prev.x !=-1 && prev.y!=-1 /*&& (((prev.y-x)*(prev.y-x))+((prev.x-y)*(prev.x-y)))<SUDDEN_JUMP*/)
   {
     line(img,Point(x,y),prev,Scalar(255,255,255),2,4);
   }
   //img.at<Vec3b>(x,y)=Vec3b(255,255,255);
   prev.y=y;
   prev.x=x;
   if(gest_flag==0)
   {
     flag=1;
   }
   //blur(img, img, Size(10,10), Point(-1,-1));
 }
 else if(switch_value==0 && count!=0)
 {
   x/=count;y/=count;
   clear();
   if(img.at<Vec3b>(y,x)!=Vec3b(255,255,255))
   {
     img.at<Vec3b>(y,x)=Vec3b(255,0,0);
   }
 }
 flip(img,img_flip,1);
 imshow("path",img_flip);
}

int main()
{
  clock_t t;
  VideoCapture vid(0);
  if(!vid.isOpened())
    return -1;
  cvNamedWindow("path",0);
  cvSetWindowProperty("path",CV_WND_PROP_FULLSCREEN,CV_WINDOW_FULLSCREEN);
  cvCreateTrackbar("Switch","path",&switch_value,1,switch_callback);
  cvCreateTrackbar("clear","path",&clear_value,1,clear_callback);
  vid>> frame;
  img=frame.clone();
  for(int i=0;i<img.rows;++i)
  {
    for(int j=0;j<img.cols;++j)
    {
      img.at<Vec3b>(i,j)=Vec3b(0,0,0);
    }
  }
  while(1)
  {
    vid>> frame;
    cvtColor(frame,frame,CV_RGB2HSV);
    blur(frame,frame,Size(10,10),Point(-1,-1));
    filter(69,138,69,108,183,241);
    out_edge();
    if(flag)
    {
      flag=0;
      gest_flag=1;
      usleep(MILS_BEFORE_GEST*1000);
      t=clock();
      while((float)(clock()-t)/CLOCKS_PER_SEC<GEST_TIME)
      {
	vid>> frame;
	cvtColor(frame,frame,CV_RGB2HSV);
	blur(frame,frame,Size(10,10),Point(-1,-1));
	filter(60,100,88,255,114,213);
	//flip(frame,frame,1);
	out_edge();
      }
      scale();
    }
    if(waitKey(10)>=0)
     break;
  }
}
