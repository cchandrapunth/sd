#include "stdafx.h"
#include <stdlib.h>
#include <gl/glut.h>
#include <fstream>
#include "mesh.h"
#include "vertex.h"
#include "vmmodel.h"
#include "log.h"

using namespace std;

static deque<mesh> faceList;
static deque<vertex> vertexList;

static deque<mesh> exfaceList;
static deque<vertex> exvertexList;

bool debug = true;
Log *pLog; 

int getFaceListSize(){	return faceList.size();};
int getVertexListSize(){	return vertexList.size();};

void import_vm(){

	pLog = new Log("vmmodel.log");

	ifstream indata;
	indata.open("modelinput.txt");
	if(!indata) {
		pLog->Write("Error: input file couldn't be opened");
		exit(1);
	}

	vertexList.clear();
	faceList.clear();

	int nvertex, nmesh;
	indata >> nvertex;
	indata >> nmesh;

	//read in vertex data
	for(int i=0; i< nvertex; i++){
		float x, y, z;
		indata >> x;
		indata >> y;
		indata >> z;
	
		vertex *v = new vertex(x, y, z);
		vertexList.push_back(*v);
	}

	//read in mesh data
	for(int j=0; j< nmesh; j++){
		int id1, id2, id3;
		indata >> id1;
		indata >> id2;
		indata >> id3;

		mesh *m = new mesh(id1, id2, id3);
		//normal vector
		vertex* v = getNormal(vertexList.at(id1), vertexList.at(id2), vertexList.at(id3));	
		m->normalX = v->x;
		m->normalY = v->y;
		m->normalZ = v->z;

		m->setColor(2);	//default is white-6
		faceList.push_back(*m);

		//fill the lookup table for vertices 
		vertexList.at(id1).addFaceId(j);
		vertexList.at(id2).addFaceId(j);
		vertexList.at(id3).addFaceId(j);
	}
	
	
	//subdivide
	int size = getFaceListSize();
	for(int i=0; i< size; i++){
		subDivide(i);
	}
	
	if(debug){
		pLog->Write("load model complete");
		printf("\n=================================================\n");
		printf("\tvertexList :: nVertex = %d\n", vertexList.size());
		printf("=================================================\n");
		for(int i=0; i< vertexList.size(); i++){
			vertexList.at(i).printv();
			vertexList.at(i).printface();
		}

		printf("\n=================================================\n");
		printf("\tfaceList :: nFace = %d\n", faceList.size());
		printf("=================================================\n");
		for(int i=0; i< faceList.size(); i++){
			faceList.at(i).printmesh();
		}
	}
}

void export_vm(){
	
	ofstream outdata;
	outdata.open("vmmodeloutput.txt");
	if(!outdata) {
		pLog->Write("Error: output file couldn't be opened\n");
		exit(1);
	}

	int nVertex = vertexList.size();
	int nMesh = faceList.size();
	outdata << nVertex << " " << nMesh << endl;

	//export vertex
	for(int i=0; i< nVertex; i++){
		vertex v = vertexList.at(i);
		outdata << v.x << "\t" << v.y << "\t" << v.z << endl;
	}

	//export mesh
	for(int i=0; i< nMesh; i++){
		mesh m = faceList.at(i);
		outdata << m.ind1 << "\t" << m.ind2 << "\t" << m.ind3 << endl;
	}
	
	outdata.close();
	if(debug){
		pLog->Write("export model complete");
		for(int i=0; i< nVertex; i++){
			vertexList.at(i).printv();
		}

		for(int i=0; i< nMesh; i++){
			faceList.at(i).printmesh();
		}
	}
}


//find normal of 3 vertices
//output pointer to norm vector
vertex* getNormal(vertex vv1, vertex vv2, vertex vv3){

	vertex* v1 = new vertex(0, 0, 0);
	vertex* v2 = new vertex(0, 0, 0);

	v1->x = vv3.x - vv1.x;
	v1->y = vv3.y - vv1.y;
	v1->z = vv3.z - vv1.z;

	v2->x = vv2.x - vv1.x;
	v2->y = vv2.y - vv1.y;
	v2->z = vv2.z - vv1.z;

	vertex *norm = new vertex(0 ,0 ,0);
	//find normal using cross product
	norm->x = (v1->y * v2->z) - (v1->z * v2->y);
	norm->y = -((v2->z * v1->x) - (v2->x * v1->z));
	norm->z = (v1->x * v2->y) - (v1->y * v2->x);	
	return normalizeV(norm);
}

