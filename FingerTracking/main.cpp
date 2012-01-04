
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

#include "picking.h"
#include "gesture.h"
#include "hand_history.h"
#include "window.h"
#include "model.h"
#include "undo.h"


//----------------------------------------------------------------
//							Variable
//----------------------------------------------------------------

#define SelBuffSize 512		//selection buffer 
#define BUFSIZE 1024 
GLuint selectBuf[BUFSIZE];

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

//gesture 
#define GESTURE_TO_USE "Click" 
#define MAXPOINT 30000
xn::DepthGenerator *ptr_DepthGen;
xn::Context context;
bool stateGrab = false; //0- not grab, 1 - already in grab


//Model
model_t sampleModel; 
point_t samplePoint; 


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
			cursorX = g_nXRes - getPalm().X;
			cursorY = getPalm().Y;
			mode = SELECT; 
			stateGrab = true;
			Beep(750,50);				//play sound lol
		}
		//stay in grab position should not be recognized as grab
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

void mainloop(){

	XnPoint3D *handPointList = new XnPoint3D[MAXPOINT];
	if(handPointList == NULL){
		printf("error. can't allocate memory for handPointList");
	}

	glutSetWindow(mainWindow);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();


	//check grabing, store palm
	checkCursor();

	if(mode == SELECT)
		drawPickMe(&sampleModel, &samplePoint);
	else {
		drawHand(handPointList);
		if(!BACK_BUFF)
		drawMe(&sampleModel, &samplePoint);
		else drawPickMe(&sampleModel, &samplePoint);
	}
	if(mode == SELECT){
		processPick(cursorX, cursorY);
		mode = RENDER;
	}
	else glutSwapBuffers();

	context.WaitAndUpdateAll();

	glFlush();
	
}

void reshape(int w1, int h1){

	float ratio; 
	int h = h1; 
	int w = w1; 

	// prevent divide by zero 
	if(h1 ==0) h=1; 
	glViewport(0, 0, w, h); 

	//PROJECTION: set window coordinate
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity(); 

	XnMapOutputMode mode;
	(*ptr_DepthGen).GetMapOutputMode(mode);

	g_nXRes = mode.nXRes;
	g_nYRes = mode.nYRes;

	//set the clipping volume corresponding to the depthmap resolution
	//left, right, buttom, top
	glOrtho(0, g_nXRes, 0, g_nYRes, -100, 1000);
	gluLookAt(80, 0, 0, 80, 0, -10, 0, 1, 0);

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

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_DEPTH_TEST);		//don't forget to enable depth test
	glEnable(GL_NORMALIZE);			//automatically rescale normal when transform the surface
	glEnable(GL_DEPTH_TEST);

}

void glInit(int argc, char **argv){

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(300, 100);
	glutInitWindowSize(w,h);
	mainWindow = glutCreateWindow("picking back buffer");

	glutKeyboardFunc(processNormalKeys);
	glutReshapeFunc(reshape);
	glutDisplayFunc(mainloop);
	glutMouseFunc(mouse);
	glutIdleFunc(mainloop);
	initRender();

	createGLUTMenus();
	glutMainLoop();
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
	glInit(argc, argv); 

	context.Shutdown();
	return(0);
}