#include "stdafx.h"
#include <stdlib.h>
#include "gl/glut.h"
#include "drawmodel.h"
#include <deque>
#include "mesh.h"
#include "vertex.h"
#include "vmmodel.h"
#include <math.h>

#include "undo.h"
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
	
	glPushMatrix();
	glLoadIdentity();
	trackRoll();
	draw_fill_model();
	draw_line_effect();

	glPopMatrix();
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	

	/*
	glLoadIdentity();
	glEnable(GL_TEXTURE_2D);

	glPushMatrix();
	glLoadIdentity();
	trackRoll();

	for(int j=0; j< getFaceListSize(); j++){
		
		glPushName(j);
		//glGetFloatv(GL_MODELVIEW_MATRIX, mat);

		if(sListContain(j) >= 0 || getSelection() == j){
			glBindTexture(GL_TEXTURE_2D, 3);	//green

			//gizmo
			glDisable(GL_TEXTURE_2D);
			glDisable(GL_LIGHTING);	
			setGizmo(j);
			glEnable(GL_TEXTURE_2D);
			glEnable(GL_LIGHTING);
		}
		else{
			setColorPaint(j);
		}
		
		//polygon
		glPolygonMode(GL_FRONT, GL_FILL);
		drawMesh(j, false);
	

		//Contour line
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_LIGHTING);	
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		drawMesh(j, true);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_LIGHTING);
		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);

		glPopName();
		//glMultMatrixf(mat);	
	}

	//drawGizmo();
	glPopMatrix();	
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	*/
}

void draw_fill_model(){

	glPolygonMode(GL_FRONT, GL_FILL);

	
	for(int j=0; j< getFaceListSize(); j++){
		
		glPushName(j);

	glColor3f(0, 1, 0);

		if(sListContain(j) >= 0 || getSelection() == j){
			//glBindTexture(GL_TEXTURE_2D, 3);	//green
			glColor3f(0, 1, 0);
			setMeshSelection(j);
		}
		else{
			setColorPaint(j);
		}
		
		
		//polygon
		drawMesh(j, false);
		glPopName();
	}
	
}

void draw_line_effect(){
	//Contour line
		glDisable(GL_LIGHTING);	
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);//line shows more
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glLineWidth(1);

		
		for(int j=0; j< getFaceListSize(); j++){
			drawMesh(j, true);
		}
		

		glEnable(GL_LIGHTING);
		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
}

void drawPickVMModel(){

	//glDisable(GL_DITHER); //disable blending color function
	glDisable(GL_LIGHT0);
	glDisable(GL_LIGHTING);
	glPushMatrix();
	trackRoll();
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	for (int i=0; i< getFaceListSize(); i++){

		//binary representative
		//0-255
		if(i < pow(2.0,8.0)){
			glColor3ub(0,0, i);	
		}
		//256-65,535
		else if(i < pow(2.0,16.0)){
			glColor3ub(0, (int)i/256, (i%256)-1 );
		}
		else printf("error: TOO MANY MASH\n");

		drawMesh(i, false);
		
	}
	glPopMatrix();

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
}

void trackRoll(){

	//the old state+ current roll in this second
	rotX = getMatX()+(int)rollvX;
	rotY = getMatY()+(int)rollvY;
	zoom = getMatZ()+zoomvZ;

	//debug
	rotX = 180;
	rotY = 90;

	vertex c = getCenter();
	glTranslated(c.x, c.y, c.z);
	glRotated(-rotX, 0, 1, 0);	//rotate around y axis

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


