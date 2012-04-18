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
#include "Pair.h"
#include "svmpredict.h"
#include "smoothing.h"
#include <map>
#include <list>

using namespace xn;

#define GESTURE_TO_USE "Click" 
#define HANDDEPTH 60 //in mm
#define HANDRADIUS 80
#define GRAB_THRESHOLD 0.45
#define m_nHistorySize 25


static xn::DepthGenerator g_DepthGenerator;
static xn::HandsGenerator g_HandsGenerator;
static xn::GestureGenerator g_GestureGenerator;

static XnPoint3D RpalmPos;
static XnPoint3D LpalmPos;
static bool RGRAB  = false;
static bool LGRAB  = false;

int printDebug = 0; 
int printTraining = 0; //0 - don't output 1 - close hand, 2 - open hand 
FILE *pFile;
XnStatus nRetVal = XN_STATUS_OK;
XnUInt16 nXRes, nYRes;
static bool SHOWHAND;

const int nb = 4;
int svm_grid[2*HANDRADIUS][2*HANDRADIUS]; // size 2HANDRADIUS*2HANDRADIUS
int numpoint[nb][nb];
int len[100];	//length

static std::map<XnUInt32, std::list<XnPoint3D> > m_History;	//point history
static std::list<int> handId;
int prime_id = 0;
float aspect = 1; 

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
void setAspect(float asp){
	aspect = asp; 
}
float getAspect(){
	return aspect;
}

void set_print_training(int i){
	printTraining = i;
}

bool hasTwoHands(){
	return handId.size() > 1 ? true: false;
}

//RIGHT HAND
bool isGrab(){ return RGRAB; }
XnPoint3D getPalm(){ return RpalmPos; }

//LEFT HAND
bool isLGrab(){ return LGRAB; }
XnPoint3D getLPalm(){ return LpalmPos; }

void switchShowHand(){
	SHOWHAND = !SHOWHAND;
		if(SHOWHAND) printf("switch hand mode to SHOW\n");
		else 	printf("switch hand mode to NOT_SHOW\n");
}

float convertX(float x){
	float left = (getCenter().x- getDiam()); //get the leftmost coordinate
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

void set_primary(){
	if(handId.size() ==0) 
		prime_id = 0;
	if(handId.size() ==1) 
		prime_id = handId.front();
	else{
	
		int rh = 0;
		float xcoor = 100000;

		//find id of a right hand 
		std::map<XnUInt32, std::list<XnPoint3D> >::const_iterator Iter;
		for(Iter = m_History.begin(); Iter != m_History.end(); ++Iter){
			XnUInt32 id = Iter->first;
			XnPoint3D point = Iter->second.front();

			if(point.X < xcoor){
				rh = id;
				xcoor = point.X;
			}
		}
		prime_id = rh;
	}
}

// Colors for the points
XnFloat Colors[][3] =
{
	{0.5,0.5,0.5},	// Grey
	{0,1,0},	// Green
	{0,0.5,1},	// Light blue
	{1,1,0},	// Yellow
	{1,0.5,0},	// Orange
	{1,0,1},	// Purple
	{1,1,1}		// White. reserved for the primary point
};
XnUInt32 nColors = 6;
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
		

		printf("New Hand: %d @ (%f, %f, %f)\n", nId, projPosition.X, projPosition.Y, projPosition.Z);

		//reset map
		m_History[nId].clear();
		handId.push_front(nId);
	
		//reset primary hand
		set_primary();

		if(nId == prime_id)
			RpalmPos = projPosition;
		else 
			LpalmPos = projPosition;

		
}
	
//Callback to be called when an existing hand has a new position
void XN_CALLBACK_TYPE Hand_Update(HandsGenerator& generator,XnUserID nId, const XnPoint3D* pPosition, 
	XnFloat fTime, void* pCookie){

		//printf("New Position: %d @ (%f, %f, %f)\n", nId, pPosition->X, pPosition->Y, pPosition->Z);
		XnPoint3D projPosition;
		g_DepthGenerator.ConvertRealWorldToProjective(1, pPosition, &projPosition);

		//store in history
		XnPoint3D* ptProjective = new XnPoint3D();
		ptProjective->X = projPosition.X;
		ptProjective->Y = projPosition.Y;
		ptProjective->Z = projPosition.Z;

		m_History[nId].push_front(*ptProjective);
		// Keep size of history buffer
		if (m_History[nId].size() > m_nHistorySize)
			m_History[nId].pop_back();

		//printf("id: %d: %f %f %f \n", (int)nId, m_History[nId].front().X, m_History[nId].front().Y, m_History[nId].front().Z  );
		//reset primary hand
		set_primary();	

		if(nId == prime_id)
			RpalmPos = projPosition;
		else 
			LpalmPos = projPosition;

}

