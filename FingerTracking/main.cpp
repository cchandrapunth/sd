
/* Chantree Chandrapunth
*  Senior Project: Virtual Sculpture 
*
*  Main loop controls the workflow while connecting 
*  the gesture and the graphical component together. 
*  Handle most of the rendering part
*/

#include "stdafx.h"
#include <stdlib.h>
#include <GL/glut.h>
#include <XnCppWrapper.h>
#include <XnVNite.h>
#include "glui.h"

#include "picking.h"
#include "gesture.h"
#include "hand_history.h"
#include "window.h"
#include "model.h"
#include "undo.h"
#include "ui.h"
#include "miniball.h"


//----------------------------------------------------------------
//							Variable
//----------------------------------------------------------------

#define SelBuffSize 512		//selection buffer 
#define BUFSIZE 1024 


//rendering mode
#define RENDER	1	
#define SELECT	2	

// static //
static int mainWindow;
static int border =6, h=480, w= 800; 
static float cursorX, cursorY; 
static int mode = RENDER; 	
static XnUInt16 g_nXRes, g_nYRes;
static bool BACK_BUFF;	//show back buffer
static XnPoint3D *handPointList;

GLuint selectBuf[BUFSIZE];

//gesture 
#define GESTURE_TO_USE "Click" 
#define MAXPOINT 30000
xn::DepthGenerator *ptr_DepthGen;
xn::Context context;
bool stateGrab = false; //0- not grab, 1 - already in grab


//Model
model_t sampleModel; 
point_t samplePoint; 

//ui
ui *Master_ui =new ui();

// feature
bool sculpting = false;
bool control = true;
bool paint = false;

//paint

#define checkImageWidth 64
#define checkImageHeight 64

static GLubyte checkImage[checkImageWidth][checkImageHeight][4];
static GLubyte redTex[4];
static GLubyte blueTex[4];
static GLubyte greenTex[4];
static GLuint texName[4];

//viewport

float zNear;
float zFar; 
GLdouble left; 
GLdouble right; 
GLdouble bottom; 
GLdouble top; 

//---------------------------------------------------------------
//				Keyboard and mouse
//----------------------------------------------------------------


void mouse(int button, int state, int x, int y){

	if (button != GLUT_LEFT_BUTTON || state != GLUT_DOWN) return;

	//only when the left button is clicked 
	cursorX = x;
	cursorY = y;
	mode = SELECT;
}

void processNormalKeys(unsigned char key, int x, int y){
	
	if(key ==27){			//'esc' to exit
		exit(0);
	}
	else if(key == 104){	//'h' to show handmap or palmpoint
		switchShowHand();
	}
	else if(key == 100){	//'d' to show front buffer or back bufferr
		BACK_BUFF = !BACK_BUFF;
		if(BACK_BUFF) printf("switch buffer to front\n");
		else 	printf("switch buffer to back\n");
	}
	else if(key == 'l'){
		enable_line();
	}
	else
		printf("key: %d\n", key);
}

//treat grab as a mouse click. 
void checkCursor(int func){
	
	if(isGrab()) {
		//keep the movement history
		storeHand(getPalm());

		//first time grab gesture occurs
		if(!stateGrab) {	
			//adjust with width and height of the screen
			cursorX = (g_nXRes-getPalm().X)*w/g_nXRes;
			cursorY = getPalm().Y*h/g_nYRes;
			mode = SELECT; 
			stateGrab = true;
			//Beep(750,50);				//play sound

		}
		//still in grab gesture
		else{
			mode = RENDER;
			if(func == 1) {
				if(getSelection() >0){
					translatePoly(&sampleModel, getSelection(), &samplePoint, gettranslateX(), gettranslateY());
					calculateNormal(&samplePoint, &sampleModel);
				}
			}
			else if(func ==2){
				translateScene(gettranslateX(), gettranslateY(), gettranslateZ());
				calculateNormal(&samplePoint, &sampleModel);
			}
			else if(func ==3){
				if(getSelection() >0){
					setColor(&sampleModel, getSelection(), 3);
				}
			}
		}
	}
	else{
		//just release
		if(stateGrab){
			stateGrab = false; 
			clearHandList();
			setNullSelection();

			//undo
			if(func == 1) storeModelHist(); 
			else if(func ==2) pushMatrix(); 
		}
	}

}
//------------------------------------------------------------------
//								Texture & UI
//------------------------------------------------------------------

