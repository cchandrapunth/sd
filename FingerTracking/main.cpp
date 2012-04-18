
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
#include <time.h>

#include "picking.h"
#include "gesture.h"
#include "hand_history.h"
#include "window.h"
#include "undo.h"
#include "ui.h"
#include "vertex.h"
#include "vmmodel.h"
#include "drawmodel.h"
#include "paint.h"
#include "svmtrain.h"
#include "Pair.h"
#include "svmpredict.h"
#include "drawbackground.h"
#include "mode.h" 
#include "display.h"


//----------------------------------------------------------------
//							Variable
//----------------------------------------------------------------

#define SelBuffSize 512		//selection buffer 
#define BUFSIZE 1024 
GLuint selectBuf[BUFSIZE];

static int mainWindow;
static int border =6, h=800, w= 800; 
static XnPoint3D *handPointList;


//gesture 
#define GESTURE_TO_USE "Click" 
#define MAXPOINT 30000
xn::DepthGenerator *ptr_DepthGen;
xn::Context context;


//ui
ui *Master_ui =new ui();
bool selection = false;
bool preview = false;

//paint
#define checkImageWidth 64
#define checkImageHeight 64

static GLuint texName[6];
static GLubyte checkImage[checkImageWidth][checkImageHeight][4]; //colorId 1
static GLubyte redTex[4];	//2
static GLubyte blueTex[4];	//3
static GLubyte greenTex[4];	//4
static GLubyte yellowTex[4]; //5
static GLubyte whiteTex[4];	//6


//viewport
float zNear;
float zFar; 
GLdouble left; 
GLdouble right;
GLdouble bottom; 
GLdouble top; 

//hands
hand_h* rhand;
hand_h* lhand;

//---------------------------------------------------------------
//				Keyboard and mouse
//----------------------------------------------------------------


void mouse(int button, int state, int x, int y){

	if (button != GLUT_LEFT_BUTTON || state != GLUT_DOWN) return;

	//only when the left button is clicked 
	set_cursor(x, y);
	set_state(1);
}

void processNormalKeys(unsigned char key, int x, int y){
	
	if(key ==27){			//'esc' to exit
		exit(0);
	}
	else if(key == 104){	//'h' to show handmap or palmpoint
		switchShowHand();
	}
	else if(key == 100){	//'d' to show front buffer or back bufferr
		switch_buffer();
		if(get_buffer()) printf("switch buffer to front\n");
		else 	printf("switch buffer to back\n");
	}
	else if(key == 111) {//'o' to train value = open hand
		set_print_training(2);
	}

	else if(key == 112) {// 'p' to train value = close hand
		set_print_training(1);
	}
	else if(key == 49){ //'1' for rotate X
			commitScene(-2, 0, 0);
			recalNormal();
	}
	else if(key == 50){ //'2' for rotate y
			commitScene(2, 0, 0);
			recalNormal();
	}
	else if(key == 51){ //'3' for line effect
			commitScene(0, -2, 0);
			recalNormal();
	}
	else if(key == 52){ //'4' for line effect
			commitScene(0, 2, 0);
			recalNormal();
	}
	else if(key == 53){ //'5' for line effect
			switchLine();
	}
	else if(key == 54){
		preview = !preview;
	}
	else
		printf("key: %d\n", key);
}


//------------------------------------------------------------------
//								Texture & UI
//------------------------------------------------------------------

void makeTexImage(){
	int i, j, c;

	//check board
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
	redTex[0] = 255; redTex[1] = 0; redTex[2] = 0; redTex[3] = 255;

	//blue
	blueTex[0] = 0; blueTex[1] = 0; blueTex[2] = 255; blueTex[3] = 255;

	//green
	greenTex[0] = 0; greenTex[1] = 255; greenTex[2] = 0; greenTex[3] = 255;

	//yellow 
	yellowTex[0] =255; yellowTex[1] = 255; yellowTex[2] = 0; yellowTex[3] = 255;

	//white
	whiteTex[0] = 255; whiteTex[1] = 255; whiteTex[2] = 255; whiteTex[3] = 255;

}
void UIhandler(){
	Master_ui->check_click(convertX(getPalm().X), convertY(getPalm().Y));
}

//------------------------------------------------------------------
//								display
//------------------------------------------------------------------

