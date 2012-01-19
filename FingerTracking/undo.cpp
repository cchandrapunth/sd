
//essentially keep track of the model transformation 
//call from translatePoly in model.cpp
//anable undo/redo 

#include "stdafx.h"
#include <stdio.h>
#include <deque>

#include "model.h"
#include "undo.h"


//should be changable by the users
#define maxAct 30
#define maxControl 10

std::deque<model_state> actionList;
std::deque<matrix_state> controlList;

static int rotateSceneX =0;
static int rotateSceneY =0;

//model history
void storeModelHist(){
	//access model data from model.cpp
	int numVertex = getnPoint();
	int numPolygon = getnPoly();
	float* vertices = getPoint();	//important! vertices => x,y,z coordinates stored
	int* polygons = getPoly();		//important! polygon => 4 points made quad

	float* tempv = (float*) malloc(sizeof(float)* numVertex*3);
	int* tempp = (int*) malloc(sizeof(int)* numPolygon*4);

	printf("store vertex\n");
	for(int i=0; i< numVertex*3; i++){
		tempv[i] = vertices[i];
		printf("%f ", vertices[i]);
	}
	for(int j=0; j< numPolygon*4; j++){
		tempp[j] = polygons[j];
	}

	//initialize model state
	model_state ms;
	ms.nPoint = numVertex;
	ms.nPoly = numPolygon;
	ms.point_st = tempv;
	ms.poly_st = tempp;

	actionList.push_front(ms);

}

void undo_m(){

	if(actionList.size() > 1){
		//redo the inverse translation 
		model_state ms = actionList.at(1);
		//copy back to the data structure

		int numVertex = ms.nPoint;
		int numPolygon = ms.nPoly;
		float* vertices = getPoint();
		int* polygons = getPoly();

		printf("\nrevert model\n");
		//x, y, z
		for(int i=0; i< numVertex*3; i++){
			vertices[i] = ms.point_st[i];
			printf("%f ", vertices[i]);
		}
		//quad
		for(int j=0; j< numPolygon*4; j++){
			polygons[j] = ms.poly_st[j];
		}

		LoadModel(&samplePoint, &sampleModel);
		actionList.pop_front();
	}
}

void addMatrix(int x, int y){
	rotateSceneX += x;
	if(rotateSceneX > 360) rotateSceneX -=360; 

	rotateSceneY += y;
	if(rotateSceneY > 360) rotateSceneY -=360; 
}

int getMatX() {return rotateSceneX;}
int getMatY() {return rotateSceneY;}

//prepare for undo
void pushMatrix(){
	matrix_state* ms= (matrix_state*) malloc (sizeof(matrix_state));
	ms->rollX = rotateSceneX; 
	ms->rollY = rotateSceneY;

	if(!controlList.empty()){
		ms->rollX += controlList.front().rollX;
		if(ms->rollX > 360) ms->rollX -=360;
		ms->rollY += controlList.front().rollY;
		if(ms->rollY > 360) ms->rollY -=360;
	}

	controlList.push_front(*ms);
	if(controlList.size() > maxControl){
		controlList.pop_back();
	}

	rotateSceneX = 0;
	rotateSceneY = 0;
}

int restoreMatX(){
	if(controlList.empty()) return 0;
	return controlList.front().rollX;
}

int restoreMatY(){
	if(controlList.empty()) return 0;
	return controlList.front().rollY;
}