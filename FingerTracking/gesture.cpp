#include "stdafx.h"
#include <stdlib.h>
#include <gl/glut.h>
#include <stdio.h>
#include <XnCppWrapper.h>
#include <XnVNite.h>

#include "gesture.h"
#include "window.h"
#include "model.h"
#include "miniball.h"

using namespace xn;

#define GESTURE_TO_USE "Click" 
#define HANDDEPTH 40 //in mm
#define HANDRADIUS 100
#define GRAB_THRESHOLD 0.45

static xn::DepthGenerator g_DepthGenerator;
static xn::HandsGenerator g_HandsGenerator;
static xn::GestureGenerator g_GestureGenerator;
static XnPoint3D palmPos;
static bool GRAB  = false;


int printDebug = 0; 
FILE *pFile;
XnStatus nRetVal = XN_STATUS_OK;
XnUInt16 nXRes, nYRes;
static bool SHOWHAND;



//-----------------------------------------------------
//					   LOG
//-----------------------------------------------------
void enableDebugGesture(){
	pFile = fopen("depthmap.txt", "w");
	printDebug = 1;				
}

//------------------------------------------------------
//                    HELPER
//------------------------------------------------------
bool isGrab(){
	return GRAB;
}

XnPoint3D getPalm(){
	return palmPos;
}

void switchShowHand(){
	SHOWHAND = !SHOWHAND;
		if(SHOWHAND) printf("switch hand mode to SHOW\n");
		else 	printf("switch hand mode to NOT_SHOW\n");
}

float convertX(float x){
	float left = getCenterSphere().X- getDiamSphere(); //get the leftmost coordinate
	return left + (nXRes-x)*(2*getDiamSphere())/nXRes;	//scale to the viewport
}

float convertY(float y){
	float bottom = getCenterSphere().Y- getDiamSphere(); //get the leftmost coordinate
	return bottom + (nYRes-y)*(2*getDiamSphere())/nYRes;
}
//for cursor, do not reverse the side (nYRes)
float convertYcursor(float y){
	float bottom = getCenterSphere().Y- getDiamSphere(); //get the leftmost coordinate
	return bottom + y*(2*getDiamSphere())/nYRes;

}

//-----------------------------------------------------
//					Register generator
//-----------------------------------------------------
xn::DepthGenerator* getDepthGenerator(Context ctx){
	nRetVal = g_DepthGenerator.Create(ctx);
	CHECK_RC(nRetVal, "Create depth");
	return &g_DepthGenerator;
}
xn::HandsGenerator* getHandGenerator(Context ctx){
	nRetVal = g_HandsGenerator.Create(ctx);
	CHECK_RC(nRetVal, "Create hands");
	return &g_HandsGenerator;
}
xn::GestureGenerator* getGestureGenerator(Context ctx){
	nRetVal = g_GestureGenerator.Create(ctx);
	CHECK_RC(nRetVal, "Create gesture");
	return &g_GestureGenerator;
}


//--------------------------------------------------
//					Gesture Callback
//--------------------------------------------------

//remove gesture generator and start hand generator
void XN_CALLBACK_TYPE Gesture_Recognized(GestureGenerator& generator, const XnChar* strGesture, 
	const XnPoint3D* pIDPosition, const XnPoint3D* pEndPosition,void* pCookie){
		//printf("Gesture recognized: %s\n", strGesture);
		g_GestureGenerator.RemoveGesture(strGesture);
		g_HandsGenerator.StartTracking(*pEndPosition);
}

//Process Gesture
//
void XN_CALLBACK_TYPE Gesture_Process(GestureGenerator& generator, const XnChar* strGesture, 
	const XnPoint3D* pPosition, XnFloat fProgress,void* pCookie)
{}

//Detect hand 
//Callback to be called when a new hand is created 
void XN_CALLBACK_TYPE Hand_Create(HandsGenerator& generator,XnUserID nId,const XnPoint3D* pPosition,
		XnFloat fTime, void* pCookie){

		XnPoint3D projPosition;
		g_DepthGenerator.ConvertRealWorldToProjective(1, pPosition, &projPosition);

		palmPos.X = projPosition.X;
		palmPos.Y = projPosition.Y;
		palmPos.Z = projPosition.Z;
		printf("New Hand: %d @ (%f, %f, %f)\n", nId, palmPos.X, palmPos.Y, palmPos.Z);
}
	
