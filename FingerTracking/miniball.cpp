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

vertex_t center;
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

		center.X = (left+right)/2;
		center.Y = (bottom+top)/2;
		center.Z = (back+front)/2;	
	}
	printf("center: %f, %f, %f\n", center.X, center.Y, center.Z);

	//calculate the distance from the center
	//and find the radius 
	
	for(int i =0; i < numVertex*3; i+=3){
		float x = v[i];
		float y = v[i+1];
		float z = v[i+2];

		float dis = sqrt(pow(x-center.X, 2)+pow(y-center.Y, 2)+pow(z-center.Z, 2));
		if(dis >radius) radius = dis;
	}
	printf("radius: %f\n", radius);

	float result[4] = {center.X, center.Y, center.Z, radius}; 
} 

vertex_t getCenterSphere(){
	return center;
}

float getDiamSphere(){
	return radius;
}
/*
int main (int argc, char* argv[])
{
  using std::cout;
  using std::endl; 
  srand (18);   

  const int       d = 5;
  const int       n = 100000;
  Miniball<d>     mb;
   
  // generate random points and check them in
  // ----------------------------------------
  Point<d> p;
  for (int i=0; i<n; ++i) {
    for (int j=0; j<d; ++j)
      p[j] = rand();
    mb.check_in(p);
  }
   
  // construct ball
  // --------------
  cout << "Constructing miniball..."; cout.flush();
  mb.build();
  cout << "done." << endl << endl;
   
  // output center and squared radius
  // --------------------------------
  cout << "Center:         " << mb.center() << endl;
  cout << "Squared radius: " << mb.squared_radius() << endl << endl;
   
  // output number of support points
  // -------------------------------
  cout << mb.nr_support_points() << " support points: " << endl << endl;
   
  // output support points
  // ---------------------
  Miniball<d>::Cit it;
  for (it=mb.support_points_begin(); it!=mb.support_points_end(); ++it)
    cout << *it << endl;
  cout << endl;
   
  // output accuracy
  // ---------------
  double slack;
  cout << "Relative accuracy: " << mb.accuracy (slack) << endl;
  cout << "Optimality slack:  " << slack << endl;

  // check validity (even if this fails, the ball may be acceptable, 
  // see the interface of class Miniball)
  // ------------------------------------
  cout << "Validity: " << (mb.is_valid() ? "ok" : "possibly invalid") << endl;
   
  return 0;
}
*/