#include "stdafx.h"

#include "model.h"
#include "softSelection.h"

void softSelect(model_t* model, int id, point_t* vertexlist,float transx, float transy, float* pointt){

	polygon_t p= model->pList[id];
	//translate x and y of the points in the chosen poly
	for(int k=0; k<4; k++){
		int index = p.p[k];
		
		float prevx = vertexlist->pPoints[index].X;
		float prevy = vertexlist->pPoints[index].Y;

		vertexlist->pPoints[index].X = prevx+transx;
		vertexlist->pPoints[index].Y = prevy+transy;
		
		//index indecates points 
		pointt[index*3] = prevx+transx;	//x pointdinate
		pointt[index*3+1] = prevy+transy; //y pointdinate
	}
}