//Callback to be called when an existing hand has a new position
void XN_CALLBACK_TYPE Hand_Update(HandsGenerator& generator,XnUserID nId, const XnPoint3D* pPosition, 
	XnFloat fTime, void* pCookie){

		//printf("New Position: %d @ (%f, %f, %f)\n", nId, pPosition->X, pPosition->Y, pPosition->Z);
		XnPoint3D projPosition;
		g_DepthGenerator.ConvertRealWorldToProjective(1, pPosition, &projPosition);

		palmPos.X = projPosition.X;
		palmPos.Y = projPosition.Y;
		palmPos.Z = projPosition.Z;
		//printf("New Position: %d @ (%f, %f, %f)\n", nId, palmPos.X, palmPos.Y, palmPos.Z);
}

//Lost hand
void XN_CALLBACK_TYPE Hand_Destroy(HandsGenerator& generator,XnUserID nId, XnFloat fTime,void* pCookie){

		printf("Lost Hand: %d\n", nId);
		g_GestureGenerator.AddGesture(GESTURE_TO_USE, NULL);
}


//hand histogram
void drawHand(XnPoint3D* handPointList){

	int nX, nY;
	int minZ = palmPos.Z- HANDDEPTH;
	int maxZ = palmPos.Z+ HANDDEPTH;
	int n = 0; //number of points

	xn::DepthMetaData depthMD;
	g_DepthGenerator.GetMetaData(depthMD);
	
	nXRes = depthMD.XRes();
	nYRes = depthMD.YRes();

	//Pointer to depthmap
	const XnUInt16* pDepth = depthMD.Data();
	XnUInt16 nValue;

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glDisable(GL_LIGHTING);

	//draw hand from depth map
	if(palmPos.Z != 0){
	glPointSize(2);
	if(!GRAB)	glColor3f(0, 1, 0);
	else glColor3f(1, 0.0, 0.0);
	

	glBegin(GL_POINTS);
		//Iterate through the pixel from top to bottom/ right to left
		for (nY= 0; nY<nYRes; nY++)
		{
			for (nX=0; nX<nXRes; nX++)
			{
				
				nValue = *pDepth;
				
				//the point falls inside the threshold
				if(nValue > minZ && nValue <maxZ){

					//in the bounding box only
					if((nX < palmPos.X+HANDRADIUS && nX > palmPos.X-HANDRADIUS) && 
						(nY < palmPos.Y+HANDRADIUS && nY > palmPos.Y-HANDRADIUS)){
							//the depth map is mirroring. As glOrtho is scaled to 0,nXRes and 0,nYRes
							//we flip the coordinate using nXRes and nYRes

							//convert in relative to viewport 
							if(SHOWHAND)
								glVertex3f(convertX(nX), convertY(nY), 0.0f);
							
							XnPoint3D *p = new XnPoint3D;
							(*p).X = nX;
							(*p).Y = nY;
							(*p).Z = nValue;
							//store the point that could be in hand region
							handPointList[n] = *p;
							n++;
					}
				}
				pDepth++;
			}
		}		
	}
	glEnd();

	if(printDebug){
			fprintf(pFile, "\nnNumberOfPoint = %u\n",  n);
		}
	
	if(n > 0) getEdge(handPointList, n);
	else GRAB = false;

	glEnable(GL_LIGHTING);
}

