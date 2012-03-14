#include "stdafx.h"
#include <stdlib.h>
#include <gl/glut.h>
#include <stdio.h>
#include <XnCppWrapper.h>
#include <XnVNite.h>

#include "vertex.h"
#include "gesture.h"
#include "window.h"
#include "vmmodel.h"
#include "drawhand.h"

using namespace xn;

#define GESTURE_TO_USE "Click" 
#define HANDDEPTH 60 //in mm
#define HANDRADIUS 80
#define GRAB_THRESHOLD 0.45

static xn::DepthGenerator g_DepthGenerator;
static xn::HandsGenerator g_HandsGenerator;
static xn::GestureGenerator g_GestureGenerator;
static XnPoint3D palmPos;
static bool GRAB  = false;

int printDebug = 0; 
int printTraining = 0; //0 - don't output 1 - close hand, 2 - open hand 
FILE *pFile;
XnStatus nRetVal = XN_STATUS_OK;
XnUInt16 nXRes, nYRes;
static bool SHOWHAND;

const int nb = 4;
int svm_grid[2*HANDRADIUS][2*HANDRADIUS]; // size 2HANDRADIUS*2HANDRADIUS
int numpoint[nb][nb];

//-----------------------------------------------------
//					   LOG
//-----------------------------------------------------
void enableDebugGesture(){

	for(int i=0; i< 2*HANDRADIUS; i++){
		for(int j=0; j< 2*HANDRADIUS; j++){
			svm_grid[i][j] = 0;
		}
	}
	pFile = fopen("depthmap.txt", "w");		

}

//------------------------------------------------------
//                    HELPER
//------------------------------------------------------
void set_print_training(int i){
	printTraining = i;
}
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
	float left = getCenter().x- getDiam(); //get the leftmost coordinate
	return left + (nXRes-x)*(2*getDiam())/nXRes;	//scale to the viewport
}

float convertY(float y){
	float bottom = getCenter().y- getDiam(); //get the leftmost coordinate
	return bottom + (nYRes-y)*(2*getDiam())/nYRes;
}
//for cursor, do not reverse the side (nYRes)
float convertYcursor(float y){
	float bottom = getCenter().y- getDiam(); //get the leftmost coordinate
	return bottom + y*(2*getDiam())/nYRes;

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

	//svm
	int svm_index = 0;


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
	glDisable(GL_TEXTURE_2D);

	//draw hand from depth map
	if(palmPos.Z != 0){
	glPointSize(2);
	if(!GRAB)	glColor3f(0, 1, 0);
	else glColor3f(1, 0.0, 0.0);
	XnPoint3D *p = new XnPoint3D;
	
	//svm label 
	if(printTraining == 1){
		fprintf(pFile, "-1\t");	//close hand
	}else if(printTraining == 2){
		fprintf(pFile, "1\t");	//open hand
	}

	glBegin(GL_POINTS);
		//Iterate through the pixel from top to bottom/ right to left
		for (nY= 0; nY<nYRes; nY++)
		{
			for (nX=0; nX<nXRes; nX++)
			{
				
				nValue = *pDepth;

				//in the bounding box only
				if((nX < palmPos.X+HANDRADIUS && nX > palmPos.X-HANDRADIUS) && 
					(nY < palmPos.Y+HANDRADIUS && nY > palmPos.Y-HANDRADIUS)){
						
					svm_index++;
					
					//the point falls inside the threshold
					if(nValue > minZ && nValue <maxZ){
						
							//the depth map is mirroring. As glOrtho is scaled to 0,nXRes and 0,nYRes
							//we flip the coordinate using nXRes and nYRes
							//convert in relative to viewport 
							if(SHOWHAND)
								glVertex3f(convertX(nX), convertY(nY), 0.0f);

								(*p).X = nX;
								(*p).Y = nY;
								(*p).Z = nValue;
								//store the point that could be in hand region
								handPointList[n] = *p;
								n++;	

								if(printTraining){
									//add to svm_grid
									int px = nX - (int)palmPos.X + HANDRADIUS;
									int py = nY - (int)palmPos.Y + HANDRADIUS;

									svm_grid[px][py] = 1;
								}
							}
				}
				pDepth++;
			}
		}		
	}
	glEnd();

	if(printTraining){ 
		int one_block = HANDRADIUS/2;
		int count =0;

		
		for(int m=0; m < nb; m++){
			for(int n=0; n< nb; n++){
				for(int j = one_block*m; j< one_block*(m+1); j++){
					for(int i= one_block*n; i< one_block*(n+1) ; i++){
						if(svm_grid[i][j]) {
							count++;
							//fprintf(pFile,"*");
						}
						//fprintf(pFile, " ");
						svm_grid[i][j] = 0;
					}
					//fprintf(pFile,"\n");
				}
				numpoint[m][n] = count;
				fprintf(pFile, "%d:%d ", (m*nb)+n+1, count);
				count =0;
			}
		}
		fprintf(pFile, "\n");
		//done
		//set_print_training(0);
	}

	if(printDebug && n > 0){
			fprintf(pFile, "\nnNumberOfPoint = %u\n",  n);
		}

	if(n> 0) find_finger(handPointList, n);

	if(n > 0) getEdge(handPointList, n);
	else GRAB = false;



	glEnable(GL_LIGHTING);
}

