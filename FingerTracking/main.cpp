
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
static int cursorX, cursorY; 
static int mode = RENDER; 	
static XnUInt16 g_nXRes, g_nYRes;
static bool BACK_BUFF;	//show back buffer

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
bool sculpting = true;
bool control = false;
bool paint = false;


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
	else
		printf("key: %d\n", key);
}

//treat grab as a mouse click. 
void checkCursor(){
	
	if(isGrab()) {
		//keep the movement history
		storeHand(getPalm());

		//first time grab gesture occurs
		if(!stateGrab) {
			cursorX = convertX(getPalm().X);
			cursorY = convertYcursor(getPalm().Y);
			mode = SELECT; 
			stateGrab = true;
			//Beep(750,50);				//play sound

		}
		//still in grab position should not be recognized as grab
		else{
			mode = RENDER;
			if(getSelection() >0){
				translatePoly(&sampleModel, getSelection(), &samplePoint,gettranslateX(), gettranslateY());
				calculateNormal(&samplePoint, &sampleModel);
				
			}
		}
	}
	else{
		//just release
		if(stateGrab){
			stateGrab = false; 
			clearHandList();

			storeModelHist();
		}
	}

}
//------------------------------------------------------------------
//								Rendering
//------------------------------------------------------------------
void UIhandler(){
	
	Master_ui->check_click(convertX(getPalm().X), convertY(getPalm().Y));
}

void mainloop(){

	if(glutGetWindow()!=mainWindow)  
		glutSetWindow(mainWindow);
	XnPoint3D *handPointList = new XnPoint3D[MAXPOINT];
	if(handPointList == NULL){
		printf("error. can't allocate memory for handPointList");
	}

	glutSetWindow(mainWindow);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	checkCursor(); //check grabing, store palm
	UIhandler(); //check ui touch

	//-------------------sculpting--------------------------
	if(sculpting){
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
	else if(control) {
		//render no back buffer 
		//diddn't process pick
		drawHand(handPointList);
		drawMe(&sampleModel, &samplePoint);
		glutSwapBuffers();
	}
	else if(paint) {
		//render no back buffer 
		//diddn't process pick
		drawHand(handPointList);
		drawPickMe(&sampleModel, &samplePoint);
		glutSwapBuffers();
	}

	context.WaitAndUpdateAll();
	
	glutSetWindow(mainWindow); //set current GLUT window before rendering
	glFlush();
	
}

void reshape(int w1, int h1){

	glutSetWindow(mainWindow); 

	vertex_t c = getCenterSphere();
	float diam = getDiamSphere();

	float zNear = 0.0;
    float zFar = zNear + diam;
	GLdouble left = c.X - diam;
    GLdouble right = c.X + diam;
    GLdouble bottom = c.Y - diam;
    GLdouble top = c.Y + diam;

     
	int h = h1; 
	int w = w1; 
	if(h1 ==0) h=1; 
	glViewport(0, 0, w, h); 

	/*
	float aspect= w/h;
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

	//set the clipping volume corresponding to the depthmap resolution
	//left, right, buttom, top
	//glOrtho(0, g_nXRes, 0, g_nYRes, -100, 1000);
	glOrtho(left, right, bottom, top, zNear, zFar);
	//gluLookAt(0, 0, 0, 0, 0, -10, 0, 1, 0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

}

//----------------------------------------------------------------
//								INIT
//----------------------------------------------------------------
void initRender(){


	GLfloat mat_specular[] = {1.0, 1.0, 1.0, 1.0};
	GLfloat mat_shininess[] = {50.0};
	GLfloat light_position[] = {100.0, -1000.0, 0.0, 0.0};
	GLfloat whitelight[] = {1.0, 1.0, 1.0, 1.0};
	GLfloat model_ambient[] = {0.5, 0, 0.5, 1.0};

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
	Master_ui->add_button("option1", 200, 150, 100, 250, option1);
	Master_ui->add_button("option2", 340, 150, 100, 250, option2);
	Master_ui->add_button("option3", 480, 150, 100, 250, option3);

}

//all ui in here
void uiInit(){
	//main menu button
	Master_ui->add_button("test butt", -30, 600, push_menu);
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
	glutDisplayFunc(mainloop);
	glutMouseFunc(mouse);


	initRender();
	uiInit(); 
	glutIdleFunc(mainloop);	//enable GLUI window to take advantage of idle event

	createGLUTMenus();
	glutMainLoop();

	context.Shutdown();
	return(0);
}