//estimate grab gesture
void getEdge(XnPoint3D* List, int nNumberOfPoints){

	int nX, nY;
	int count = 0; 
	int MAXH = 10000;

	//find highest, lowest, lefmost and rightmost
	XnPoint3D *highest = new XnPoint3D; 
	(*highest).X = MAXH; (*highest).Y = MAXH;
	XnPoint3D *lowest = new XnPoint3D;
	(*lowest).X = 0; (*lowest).Y = 0;
	XnPoint3D *leftmost = new XnPoint3D; 
	(*leftmost).X = MAXH; (*leftmost).Y = MAXH;
	XnPoint3D *rightmost = new XnPoint3D;
	(*rightmost).X = 0; (*rightmost).Y = 0;

	XnPoint3D* ptr = List; 


	//Find top and bottom of the hand area. 
	while(count < nNumberOfPoints){	
		unsigned int xx = ptr->X;
		unsigned int yy = ptr->Y;

		if(yy < (*highest).Y)
			highest = ptr;

		else if(yy > (*lowest).Y)
			lowest = ptr;

		if(xx < (*leftmost).X)
			leftmost = ptr;

		else if(xx > (*rightmost).X)
			rightmost = ptr;

		count++;
		ptr++;
	}

	//hand length
	estimateGrab(List, nNumberOfPoints, highest, lowest, leftmost, rightmost);

	//old approach
	float lenght = lowest->Y - highest->Y;
	//if (lenght > GRAB_THRESHOLD) GRAB  = false; 
	//else GRAB = true;

	if(printDebug){
		fprintf(pFile, "H:%.1f,%.1f \tL:%.1f,%.1f >> %f\n", (*highest).X, (*highest).Y, (*lowest).X, (*lowest).Y, lenght );
		fprintf(pFile, "H:%.1f,%.1f \tL:%.1f,%.1f >> %f\n", (*leftmost).X, (*leftmost).Y, (*rightmost).X, (*rightmost).Y);
	}


	//------draw-------

	
	if(SHOWHAND){
		//draw top and bottom
		glPointSize(8);
		glColor3f(0, 1.0, 0);
		glBegin(GL_POINTS);
		glVertex2f((GLfloat) convertX((*highest).X), (GLfloat) convertY((*highest).Y));
		glVertex2f((GLfloat) convertX((*lowest).X), (GLfloat) convertY((*lowest).Y));
		glVertex2f((GLfloat) convertX((*rightmost).X), (GLfloat) convertY((*rightmost).Y));
		glVertex2f((GLfloat) convertX((*leftmost).X), (GLfloat) convertY((*leftmost).Y));
		glEnd();
	}
	else{
		glPointSize(12);
		if(GRAB) glColor3f(1.0, 0, 0);
		else glColor3f(0, 1.0, 0);
				
		glBegin(GL_POINTS);
		glVertex2f((GLfloat) convertX(palmPos.X), (GLfloat) convertY(palmPos.Y));
		glEnd();

		//cross hair 
		glBegin(GL_LINES);
		glVertex2f((GLfloat) convertX(palmPos.X), (GLfloat) convertY(palmPos.Y)+30.0);
		glVertex2f((GLfloat) convertX(palmPos.X), (GLfloat) convertY(palmPos.Y)-30.0);
		glVertex2f((GLfloat) convertX(palmPos.X)+30.0, (GLfloat) convertY(palmPos.Y));
		glVertex2f((GLfloat) convertX(palmPos.X)-30.0, (GLfloat) convertY(palmPos.Y));
		glEnd();
	}
}

//calculate the area half top of the hand area 
//greater than 50% => grab 
void estimateGrab(XnPoint3D* list, int n, XnPoint3D* highest, XnPoint3D* lowest, XnPoint3D* leftmost, XnPoint3D* rightmost){
	


	int nX, nY, nom=0, denom=0; 
	int y = (int)palmPos.Y;
	int xr = (int)(*rightmost).X; 
	int xl = (int)(*leftmost).X; 
	int yh = (int)(*highest).Y;

	if(printDebug){
		fprintf(pFile, "top = %d,%d ---> %d,%d\n", xr,yh,xl,yh );
		fprintf(pFile, "bottom = %d,%d ---> %d,%d\n", xr,y,xl,y);
	}

	XnPoint3D* ptr = list;
	int count = 0;
	//draw bonding box 
	// (max) xr,yh----------------xl, yh 
	//		 |						|
	//		 |						|
	//		 xr,y-------x,y-------xl, y
	
	
	while (count < n){
		int xx = ptr->X;
		int yy = ptr->Y;
		//fprintf(pFile,"%d,%d ", xx, yy);
		
		if(xx < xr && xx> xl && yy < y && yy > yh) nom++;
		count++;
		ptr++;
	}
	denom = (xr-xl)*(y-yh);
	float percent =  ((float)nom)/denom;
	if(denom != 0 && printDebug) {
		fprintf(pFile, "estimate hand point = %d, %d, %f\n", nom, denom, ((float)nom)/denom);
	}

	//estimate hand area > 0.5 => grab
	if (percent > GRAB_THRESHOLD) GRAB  = true; 
	else GRAB = false;

}