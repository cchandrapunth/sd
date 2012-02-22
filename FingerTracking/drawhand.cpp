#include "stdafx.h"
#include <stdlib.h>
#include <gl/glut.h>
#include <math.h>

int initZ = -1;

void drawRHand(bool grab, float x, float y, float z){

	glPointSize(12);
		if(grab) glColor3f(1.0, 0, 0);
		else glColor3f(0, 1.0, 0);
						
		float dis = 0.1;
		//keep the first depth
		if(initZ == -1){
			initZ = z;
		}
		else{
			dis += (z- initZ)/500;
			if(dis < 0) dis = 0;
		}	
 
		glBegin(GL_POINTS);
		glVertex2f(x, y);
		glEnd();

		glBegin(GL_LINES);
		glVertex2f((GLfloat) x, (GLfloat) y+dis);
		glVertex2f((GLfloat) x, (GLfloat) y-dis);
		glVertex2f((GLfloat) x+dis, (GLfloat) y);
		glVertex2f((GLfloat) x-dis, (GLfloat) y);
		glEnd();
}