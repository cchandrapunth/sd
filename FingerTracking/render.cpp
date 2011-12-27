#include "stdafx.h"
#include <math.h>
#include <stdlib.h>
#include <GL/glut.h>
#include <stdio.h>

#include "render.h";
#include "model.h";

static GLint model_display_list;

void quit(){exit(0);}

void processKeyboard(unsigned char key, int x, int y){
	printf("key: %d\n", key);
}

void picked(GLuint name, int sw){
	printf("my name = %d in %d\n", name, sw);
}

void init(camera *cam){

	cam->pos[0] = 80.0;
	cam->pos[1] = 0.0;
	cam->pos[2] = 0.0;

	cam->lookAt[0] = 80.0;
	cam->lookAt[1] = 0.0;
	cam->lookAt[2] = -10.0;

	cam->lookUp[0] = 0;
	cam->lookUp[1] = 1;
	cam->lookUp[2] = 0;
}

/*
void drawModel(){
	//draw rectangle
	glRecti(100,100, 300, 200);
	
}

GLint createDL(){
	GLuint modelDL;
	
	//create an id for the list 
	modelDL = glGenLists(1); 

	glNewList(modelDL, GL_COMPILE);
		drawModel();
	glEndList();

	return modelDL; 
}

void draw(bool isgrab, int id, float transX, float transY){
	

	//Draw 4 rectangle 

	glColor3f(1.0f, 1.0f, 1.0f);
	for(int i= 0; i<2; i++){
		for(int j= 0; j<2; j++){
			glPushMatrix();
			glPushName(i*2+j);
			glTranslatef(i*250.0,j*180.0,0);
			glColor3f(1.0f, 1.0f, 1.0f);

			//transform the model if it is picked
			if(isgrab && id ==i*2+j){
				glColor3f(0.5f, 0.5f, 0.5f);
				glTranslatef(transX, transY, 0);
			}

			glCallList(model_display_list);
			glPopName();
			glPopMatrix();
		}
	}
}

void drawPickingMode(bool isgrab, int id, float transX, float transY){

	//Draw 4 rectangle 
	
	glDisable(GL_DITHER); //disable blending color function
	
	for(int i= 0; i< 2; i++){
		for(int j=0;j<2; j++){
			glPushMatrix();

			switch(i*2+j){
				case 0: glColor3ub(255, 0, 0); break;
				case 1: glColor3ub(0, 255, 0); break;
				case 2: glColor3ub(0, 0, 255); break; 
				case 3: glColor3ub(130, 0, 130); break;
			}

			//transform the model if it is picked
			if(isgrab && id ==i*2+j){
				glTranslatef(transX, transY, 0);
			}

			glTranslatef(i*250.0,j*180.0,0);
			glCallList(model_display_list);
			glPopMatrix();
		}
	}
	glEnable(GL_DITHER);
}
*/