//angle from x-axis to vector x2,y2 -> x1,y1
float findAngle(float x1, float y1, float x2, float y2, float x3, float y3){
	float PI = 3.14159265;
	float angle1= atan2((y2-y1),(x2-x1));	//vector 2->1
	float angle2= atan2((y2-y3),(x2-x3));	//vector 2->3

	float anglebtw= (angle1-angle2)*(180/PI);
	if(anglebtw < 0){
		anglebtw+=360;
	}
	return anglebtw;
}

float dis(XnPoint3D a, XnPoint3D b){
	return sqrt(pow(a.X-b.X, (float)2.0) +pow(a.Y-b.Y, (float) 2));

}

void find_finger(XnPoint3D* List, int nNumberOfPoints){
	//giftwrap technique 

	int count = 0; 
	int MAXH = 10000;

	//find lowest point
	XnPoint3D *lowest = new XnPoint3D;
	(*lowest).X = 0; (*lowest).Y = 0;
	XnPoint3D* ptr = List; 
	while(count < nNumberOfPoints){	
		unsigned int xx = ptr->X;
		unsigned int yy = ptr->Y;

		if(xx > (*lowest).X){
			lowest->X = ptr->X;
			lowest->Y = ptr->Y;
		}
		count++;
		ptr++;
	}

	
	//find the smallest angle (2D)
	XnPoint3D* base = new XnPoint3D;
	base = lowest;

	XnPoint3D* next = new XnPoint3D;
	XnPoint3D convexList[200];
	convexList[0].X = base->X;
	convexList[0].Y = base->Y;
	int last = 1; 
	int min = 360;
	
	//first one compare to y axis
	for(int i=0; i< nNumberOfPoints; i++){
		float a = findAngle(base->X, base->Y-5, base->X, base->Y, List[i].X, List[i].Y);
		if(a <= min && base->X != List[i].X && base->Y != List[i].Y){
			*next = List[i];
			min = a;
		}
	}
	convexList[last] = *next;
	last++;

	//the rest 
	int k=0;
	while(k < 30 ){
	
		min = 360;
		for(int i=0; i< nNumberOfPoints; i++){
			//angle between two line
			float a = findAngle(convexList[last-2].X, convexList[last-2].Y, 
				convexList[last-1].X, convexList[last-1].Y, List[i].X, List[i].Y);
			
			//find min angle. not itself
			if(a <= min && a != 0 && 
					convexList[last-1].X != List[i].X && convexList[last-1].Y != List[i].Y){
				*next = List[i];
				min = a;
			}
		} 
		convexList[last].X = next->X;
		convexList[last].Y = next->Y;
		last++;
		k++;
	}

	glLineWidth(3);
	glColor3f(0,0,1);
	glBegin(GL_LINE_STRIP);
	for(int n=0; n< last; n++){
		glVertex3f(convertX(convexList[n].X), convertY(convexList[n].Y), 0);
		fprintf(pFile, "%f, %f ", convexList[n].X, convexList[n].Y);
	}
	fprintf(pFile, "\n");
	glEnd();
	/*
	base = lowest;
	fprintf(pFile, "%.2f-%.2f\t", base->X, base->Y);
	convex[convexSize].X = base->X;
	convex[convexSize].Y = base->Y;
	convexSize++;

	while(!(smallestAngle->X == lowest->X && smallestAngle->Y == lowest->Y)){	
		ptr = List; 
		float angle = 360;

		for(int i = 0; i< nNumberOfPoints; i++){
			float a = findAngle(List[i].X, List[i].Y, base->X, base->Y);
			if(a < angle && a > 0){
				smallestAngle->X = List[i].X;
				smallestAngle->Y = List[i].Y;
				angle = a;
			}
		}
		convex[convexSize].X = smallestAngle->X;
		convex[convexSize].Y = smallestAngle->Y;
		fprintf(pFile, "%.2f-%.2f\t", smallestAngle->X, smallestAngle->Y);
		convexSize++;

		base->X = smallestAngle->X;
		base->Y = smallestAngle->Y;
	}
	fprintf(pFile, "\n");

	
	glLineWidth(3);
	glColor3f(0, 0, 255);
	glBegin(GL_LINE_LOOP);
	for(int i=0; i< convexSize; i++){
		glVertex3f(convertX(convex[i].X), convertY(convex[i].Y), 0);
	}
	glEnd();

	*/
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
		float x = convertX(palmPos.X);
		float y = convertY(palmPos.Y);
		float z = palmPos.Z;
		drawRHand(GRAB, x, y, z);
		
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