void display(){

	if(handPointList == NULL){
		printf("error. can't allocate memory for handPointList");
	}

	glutSetWindow(mainWindow);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	if(!preview){
		draw_background();

		glLoadIdentity();
		UIhandler(); //check ui touch

		//display
		mode_selection(handPointList, rhand, lhand);

		
	}else{
		preview_scene();
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

	float adjbottom = bottom;
	float adjtop = top;
	float adjright = right;
	float adjleft = left;

	
	float aspect= (float)w/h;
	printf("asp: %f\n", aspect);
	if ( aspect < 1.0 ) { // window taller than wide
     adjbottom /= aspect;
     adjtop /= aspect;
   } else {
     adjleft *= aspect;
     adjright *= aspect;
   }
	
	setAspect(aspect);
   

	//PROJECTION: set window coordinate
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity(); 

	XnMapOutputMode mode;
	(*ptr_DepthGen).GetMapOutputMode(mode);

	set_nRes(mode.nXRes, mode.nYRes);

	//set the clipping volume corresponding to the viewport
	//left, right, buttom, top
	glOrtho(adjleft, adjright, adjbottom, adjtop, zNear, zFar);

	//eyex, eyey, eyez, centerx, centery, centerz, upx, upy, upz
	gluLookAt(0, 0, 1, 0, 0, 0, 0, 1, 0);

	printf("l: %f, r:%f, bot: %f, top: %f, zN: %f, zF: %f\n", adjleft, adjright, adjbottom, adjtop, zNear, zFar);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

}

//----------------------------------------------------------------
//								INIT
//----------------------------------------------------------------
void initTex(void){
	glClearColor(0.1, 0.1, 0.1, 1.0);
	glEnable(GL_DEPTH_TEST);
	makeTexImage();
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glGenTextures(6, texName);
	printf("generate texture: %d, %d, %d, %d, %d, %d\n", texName[0], texName[1], texName[2], texName[3], texName[4], texName[5]);


	//check texture
	glBindTexture(GL_TEXTURE_2D, texName[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, checkImageWidth, checkImageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, checkImage);


	//plane color texture
	for(int i=1; i< 6; i++){
		glBindTexture(GL_TEXTURE_2D, texName[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		if(i==1) 
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, redTex);
		if(i==2)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, greenTex);
		if(i==3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, blueTex);
		if(i==4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, yellowTex);
		if(i==5) 
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, whiteTex);
	}
}


void initRender(){
	GLfloat lmodel_ambient[] = {0.5, 0.5, 0.5, 1.0 };
	GLfloat mat_specular[] = {0.8, 0.8, 0.8, 1.0};
	GLfloat diffuseMaterial[4] = {0.4, 0.4, 0.4, 1.0};
	GLfloat diffuse[] = {1.0,1.0,1.0,1.0};
	GLfloat light_position[] = {1.0, 2.0, 1, 1.0};
	GLfloat light_position1[] = {-1.0, 2.0, 1, 1.0};
	GLfloat light_position2[] = {0.0, -0.2, 8, 1.0};

	glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuseMaterial);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialf(GL_FRONT, GL_SHININESS, 20.0);

	//light0-1
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glEnable(GL_LIGHT2);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT1, GL_POSITION, light_position1);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT2, GL_POSITION, light_position2);
	glLightfv(GL_LIGHT2, GL_DIFFUSE, diffuse);	
	

	//glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);

	glColorMaterial(GL_FRONT, GL_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);


	//new
	import_vm();
	copy_vmmodel();
	findBoundingSphere();

	handPointList = new XnPoint3D[MAXPOINT];
	rhand = new hand_h();
	lhand = new hand_h();

	glEnable(GL_NORMALIZE);			//automatically rescale normal when transform the surface

}

//-----------------push menu-------------------
//sculpting
void option1(){
	set_mode(1);
	Master_ui->remove_menu();
}
//paint brush
void option2(){
	set_mode(2);
	Master_ui->remove_menu();

	float width = right -left;
	float height = top -bottom; 
	float off = height/80;

	Master_ui->add_button("red", left+ width/15, top-height/12, width/12, height/12-off, setRed);	//red
	Master_ui->add_button("green", left+ width/15, top-height*2/12, width/12, height/12-off, setGreen);	//green
	Master_ui->add_button("blue", left+ width/15, top-height*3/12, width/12, height/12-off, setBlue);	//blue
	Master_ui->add_button("yellow", left+ width/15, top-height*4/12, width/12, height/12-off, setYellow);	//yellow
	Master_ui->add_button("white", left+ width/15, top-height*5/12, width/12, height/12-off, setWhite);	//white
	Master_ui->add_button("black", left+ width/15, top-height*6/12, width/12, height/12-off, setBlack);	//black
}
//selection?
void option3(){
	set_mode(3);
	Master_ui->remove_menu();
}

