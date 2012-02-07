#include "stdafx.h"
#include <stdlib.h>
#include "gl/glut.h"
#include "drawmodel.h"
#include <deque>
#include "mesh.h"
#include "vertex.h"
#include "vmmodel.h"

void drawVMModel(){

    glLoadIdentity();
	glEnable(GL_TEXTURE_2D);

	for(int j=0; j< getFaceListSize(); j++){
			glPushMatrix();
			glPushName(j);
			
			glLoadIdentity();
			//handleRoll();
				
			/*
			if(getSelection() == j){
				glBindTexture(GL_TEXTURE_2D, 4);
			}
			else{
				loadColor(p);
			}
			*/
			
			drawMesh(j);
		
			glPopName();
			glPopMatrix();
	}
}