//normalize
vertex* normalizeV(vertex* norm){
	float CombinedSquares = (norm->x * norm->x) +(norm->y * norm->y) +(norm->z * norm->z);
	float NormalisationFactor = sqrt(CombinedSquares);
		 norm->x = norm->x / NormalisationFactor;		
		 norm->y = norm->y / NormalisationFactor;
		 norm->z = norm->z / NormalisationFactor;
	return norm;
}

void setColorPaint(int id){
	int cid = faceList.at(id).colorId;
	//check-1, red-2, blue-3, green-4, yellow-5, white-6 
	glBindTexture(GL_TEXTURE_2D,cid);

}

void drawMesh(int meshId){

	mesh m = faceList.at(meshId);

	vertex v1 = vertexList.at(m.ind1);
	vertex v2 = vertexList.at(m.ind2);
	vertex v3 = vertexList.at(m.ind3);

	glBegin(GL_TRIANGLES);
	 glNormal3f(m.normalX, m.normalY, m.normalZ);
	 glVertex3f(v1.x, v1.y, v1.z);
	 glVertex3f(v2.x, v2.y, v2.z);
	 glVertex3f(v3.x, v3.y, v3.z);
	glEnd();
	 
}
void paintMesh(int mid, int cid){
	faceList.at(mid).colorId = cid;
}

bool sameVertex(vertex v1, vertex v2){
	if(v1.x - v2.x < 0.001 && v1.x - v2.x > -0.001 &&
			v1.y - v2.y < 0.001 && v1.y - v2.y > -0.001 &&
			v1.z - v2.z < 0.001 && v1.z - v2.z > -0.001){
				return true;
	}
	return false;
}

