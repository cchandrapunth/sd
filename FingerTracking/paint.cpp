
#include "stdafx.h"
#include "model.h"
#include "paint.h"

void paintMesh(model_t* model, int id){
	model->pList[id].color.r = 255;
	model->pList[id].color.g = 0;	
	model->pList[id].color.b = 0;
	
}