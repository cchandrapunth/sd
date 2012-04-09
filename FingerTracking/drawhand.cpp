#include "stdafx.h"
#include <stdlib.h>
#include <gl/glut.h>
#include <math.h>

int initZ = 4.5;

void drawRHand(bool grab, float x, float y, float z){

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

		/*
		glBegin(GL_LINES);
		glVertex3f((GLfloat) x, (GLfloat) y+dis, initZ);
		glVertex3f((GLfloat) x, (GLfloat) y-dis, initZ);
		glVertex3f((GLfloat) x+dis, (GLfloat) y, initZ);
		glVertex3f((GLfloat) x-dis, (GLfloat) y, initZ);
		glEnd();
		*/
}