#include "stdafx.h"
#include <math.h>
#include "model.h"
#include "softSelection.h"

point_t* vlist; 
float* plist;
float maxEffect =0.5;

void translatePoint(int id, float mult, float transx, float transy){
		float prevx = vlist->pPoints[id].X;
		float prevy = vlist->pPoints[id].Y;

		vlist->pPoints[id].X = prevx+(mult*transx);
		vlist->pPoints[id].Y = prevy+(mult*transy);

		printf("id: %d, tranx: %f, transy: %f\n", id, mult*transx, mult*transy);
		//index indecates points 
		plist[id*3] = prevx+(mult*transx);	//x pointdinate
		plist[id*3+1] = prevy+(mult*transy); //y pointdinate
}

//assign max distance = 0.5
void linearfunc(float xc, float yc, float transx, float transy){
	int n = vlist->nVertexs;
	
	for(int i=0; i<n; i++){
		float dis = sqrt(pow(vlist->pPoints[i].X -xc, 2)+ pow(vlist->pPoints[i].Y -yc, 2));
		if(dis < maxEffect){
			double param = (maxEffect-dis)/maxEffect;
			translatePoint(i,pow(param, 2)/0.5, transx, transy);
		}
	}

}


void softSelect(model_t* model, int id, point_t* vertextlist,float transx, float transy, float* pointt){

	vlist = vertextlist;
	plist = pointt;

	//acquire the mesh
	polygon_t mesh= model->pList[id];
	
	//get its vertex id
	int* targets = mesh.p;
	
	//find vertex center of the mass (estimated)
	float xCenter = (vlist->pPoints[targets[0]].X + vlist->pPoints[targets[1]].X+ vlist->pPoints[targets[2]].X)/3;
	float yCenter = (vlist->pPoints[targets[0]].Y + vlist->pPoints[targets[1]].Y+ vlist->pPoints[targets[2]].Y)/3;

	linearfunc(xCenter, yCenter, transx, transy);
}

