
//This component handle the color picking 
#include "stdafx.h"
#include <stdlib.h>
#include <GL/glut.h>
#include <XnCppWrapper.h>
#include <XnVNite.h>

#include "render.h"
#include "gesture.h"
#include "hand_history.h"
#include "window.h"
#include "model.h"

#include "undo.h"


// static //
static camera cam;
static int mainWindow;
static int border =6, h=480, w= 800; 

// selection buffer //
#define SelBuffSize 512 
bool BACK_BUFF = false;

// picking stuff //
#define RENDER	1
#define SELECT	2
#define BUFSIZE 1024 
GLuint selectBuf[BUFSIZE];
GLint hits; 
int mode = RENDER; 
int cursorX, cursorY; 
int pickMe = 0;			//which one is picked

//gesture 
#define GESTURE_TO_USE "Click" 
#define MAXPOINT 30000
xn::DepthGenerator *ptr_DepthGen;
xn::Context context;
bool stateGrab = false; //0- not grab, 1 - already in grab

//ortho
XnUInt16 g_nXRes, g_nYRes;

//model
model_t sampleModel;
point_t samplePoint; 

//---------------------------------
//         Resizeing 
//---------------------------------

void reshape(int w1, int h1){

	float ratio; 
	h = h1; 
	w = w1; 
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
	gluLookAt(cam.pos[0], cam.pos[1], cam.pos[2], cam.lookAt[0], cam.lookAt[1], cam.lookAt[2], cam.lookUp[0], cam.lookUp[1], cam.lookUp[2]);
	// Set the clipping volume
	//ratio = 1.0f * w / h;
	//gluPerspective(45,ratio,0.1,1000);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

}

//----------------------------------------
//			PICKING STUFF
//----------------------------------------
void processPick(){
	GLint viewport[4];
	GLubyte pixel[3];

	//ask for value of the viewport
	glGetIntegerv(GL_VIEWPORT, viewport);

	//read pixel under the curser
	glReadPixels(cursorX, viewport[3]-cursorY, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, (void*) pixel);

	printf("%d %d %d\n", pixel[0], pixel[1], pixel[2]);

	if (pixel[0] == 255 && pixel[1] == 0 && pixel[2] == 0){
	  printf ("You picked the 1st rect");
	  pickMe = 0;
	}
	else if (pixel[0] == 0 && pixel[1] == 255 && pixel[2] == 0){
	  printf ("You picked the 2nd rect");
	  pickMe = 1;
	}
	else if (pixel[0] == 0 && pixel[1] == 0 && pixel[2] == 255){
	  printf ("You picked the 3rd rect");
	  pickMe = 2;
	}
	else if (pixel[0] == 255 && pixel[1] == 255 && pixel[2] == 0){
	  printf ("You picked the 4rd rect");
	  pickMe = 3;
	}
	else if (pixel[0] == 0 && pixel[1] == 255 && pixel[2] == 255){
	  printf ("You picked the 5rd rect");
	  pickMe = 4;
	}
	else if (pixel[0] == 255 && pixel[1] == 0 && pixel[2] == 255){
	  printf ("You picked the 6rd rect");
	  pickMe = 5;
	}
	else if (pixel[0] == 130 && pixel[1] == 0 && pixel[2] == 130){
	  printf ("You picked the 7rd rect");
	  pickMe = 6;
	}
	else if (pixel[0] == 0 && pixel[1] == 130 && pixel[2] == 130){
	  printf ("You picked the 8rd rect");
	  pickMe = 7;
	}
	else{
	   printf("You didn't click any rect!");
	   pickMe = -1;
	 }
	printf ("\n");

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
			if(pickMe >0){
				translatePoly(sampleModel.pList[pickMe], &samplePoint,gettranslateX(), gettranslateY());
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
//-------------------------------------
//          Rendering
//-------------------------------------

void renderScene(){

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
		processPick();
		mode = RENDER;
	}
	else glutSwapBuffers();

	
	context.WaitAndUpdateAll();

	glFlush();
	
}

//------------------------------------------------
//				Keyboard and mouse
//------------------------------------------------

void processNormalKeys(unsigned char key, int x, int y){
	
	if(key ==27){			//'esc' to exit
		quit();
	}
	else if(key == 104){	//'h' to show handmap or palmpoint
		switchHandMode();
	}
	else if(key == 100){	//'d' to show front buffer or back bufferr
		BACK_BUFF = !BACK_BUFF;
	}
	else
		processKeyboard(key, x, y);
}

void mouse(int button, int state, int x, int y){

	if (button != GLUT_LEFT_BUTTON || state != GLUT_DOWN)
	return;

	//only when the left button is clicked 
	cursorX = x;
	cursorY = y;
	mode = SELECT;
}
void initRender(){
	
	//model_display_list = createDL();


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
	glEnable(GL_DEPTH_TEST);				//don't forget to enable depth test
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
	glutDisplayFunc(renderScene);
	glutMouseFunc(mouse);
	glutIdleFunc(renderScene);

	initRender();
	init(&cam);

	createGLUTMenus();
	glutMainLoop();
}


void kinectInit(){

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
	
	debugGesture();

	kinectInit();
	glInit(argc, argv); 


	context.Shutdown();
	return(0);
}