//divide a mesh into 4 new mesh
//handle the operation and restoration in the deque
void subDivide(int meshId){

	mesh m = faceList.at(meshId);

	vertex v1 = vertexList.at(m.ind1);
	vertex v2 = vertexList.at(m.ind2);
	vertex v3 = vertexList.at(m.ind3);


	vertex *v12 = new vertex(0, 0, 0);
	vertex *v23 = new vertex(0, 0, 0);  
	vertex *v31 = new vertex(0, 0, 0);

	v12->x = (v1.x+v2.x)/2.0;
	v12->y = (v1.y+v2.y)/2.0;
	v12->z = (v1.z+v2.z)/2.0;

	v23->x = (v2.x+v3.x)/2.0;
	v23->y = (v2.y+v3.y)/2.0;
	v23->z = (v2.z+v3.z)/2.0;

	v31->x = (v3.x+v1.x)/2.0;
	v31->y = (v3.y+v1.y)/2.0;
	v31->z = (v3.z+v1.z)/2.0;

	v12 = normalizeV(v12);
	v23 = normalizeV(v23);
	v31 = normalizeV(v31);



	//only check the vector once 
	bool v12repete = false, v23repete = false, v31repete = false;

	//assign index
	int size = vertexList.size();
	int indexv12;
	int indexv23;
	int indexv31;

	//check if the vertices already existed?
	//add only the new vertices
	//change index if neccessary
	//add face id 
	//*************

	for(int k=0; k< size; k++){
		vertex* v = &(vertexList.at(k));
		if(!v12repete && sameVertex(*v12, *v)){
			v12 = v;
			indexv12 = k;
			v12repete = true;
			break;
		} 
	}

	for(int k=0; k< size; k++){
		vertex* v = &(vertexList.at(k));
		if(!v23repete && sameVertex(*v23, *v)){
			v23 = v;
			indexv23 = k;
			v23repete = true;
			break;
		} 
	}

	for(int k=0; k< size; k++){
		vertex* v = &(vertexList.at(k));
		if(!v31repete && sameVertex(*v31, *v)){
			v31 = v;
			indexv31 = k;
			v31repete = true;
			break;
		}
	}

	if(!v12repete) {
		vertexList.push_back(*v12);		//index = begin_index	
		indexv12 = vertexList.size()-1;
	}

	if(!v23repete) {
		vertexList.push_back(*v23);		//index = begin_index+1
		indexv23 = vertexList.size()-1;
	}

	if(!v31repete) {
		vertexList.push_back(*v31);		//index = begine_index+2
		indexv31 = vertexList.size()-1;
	}


	//store the old index
	printf("old1 ");
	int old1 = faceList.at(meshId).ind1;
	printf("indexv12 ");
	int old2 = faceList.at(meshId).ind2;
	printf("indexv31 ");
	int old3 = faceList.at(meshId).ind3;

	//add the first triangle (replace the old one)
	//v1, v12, v31
	faceList.at(meshId).ind1 = old1;
	faceList.at(meshId).ind2 = indexv12;
	faceList.at(meshId).ind3 = indexv31;
	faceList.at(meshId).colorId = 5;

	vertexList.at(old1).printv();
	vertexList.at(indexv12).printv();
	vertexList.at(indexv31).printv();

	//find normal
	vertex* v = getNormal(vertexList.at(old1), vertexList.at(indexv12), vertexList.at(indexv31));	
	faceList.at(meshId).normalX = v->x;
	faceList.at(meshId).normalY = v->y;
	faceList.at(meshId).normalZ = v->z;

	//the rest 
	for(int i=0; i< 3; i++){	
		mesh *m = new mesh(0, 0, 0);
		vertex* v;

		if(i == 0){
			m->ind1 = old2;			//v2
			m->ind2 = indexv23;		//v23
			m->ind3 = indexv12;		//v12

		v = getNormal(vertexList.at(old2), vertexList.at(indexv23), vertexList.at(indexv12));	
		

		}
		else if(i==1){
			m->ind1 = old3;			//v3
			m->ind2 = indexv31;		//v31
			m->ind3 = indexv23;		//v23

			v = getNormal(vertexList.at(old3), vertexList.at(indexv31), vertexList.at(indexv23));	
		}
		else{
			m->ind1 = indexv12;		//v12
			m->ind2 = indexv23;		//v23
			m->ind3 = indexv31;		//v31

			v = getNormal(vertexList.at(indexv12), vertexList.at(indexv23), vertexList.at(indexv31));	
		}

		m->normalX = v->x;
		m->normalY = v->y;
		m->normalZ = v->z;
		m->colorId = 5;

		faceList.push_back(*m);	
	}
}



void interpolate(int id, float transx, float transy, float transz, int rotx, int roty){
	//multiply by inverse matrix
	float radian = rotx*2*3.14159265/360;
	
	float relativeTransx = transx;
	float relativeTransy = transy;
	float relativeTransz = transz;

	//rotate around x axis
	//x, ycos0, zcos0
	relativeTransy = relativeTransy*cos(radian);
	relativeTransz = relativeTransz*cos(radian);

	//rotate around y axis 
	//xcos0, y, zcos0
	relativeTransx = relativeTransx*cos(radian);
	relativeTransz = relativeTransz*cos(radian);

	/*
	//move only the selected mesh
	for(int i=0; i<3; i++){

		int index = 0;
		if(i == 0){	
			index = faceList.at(id).ind1;
		}else if(i ==1) {
			index = faceList.at(id).ind2;
		}else{
			index = faceList.at(id).ind3;
		}

		float prevx = vertexList.at(index).x;
		float prevy = vertexList.at(index).y;
		float prevz = vertexList.at(index).z;


		vertexList.at(index).x = relativeTransx/120+ prevx;
		vertexList.at(index).y = relativeTransy/120+ prevy;
		vertexList.at(index).z = relativeTransz/120+ prevz;

		printf("id: %d, tranx: %f, transy: %f, tranz: %f \n", id, relativeTransx, relativeTransy, relativeTransz);
	}
	*/

	softselection(id, relativeTransx, relativeTransy, relativeTransz);

	bool once = false;
	for(int i=0; i< faceList.size(); i++){
		once = checkSize(i);
		if(once) break;	//only one large mesh would subdivide the whole model 
	}
}