//Lost hand
void XN_CALLBACK_TYPE Hand_Destroy(HandsGenerator& generator,XnUserID nId, XnFloat fTime,void* pCookie){

		printf("Lost Hand: %d\n", nId);
		g_GestureGenerator.AddGesture(GESTURE_TO_USE, NULL);

		m_History.erase(nId);
		handId.remove(nId);
		deleteSmHand(nId);

		//reset primary hand
		set_primary();
}

/**********************************************************
						Draw hands 
***********************************************************/

void draw_hand(XnPoint3D* handPointList)
{
	//disable all lighting effect
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);	
		
	std::map<XnUInt32, std::list<XnPoint3D> >::const_iterator PointIterator;
	XnFloat* m_pfPositionBuffer;
	
	// Go over each existing hand
	for (PointIterator = m_History.begin();PointIterator != m_History.end();++PointIterator) {
		// Clear buffer
		XnUInt32 nPoints = 0;
		XnUInt32 i = 0;
		XnUInt32 Id = PointIterator->first;
		XnPoint3D pt(*PointIterator->second.begin());	

		//draw histogram		
		int hpoint = draw_map(handPointList, pt, Id);

		//predict
		if(hpoint > 0){
			predict_gesture(handPointList, pt, hpoint, Id);

			//set color for each person
			XnUInt32 nColor = Id % nColors;
			glPointSize(8);
			glColor4f(Colors[nColor][0],
				Colors[nColor][1],
				Colors[nColor][2],
				1.0f);
			if(prime_id == Id) {
				glColor4f(Colors[nColors][0],
				Colors[nColors][1],
				Colors[nColors][2],
				1.0f);
			}

			//draw palm 
			if(Id == prime_id) 
				drawRHand(RGRAB, convertX(pt.X),convertY(pt.Y), pt.Z);
			else {
				//drawLHand(LGRAB, convertX(pt.X),convertY(pt.Y), pt.Z);
			}
			
		}
	}

	glEnable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
}

//hand histogram
int draw_map(XnPoint3D* handPointList, XnPoint3D palm, int id){
	
	if(palm.Z == 0) return 0;

	int nX, nY;
	int n = 0; //number of points

	//depth threshold
	int minZ = palm.Z- HANDDEPTH;
	int maxZ = palm.Z+ HANDDEPTH;

	//svm
	int svm_index = 0;

	//depth data from kinect
	xn::DepthMetaData depthMD;
	g_DepthGenerator.GetMetaData(depthMD);
	nXRes = depthMD.XRes();
	nYRes = depthMD.YRes();
	//Pointer to depthmap
	const XnUInt16* pDepth = depthMD.Data();
	XnUInt16 nValue;

	//set color
	glPointSize(2);
	if(id == prime_id)	{
		if(!RGRAB) glColor3f(0, 1, 0);
		else glColor3f(1, 0.0, 0.0);
	}
	else {
		if(!LGRAB) glColor3f(0, 1, 0);
		else glColor3f(1, 0.0, 0.0);
	}
	XnPoint3D *p = new XnPoint3D;


	glBegin(GL_POINTS);
	//draw histogram from depthmap
	//Iterate through the pixel from top to bottom/ right to left
	for (nY= 0; nY<nYRes; nY++)
	{
		for (nX=0; nX<nXRes; nX++)
		{
			nValue = *pDepth;

			//in the bounding box only
			if((nX < palm.X+HANDRADIUS && nX > palm.X-HANDRADIUS) && 
				(nY < palm.Y+HANDRADIUS && nY > palm.Y-HANDRADIUS)){

					svm_index++;

					//the point falls inside the threshold
					if(nValue > minZ && nValue <maxZ){

						//the depth map is mirroring. As glOrtho is scaled to 0,nXRes and 0,nYRes
						//flip the coordinate using nXRes and nYRes
						//convert in relative to viewport 
						if(SHOWHAND)
							glVertex3f(convertX(nX), convertY(nY), 3.0f);

						(*p).X = nX;
						(*p).Y = nY;
						(*p).Z = nValue;
						//store the point that could be in hand region
						handPointList[n] = *p;
						n++;	

						//training data for svm
						if(printTraining){
							//add to svm_grid
							int px = nX - (int)palm.X + HANDRADIUS;
							int py = nY - (int)palm.Y + HANDRADIUS;

							svm_grid[px][py] = 1;
						}
					}
			}
			pDepth++;
		}
	}		
	delete(p);
	glEnd();

	return n;
}

