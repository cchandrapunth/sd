#include "stdafx.h"
#include <stdio.h>
#include <XnCppWrapper.h>
#include <XnVNite.h>
#include <deque>
#include <math.h>
#include "vertex.h"


#include "hand_history.h";

//helper for the real time effect 
//allow the model to react to movement of the hand during grab gesture

#define MAX_HANDLIST 8
std::deque<XnPoint3D> handList;

//list of palm position when grab occurs
//keep the newest element at the front. maintain size at MAX_HANDLIST
void storeHand(XnPoint3D p){
	handList.push_front(p);
	if(handList.size() > MAX_HANDLIST)
		handList.pop_back();
}

//clear the list after released
void clearHandList(){
	handList.clear();
}

vertex normalize(){
	float vx = handList.at(1).X -handList.front().X;
	float vy = handList.at(1).Y -handList.front().Y;
	float vz = handList.at(1).Z -handList.front().Z;
	/*
	int unit = sqrt(pow(vx, 2)+ pow(vy, 2)+ pow(vz, 2));
	vertex uv = new vertex(vx/unit, vy/unit, vy/unit);
	printf("x: %.2f, y: %.2f, z: %.2f\n", uv.x, uv.y, uv.z);
	*/
	vertex uv = new vertex(vx, vy, vz);
	return uv;
}

//movement in x-axis
float gettranslateX(){
	return (handList.size() > 1) ? normalize().x : 0;
}

//movement in y-axis
float gettranslateY(){
	return (handList.size() > 1) ? normalize().y : 0;
}

float gettranslateZ(){
	return (handList.size() > 1) ? normalize().z : 0;
	
}

