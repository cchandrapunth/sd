/*
* vertex class 
* Allow explicit look up face surrounding a vertex
* Every face can have maximum 5 faces (faceId)
* 
*/
#pragma once
class vertex
{
public:
	//coordinates
	float x, y, z;	

	//faces
	int max;
	int *faceId;
	int nface;

	//functions
	vertex(float, float, float);
	//~vertex();
	int addFaceId(int);

	//debug
	void printv();
	void printface();
};

