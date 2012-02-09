#include "StdAfx.h"
#include "mesh.h"
#include "log.h"

//constructor
mesh::mesh(int id1, int id2, int id3)
{
	ind1 = id1;
	ind2 = id2;
	ind3 = id3;
	colorId  =1;	//default color is red

	normalX = 0; normalY = 0; normalZ = 0;
}

/*
mesh::~mesh(){
	delete this;
}
*/

void mesh::setColor(int id){
	colorId = id;
}

void mesh::printmesh(){
	printf("ind %d, %d, %d \t| normal %.5f, %.5f, %.5f \t| colorId = %d\n", ind1, ind2, ind3, normalX, normalY, normalZ, colorId);
}
