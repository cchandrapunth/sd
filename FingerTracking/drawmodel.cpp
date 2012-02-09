#include "stdafx.h"
#include <stdlib.h>
#include "gl/glut.h"
#include "drawmodel.h"
#include <deque>
#include "mesh.h"
#include "vertex.h"
#include "vmmodel.h"

#include "model.h"

#include "undo.h"
#include "softSelection.h"
#include "miniball.h"
#include "picking.h"

//translate scene
float rollvX = 0; 
float rollvY = 0;
float zoomvZ;

//store new roll input 
int rotX, rotY;
float zoom, rate = 500;

int getRotX(){ return rotX; }
int getRotY(){ return rotY; }

void drawVMModel(){
	//qraternian

	float mat[16];
	

	glLoadIdentity();
	glEnable(GL_TEXTURE_2D);

	for(int j=0; j< getFaceListSize(); j++){
		glPushMatrix();
		glPushName(j);

		glLoadIdentity();
		trackRoll();
		//glGetFloatv(GL_MODELVIEW_MATRIX, mat);

		if(getSelection() == j){
			printf("select %d\n", getSelection());
			glBindTexture(GL_TEXTURE_2D, 3);	//green
		}
		else{
			setColorPaint(j);
		}

		drawMesh(j);

		glPopName();
		glPopMatrix();
		//glMultMatrixf(mat);	
	}
}

void drawPickVMModel(){

	glDisable(GL_DITHER); //disable blending color function
	glDisable(GL_LIGHT0);
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);

	for (int i=0; i< getFaceListSize(); i++){
		glPushMatrix();

		trackRoll();
		if(i < 255){
			glColor3ub(255,255, i);	
		}
		else printf("TOO MANY MASH\n");

		drawMesh(i);
		glPopMatrix();
	}
	

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_DITHER);
	glEnable(GL_TEXTURE_2D);
}

void trackRoll(){

	//the old state+ the new adjustment for this grab+ current roll in this second
	rotX = restoreMatX()+getMatX()+ (int)rollvX;
	rotY = restoreMatY()+getMatY()+ (int)rollvY;
	zoom = restoreMatZ()+getMatZ()+zoomvZ;

	vertex c = getCenterSphere();
	glTranslated(c.x, c.y, c.z);
	glRotated(-rotX, 0, 1, 0);	//rotate around y axis
	glTranslated(-c.x, -c.y, -c.z);

	glTranslated(c.x, c.y, c.z);
	glRotated(-rotY, 1, 0, 0);	//rotate around x axis
	glTranslated(-c.x, -c.y, -c.z);
	
	//when hand lost -> #INF
	if(abs(zoom) >1){
		glTranslated(c.x, c.y, c.z);
		glScalef(1+(float)zoom/rate, 1+(float)zoom/rate, 1+(float)zoom/rate);
		glTranslated(-c.x, -c.y, -c.z);
	}

	addMatrix((int)rollvX, (int)rollvY, zoomvZ);

	//reset
	rollvX=0;
	rollvY=0;
	zoomvZ=0;
}

void commitScene(float transx, float transy, float z){
	//set variable for rotate/zoom
	rollvX = transx;
	rollvY = transy;
	zoomvZ = z;
}

