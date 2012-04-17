#include "stdafx.h"
#include <stdlib.h>
#include <gl/glut.h>
#include <math.h>
#include "drawhand.h"
#include "vertex.h"
#include "vmmodel.h"
#include "picking.h"

int initZ = 4.5;

void drawRHand(bool grab, float x, float y, float z){
		if(getSelection() >= 0){
			drawSelection(grab);
			return;
		}

		glPointSize(12);
		if(grab) glColor3f(1.0, 0, 0);
						
		float dis = 0.1;
 
		glBegin(GL_POINTS);
		glVertex3f(x, y, initZ);
		glEnd();

		glBegin(GL_LINES);
		glVertex3f((GLfloat) x, (GLfloat) y+dis, initZ);
		glVertex3f((GLfloat) x, (GLfloat) y-dis, initZ);
		glVertex3f((GLfloat) x+dis, (GLfloat) y, initZ);
		glVertex3f((GLfloat) x-dis, (GLfloat) y, initZ);
		glEnd();
		
}

void drawLHand(bool grab, float x, float y, float z){

		glPointSize(12);
		if(grab) glColor3f(1.0, 0, 0);
						
		float dis = 0.1;

		glBegin(GL_POINTS);
		glVertex3f(x, y, initZ);
		glEnd();
}


//input selected mesh 
//draw sphere 
void drawSelection(bool grab){
	glEnable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);

	float *center = getCenterSelection();
	glPushMatrix();
	
	if(grab) glColor3f(1,0,0);  //action 
	else glColor3f(0, 0, 1);	//no action 

	glTranslatef(center[0], center[1], center[2]);
	float eff = getEffect();
	glutSolidSphere	(eff/4, 16, 16);
	glPopMatrix();

	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
}