void makeTexImage(){
	int i, j, c;

	//check
	for(i=0; i< 64; i++){
		for(j=0; j< 64; j++){
			c = (((i&0x8)==0) ^ ((j&0x8))==0)*255;
			
			checkImage[i][j][0] = (GLubyte) c;
			checkImage[i][j][1] = (GLubyte) c;
			checkImage[i][j][2] = (GLubyte) c;
			checkImage[i][j][3] = (GLubyte) 255;
		}	
	}

	//red
	redTex[0] = 255;
	redTex[1] = 0;
	redTex[2] = 0;
	redTex[3] = 255;

	//blue
	blueTex[0] = 0;
	blueTex[1] = 0;
	blueTex[2] = 255;
	blueTex[3] = 255;

	//green
	greenTex[0] = 0;
	greenTex[1] = 255;
	greenTex[2] = 0;
	greenTex[3] = 255;
}
void UIhandler(){
	
	Master_ui->check_click(convertX(getPalm().X), convertY(getPalm().Y));
}

//------------------------------------------------------------------
//								display
//------------------------------------------------------------------
void display(){

	if(glutGetWindow()!=mainWindow)  
		glutSetWindow(mainWindow);
	if(handPointList == NULL){
		printf("error. can't allocate memory for handPointList");
	}

	glutSetWindow(mainWindow);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glLoadIdentity();

	UIhandler(); //check ui touch

	//-------------------sculpting--------------------------
	
	if(sculpting){
		//check grabing, store palm
		checkCursor(1); 
		
		if(mode == SELECT){
			drawPickMe(&sampleModel, &samplePoint);
			processPick(cursorX, cursorY);
			mode = RENDER;
		}
		else {
			drawHand(handPointList);
			if(!BACK_BUFF)
				drawMe(&sampleModel, &samplePoint);
			else drawPickMe(&sampleModel, &samplePoint);
			glutSwapBuffers();
		}

	}
	//-------------------control----------------------------
	else if(control) {
		checkCursor(2); 

		if(mode == SELECT){
			drawPickMe(&sampleModel, &samplePoint);
			mode = RENDER;
		}
		else {
			drawHand(handPointList);
			if(!BACK_BUFF)
				drawMe(&sampleModel, &samplePoint);
			else drawPickMe(&sampleModel, &samplePoint);
			glutSwapBuffers();
		}
	}
	else if(paint) {
		checkCursor(3);

		if(mode == SELECT){
			drawPickMe(&sampleModel, &samplePoint);
			processPick(cursorX, cursorY);
			mode = RENDER;
		}
		else {
			drawHand(handPointList);
			if(!BACK_BUFF)
				drawMe(&sampleModel, &samplePoint);
			else drawPickMe(&sampleModel, &samplePoint);
			glutSwapBuffers();
		}
	}

	context.WaitAndUpdateAll();
	
	glutSetWindow(mainWindow); //set current GLUT window before rendering
	glFlush();
	
}

//-------------------------------------------------------------------------
//									Reshape
//-------------------------------------------------------------------------
void reshape(int w1, int h1){

	glutSetWindow(mainWindow); 

	h = h1; 
	w = w1; 
	if(h1 ==0) h=1; 
	glViewport(0, 0, w, h); 

	/*
	float aspect= (float)w/h;
	if ( aspect < 1.0 ) { // window taller than wide
     bottom /= aspect;
     top /= aspect;
   } else {
     left *= aspect;
     right *= aspect;
   }*/

	//PROJECTION: set window coordinate
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity(); 

	XnMapOutputMode mode;
	(*ptr_DepthGen).GetMapOutputMode(mode);

	g_nXRes = mode.nXRes;
	g_nYRes = mode.nYRes;

	//set the clipping volume corresponding to the viewport
	//left, right, buttom, top
	glOrtho(left, right, bottom, top, zNear, zFar);


	printf("l: %f, r:%f, bot: %f, top: %f, zN: %f, zF: %f\n", left, right, bottom, top, zNear, zFar);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

}

