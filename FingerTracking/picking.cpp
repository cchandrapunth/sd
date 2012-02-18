#include "stdafx.h"
#include <math.h>
#include <stdlib.h>
#include <GL/glut.h>
#include <stdio.h>

#include "picking.h";
#include "model.h";
#include "undo.h";

static GLint model_display_list;


void processPick(float cursorX, float cursorY){
	GLint viewport[4];
	GLubyte pixel[3];

	//ask for value of the viewport
	glGetIntegerv(GL_VIEWPORT, viewport);

	//read pixel under the curser
	glReadPixels(cursorX, viewport[3]-cursorY, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, (void*) pixel);

	//printf("cursorX= %f, cursorY= %f\n", cursorX, viewport[3]-cursorY);
	printf("%d %d %d\n", pixel[0], pixel[1], pixel[2]);

	if(pixel[0] == 255 && pixel[1] == 255){
		pickMe = pixel[2];
		printf("You pick %d\n", pixel[2]);
	}else if(pixel[0] == 255 && pixel[2] == 255){
		pickMe = pixel[1]+ 255; 
	}
	else{
	   printf("You didn't click any rect!");
	   pickMe = -1;
	 }
	printf ("\n");

}

int getSelection(){
	return pickMe;
}
void setNullSelection(){
	pickMe = -1;
}

void picked(GLuint name, int sw){
	printf("my name = %d in %d\n", name, sw);
}