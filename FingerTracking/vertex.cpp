#include "StdAfx.h"
#include "vertex.h"
#include <deque>
#include "log.h"
vertex::vertex(float a, float b, float c)
{
	max = 15;
	x = a;
	y = b;
	z = c;

	faceId = (int *) malloc(sizeof(int)*max);
	nface = 0;
	
}

/*
vertex::~vertex(){
	printf("delete\n");
	delete this;
}
*/
//auto-incremented number of face
int vertex::addFaceId(int i){
	if(nface >= max) {
		printf("unable to add face because the vertex had %d face\n", max);
		return -1;
	}
	else{
		faceId[nface] = i;
		nface++;
	}
	return 1;
}

void vertex::printv(){
	printf(" vx:%.5f, vy:%.5f, vz:%.5f\n", x, y, z);
}

void vertex::printface(){
	printf("nface= %d \t| faceId ", nface);
	for(int i=0; i< nface; i++){
		printf("%d ", faceId[i]);
	}
	printf("\n");
}