//----------------------------------------------------------------
//								INIT
//----------------------------------------------------------------
void initTex(void){
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glShadeModel(GL_FLAT);
	glEnable(GL_DEPTH_TEST);
	makeTexImage();
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glGenTextures(4, texName);

	glBindTexture(GL_TEXTURE_2D, texName[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, checkImageWidth, checkImageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, checkImage);
	
	
	glBindTexture(GL_TEXTURE_2D, texName[1]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, redTex);

	glBindTexture(GL_TEXTURE_2D, texName[2]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, blueTex);

	glBindTexture(GL_TEXTURE_2D, texName[3]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, greenTex);
}

void initRender(){

	GLfloat mat_specular[] = {1.0, 1.0, 1.0, 1.0};
	GLfloat mat_shininess[] = {50.0};
	//GLfloat light_position[] = {0.0, -100.0, 0.0, 0.0};
	GLfloat light_position[] = {-1.0, -1.0, 5, 10.0};
	GLfloat whitelight[] = {1.0, 1.0, 1.0, 1.0};
	GLfloat model_ambient[] = {1.0, 1.0, 1.0, 1.0};

	glClearColor(0.0,0.0,0.0,0.0);
	glShadeModel(GL_FLAT);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, whitelight);
	glLightfv(GL_LIGHT0, GL_SPECULAR, whitelight);

	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, model_ambient);


	ImportModel();
	LoadModel(&samplePoint, &sampleModel);
	storeModelHist();
	calBoundingSphere();

	handPointList = new XnPoint3D[MAXPOINT];

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_DEPTH_TEST);		//don't forget to enable depth test
	glEnable(GL_NORMALIZE);			//automatically rescale normal when transform the surface
	glEnable(GL_DEPTH_TEST);

}

//-----------------push menu-------------------
//sculpting
void option1(){
	printf("Ready to sculpt?\n");
	sculpting = true;
	control = false;
	paint = false;
	Master_ui->remove_menu();
}
//move camera (move/rotate object)
void option2(){
	printf("Control\n");
	sculpting = false;
	control = true;
	paint = false;
	Master_ui->remove_menu();
}
//paint brush
void option3(){
	printf("Paint\n");
	sculpting = false;
	control = false;
	paint = true;
	Master_ui->remove_menu();
}


//FIXME: should hide the menu button once it's click
void push_menu(){
	//draw panel 
	Master_ui->activate_menu = true;
	printf("pushing menu\n");

	float width = right -left;
	float height = top -bottom; 
	float off = width/20;

	Master_ui->add_button("Sculpt", left+ width/5, bottom+height/3, width/5-off, height/3, option1);
	Master_ui->add_button("Rotate", left+ width*2/5, bottom+height/3, width/5-off, height/3, option2);
	Master_ui->add_button("Paint", left+ width*3/5, bottom+height/3, width/5-off, height/3, option3);
}

//all ui in here
void uiInit(){

	vertex_t c = getCenterSphere();
	float diam = getDiamSphere();

	zNear = 0;
    zFar = 5;
	left = c.X - diam;
    right = c.X + diam;
    bottom = c.Y - diam;
    top = c.Y + diam;

	//main menu button
	Master_ui->add_button("Menu", left+0.5, bottom+0.5, 0.5, 0.3, push_menu);
}


void kinectInit(){

	//debug to gesture log file "depthmap.txt"
	enableDebugGesture();

	// Initialize context object
	XnStatus nval = context.Init();
	CHECK_RC(nval, "Initialize context");

	// Create the gesture, hand, depth object
	ptr_DepthGen = getDepthGenerator(context);
	HandsGenerator* ptr_HandsGen = getHandGenerator(context);
	GestureGenerator* ptr_GestureGen = getGestureGenerator(context);


	//Register to callbacks
	XnCallbackHandle h1, h2;
	nval = (*ptr_GestureGen).RegisterGestureCallbacks(Gesture_Recognized, Gesture_Process, NULL, h1);
	CHECK_RC(nval, "Register gesture");
	nval = (*ptr_HandsGen).RegisterHandCallbacks(Hand_Create, Hand_Update, Hand_Destroy, NULL, h2);
	CHECK_RC(nval, "Register hands");

	// Tell the context objects to start generating data
	nval = context.StartGeneratingAll();
	CHECK_RC(nval, "Start Generating All Data");

	// Calibration
	nval = (*ptr_GestureGen).AddGesture(GESTURE_TO_USE, NULL);									
	CHECK_RC(nval, "Add gesture");

}

int main (int argc, char **argv){

	kinectInit();
	
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(300, 100);
	glutInitWindowSize(w,h);
	mainWindow = glutCreateWindow("picking back buffer");

	glutKeyboardFunc(processNormalKeys);
	glutReshapeFunc(reshape);
	glutDisplayFunc(display);
	glutMouseFunc(mouse);


	initRender();
	initTex();
	uiInit();
	glutIdleFunc(display);	//enable GLUI window to take advantage of idle event

	createGLUTMenus();
	glutMainLoop();

	FreeModel(&sampleModel);
	context.Shutdown();
	return(0);
}