void up(){
	if(is_mode(1)) upEffect();
	else upBrush();
}

void down(){
	if(is_mode(1)) downEffect();
	else downBrush();
}

//reload model
void reload(){
	import_vm();
}

void selectionMode(){
	selection = !selection;
	if(!selection){
		//clearSelectionList();
	}
}

void push_menu(){
	//draw panel 
	Master_ui->activate_menu = true; //addpanel
	printf("pushing menu\n");

	float width = right -left;
	float height = top -bottom; 
	float off = width/20;

	Master_ui->add_button("Sculpt", left+ width*1/4, bottom+height/3, width/4-off, height/3, option1);
	Master_ui->add_button("Paint", left+ width*2/4, bottom+height/3, width/4-off, height/3, option2);
	//Master_ui->add_button("Slice", left+ width*3/5, bottom+height/3, width/5-off, height/3, option3);
}

//all ui in here
void uiInit(){

	vertex c = getCenter();
	float diam = getDiam();

	zNear = -5;
    zFar = 100;
	left = c.x - diam;
    right = c.x + diam;
    bottom = c.y - diam;
    top = c.y + diam;

	//main menu button
	Master_ui->add_button("Menu", left+(right-left)/15, bottom+0.5, 0.5, 0.3, push_menu);
	Master_ui->add_button("-", left+(right-left)/15, bottom+0.8, 0.3, 0.3, down);
	Master_ui->add_button("+", left+(right-left)/15, bottom+1.1, 0.3, 0.3, up);

	//Master_ui->add_button("reset", right-(right-left)/5, bottom+0.5, 0.5, 0.3, reload);	//if remove, fix ui.cpp (count)
	//Master_ui->add_button("select", right-(right-left)/5, bottom+0.8, 0.5, 0.3, selectionMode);
	Master_ui->add_button("rotate", right-(right-left)/5, bottom+0.8, 0.5, 0.3, activate_rotate);
	Master_ui->add_button("undo", right-(right-left)/5, bottom+1.1, 0.5, 0.3, undo_vmmodel);
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

void initSVM(){
	
	char raw_training_set []= "training.txt";
	char training_model [] = "training_mod.txt";
	//char test_set [] = "svm_testing.txt";
	//char predicted_result [] = "svm_result.out";
	
	svm_train(raw_training_set, training_model);
	init_predict(training_model);

}
void wait ( int seconds )
{
  clock_t endwait;
  endwait = clock () + seconds * CLOCKS_PER_SEC ;
  while (clock() < endwait) {}
}


int main (int argc, char **argv){
	
	
	kinectInit();
	
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(400, 0);
	glutInitWindowSize(w,h);
	mainWindow = glutCreateWindow("picking back buffer");

	glutKeyboardFunc(processNormalKeys);
	glutReshapeFunc(reshape);
	glutDisplayFunc(display);
	glutMouseFunc(mouse);


	initRender();
	//initTex();
	uiInit();
	initSVM();
	glutIdleFunc(display);	//enable GLUI window to take advantage of idle event

	createGLUTMenus();
	glutMainLoop();

	context.Shutdown();
	return(0);
	
	
	/* svm train
	char raw_training_set []= "a1a.train.train";
	char training_model [] = "a1a.mod.txt";
	char test_set [] = "a1a.test.test";
	char predicted_result [] = "a1a.result.out";
	svm_train(raw_training_set, training_model);
	svm_predict(test_set, training_model, predicted_result);
	*/

	/*
	char raw_training_set []= "training.txt";
	char training_model [] = "training.mod.txt";
	char test_set [] = "testing.txt";
	char predicted_result [] = "result.out";
	svm_train(raw_training_set, training_model);
	svm_predict(test_set, training_model, predicted_result);
	*/

	
}