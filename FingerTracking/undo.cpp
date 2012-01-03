
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

std::deque<model_state> actionList;


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

