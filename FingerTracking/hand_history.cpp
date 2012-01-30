#include "stdafx.h"
#include <stdio.h>
#include <XnCppWrapper.h>
#include <XnVNite.h>
#include <deque>


#include "hand_history.h";

//helper for the real time effect 
//allow the model to react to movement of the hand during grab gesture

#define MAX_HANDLIST 2000
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

//movement in x-axis
float gettranslateX(){
	return (handList.size() > 1) ? handList.at(1).X -handList.front().X : 0;
	
}

//movement in y-axis
float gettranslateY(){
	return (handList.size() > 1) ? handList.at(1).Y -handList.front().Y : 0;
}