//use svm prediction for gesture prediction
void predict_gesture(XnPoint3D* handPointList, XnPoint3D palm, int n, int id){

	bool result1, result2;

	//SVM predict
	result1 = find_finger(handPointList, n, palm);

	//Edge predict
	result2 = getEdge(handPointList, n, palm);


	if(result1)fprintf(pFile, "-1\t");
	else fprintf(pFile, "1\t");

	if(result2)fprintf(pFile, "-1\t");
	else fprintf(pFile, "1\t");

	if(result1 && result2) fprintf(pFile, "-1\t");
	else fprintf(pFile, "1\t");

	
	//smoothing algorithm
	smoothHand(result2, id);
	bool smooth_result = isGrab(id);

	//print
	if(smooth_result) {
		fprintf(pFile, "-1");
		if(id == prime_id) RGRAB = true;
		else LGRAB = true;
		//printf("grab\n");
	}
	else {
		fprintf(pFile, "1");
		if(id == prime_id) RGRAB = false;
		else LGRAB = false;
		//printf("-\n");
	}
	fprintf(pFile,"\n");

}

/**********************************************************
					Machine Learning
**********************************************************/
bool find_finger(XnPoint3D* List, int nNumberOfPoints, XnPoint3D palm){
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

	
	//find convex hull
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
	
	for(int i=0; i< 100; i++){
		len[i] = 0;
	}

	//distance from the center hand
	for(int n=0; n< last; n++){
		//only consider point above the central hand
			int d = (int)dis(convexList[n].X, convexList[n].Y, palm.X, palm.Y);
			len[d] +=1;
	}

	//for training
	//svm label 
	if(printTraining == 1){
		fprintf(pFile, "-1\t");	//close hand
	}else if(printTraining == 2){
		fprintf(pFile, "1\t");	//open hand
	}
	if(printTraining){
		for(int i=0; i< 100; i++){
			if(len[i]) fprintf(pFile, "%d:%d ", i, len[i]);
		}
		fprintf(pFile, "\n");
		//set_print_training(0);	 
	}

	Pair *p = (Pair *)malloc(sizeof(Pair)*100);
	int size = 0;
	for(int i=0; i< 100; i++){
		if(len[i]){
			p[size].index = i;
			p[size].value = len[i];
			size++;
		}
	}
	int predict = svm_rt_predict(p, size);
	
	
	//draw finnger tips
	if(SHOWHAND){
		glPointSize(10);
		glColor3f(0,0,1);
		glBegin(GL_POINTS);
		for(int n=0; n< last; n++){
				glVertex3f(convertX(convexList[n].X), convertY(convexList[n].Y), 3.0);
			
		}

		glEnd();
	}	

	if(predict >0) return false;
	else return true;
}


/**********************************************************
					filter method
**********************************************************/
bool getEdge(XnPoint3D* List, int nNumberOfPoints, XnPoint3D palm){

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
	
	/*
	glColor3f(0, 0, 0);
	glBegin(GL_POINTS);
	glVertex3f(convertX(highest->X), convertY(highest->Y), 4.1f);
	glVertex3f(convertX(lowest->X), convertY(lowest->Y), 4.1f);
	glVertex3f(convertX((highest->X+lowest->X)/2), convertY((highest->Y+lowest->Y)/2), 4.1f);
	glEnd();
	*/

	//hand length
	return estimateGrab(List, nNumberOfPoints, highest, lowest, leftmost, rightmost, palm);
}

//calculate the area half top of the hand area 
//greater than 50% => grab 
bool estimateGrab(XnPoint3D* list, int n, 
	XnPoint3D* highest, XnPoint3D* lowest, 
	XnPoint3D* leftmost, XnPoint3D* rightmost, XnPoint3D palm){
	


	int nX, nY, nom=0, denom=0; 
	int y = (int)palm.Y;
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
	if (percent > GRAB_THRESHOLD) return true; 
	else return false;

}

/*******************************************************
				Helper Function
********************************************************/

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

float dis(float x1, float y1, float x2, float y2){
	return sqrt(pow(x1-x2, (float)2.0) +pow(y1-y2, (float) 2));
}