bool checkSize(int i){

	float maxArea = 1.5;

	//consider every mesh
	
		mesh m= faceList.at(i);
		float *length = new float[3];

		for(int j=0; j< 3; j++){
			
			int id1, id2;

			if(j ==0) {
				id1 = m.ind1;
				id2 = m.ind2;
			}
			if(j ==1){
				id1 = m.ind2;
				id2 = m.ind3;
			}
			if(j ==2){
				id1 = m.ind3;
				id2 = m.ind1;
			}

			float x1 = vertexList.at(id1).x;
			float y1 = vertexList.at(id1).y;
			float z1 = vertexList.at(id1).z;
			
			float x2 = vertexList.at(id2).x;
			float y2 = vertexList.at(id2).y;
			float z2 = vertexList.at(id2).z;

			length[j] = sqrt(pow(x1-x2, 2)+ pow(y1-y2, 2)+ pow(z1-z2, 2)); 
		}
		//printf("id: %d, l1: %f, l2: %f, l3: %f\n", i, length[0], length[1], length[2]);
		
		//find the area of triangles
		float p = (length[0]+ length[1]+ length[2])/2;
		float area = sqrt(p*(p-length[0])*(p-length[1])*(p-length[2]));
		//printf("id: %d, area: %f\n", i,area);

		
		if(area > maxArea) {
			int nmesh = faceList.size();
			//for(int k=0; k< nmesh; k++){
			//	subDivide(k);	//sudivide this mesh
			//}
			return true;
		}
		return false;

}

//copy to ex-data
void copy_vmmodel(){

	int nv = vertexList.size();
	int nf = faceList.size();
	
	for(int i=0 ; i< nv; i++){
		vertex* v = new vertex(&(vertexList.at(i)));
		exvertexList.push_back(*v);
	}

	for(int i=0; i< nf; i++){
		mesh* m = new mesh(&(faceList.at(i)));
		exfaceList.push_back(*m);
	}
}

//restore ex-data to the data structure
void undo_vmmodel(){
	
	if(exvertexList.size() >0 && exfaceList.size() > 0 ){
	vertexList.clear();
	faceList.clear();

	int nv = exvertexList.size();
	int nf = exfaceList.size();

	for(int i=0 ; i< nv; i++){
		vertexList.push_back(exvertexList.at(i));
	}

	for(int i=0; i< nf; i++){
		faceList.push_back(exfaceList.at(i));
	}

	exvertexList.clear();
	exfaceList.clear();
	}
}

//----------------------Softselection zone------------------------------------------
void softselection(int id,float relativeTransx,float relativeTransy,float relativeTransz){

	//function
	//f(x,y) = Ae^ -((x-x0)^2/2sx^2  + (y-y0)^2/2sy^2)
	//A = amplitude
	//x0, y0 = center
	//sx, sy = x and y spreads

	float s = 0.4;

	float denom = 2*pow(s,2);

	//find the center 
	int ind1 = faceList.at(id).ind1;
	int ind2 = faceList.at(id).ind2;
	int ind3 = faceList.at(id).ind3;
	//center
	float x0 = (vertexList.at(ind1).x + vertexList.at(ind2).x + vertexList.at(ind3).x )/3;	
	float y0 = (vertexList.at(ind1).y + vertexList.at(ind2).y + vertexList.at(ind3).y )/3; 
	float z0 = (vertexList.at(ind1).z + vertexList.at(ind2).z + vertexList.at(ind3).z )/3; 

	float exp, coef= 1, e= 2.71828183;
	for(int i=0; i< vertexList.size(); i++){
		vertex v = vertexList.at(i);

		exp = pow(v.x-x0, 2)/denom + pow(v.y-y0, 2)/denom;
		coef = pow(e, -exp);

		printf("v: %d \t| coef = %f\n", i, coef);


		//translate the point
		vertexList.at(i).x = vertexList.at(i).x + relativeTransx/100*coef;	//x
		vertexList.at(i).y = vertexList.at(i).y + relativeTransy/100*coef;	//y
		vertexList.at(i).z = vertexList.at(i).z + relativeTransz/200*coef;
	}

}