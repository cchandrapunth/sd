
#include "stdafx.h"
#include <GL/glut.h>

#include "window.h"

#define RED 1
#define GREEN 2
#define BLUE 3
#define ORANGE 4


void createGLUTMenus() {

	int menu;

	// create the menu and
	// tell glut that "processMenuEvents" will
	// handle the events
	menu = glutCreateMenu(processMenuEvents);

	//add entries to our menu
	glutAddMenuEntry("Red",RED);
	glutAddMenuEntry("Blue",BLUE);
	glutAddMenuEntry("Green",GREEN);
	glutAddMenuEntry("Orange",ORANGE);

	// attach the menu to the right button
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}


void processMenuEvents(int option) {

	switch (option) {
		case RED :
			printf("red"); break;
		case GREEN :
			printf("green"); break;
		case BLUE :
			printf("blue"); break;
		case ORANGE :
			printf("orange"); break;
	}
}
