#include "stdafx.h"
#include <cstdlib>
#include <cassert>
#include <iostream>
#include <math.h>


#include "model.h"
#include "miniball.h"

#define MIN_F -1000000;
#define MAX_F 1000000;

using std::cout;
using std::endl;

vertex *center;
float radius = 0;

void calBoundingSphere(){

	//find the bound
	float top = MIN_F; 
	float right = MIN_F;
	float front= MIN_F;
	float bottom = MAX_F;
	float left = MAX_F;
	float back = MAX_F;

	int numVertex = getnPoint();
	float* v = getPoint();	//important! vertices => x,y,z coordinates stored

	//iterate over all vertices in the model
	for(int id =0; id < numVertex*3; id+=3){

		float x = v[id];
		float y = v[id+1];
		float z = v[id+2];

		//find the bounding box
		if(x < left) left = x;
		else if(x > right) right = x;
		
		if(z < back) back = z;
		else if(z > front) front = z;
		
		if(y <bottom) bottom = y;
		else if(y >top) top = y;

		//find the model's center from the bounding box
		center = new vertex((left+right)/2, (bottom+top)/2, (back+front)/2);

	}
	printf("center: %f, %f, %f\n", center->x, center->y, center->z);

	//calculate the distance from the center
	//and find the radius 
	
	for(int i =0; i < numVertex*3; i+=3){
		float x = v[i];
		float y = v[i+1];
		float z = v[i+2];

		float dis = sqrt(pow(x-center->x, 2)+pow(y-center->y, 2)+pow(z-center->z, 2));
		if(dis >radius) radius = dis;
	}
	printf("radius: %f\n", radius);

	float result[4] = {center->x, center->y, center->z, radius}; 
} 

vertex getCenterSphere(){
	return (*center);
}

float getDiamSphere(){
	return radius*2;
}