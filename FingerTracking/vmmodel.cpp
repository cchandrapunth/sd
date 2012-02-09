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

		m->setColor(6);	//default is white-6
		faceList.push_back(*m);

		//fill the lookup table for vertices 
		vertexList.at(id1).addFaceId(j);
		vertexList.at(id2).addFaceId(j);
		vertexList.at(id3).addFaceId(j);
	}

	if(debug){
		pLog->Write("load model complete");
		for(int i=0; i< nvertex; i++){
			vertexList.at(i).printv();
			vertexList.at(i).printface();
		}

		for(int i=0; i< nmesh; i++){
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

	/*
	drawtriangle(v1, *v12, *v31);
	drawtriangle(v2, *v23, *v12);
	drawtriangle(v3, *v31, *v23);
	drawtriangle(*v12, *v23, *v31);
	*/

	//add 3 new vertices 
	int begin_index = vertexList.size();
	vertexList.push_back(*v12);		//index = begin_index
	vertexList.push_back(*v23);		//index = begin_index+1
	vertexList.push_back(*v31);		//index = begine_index+2

	//store the old index
	int old1 = faceList.at(meshId).ind1;
	int old2 = faceList.at(meshId).ind2;
	int old3 = faceList.at(meshId).ind3;

	//add the first triangle (replace the old one)
	//v1, v12, v31
	faceList.at(meshId).ind1 = old1;
	faceList.at(meshId).ind2 = begin_index;
	faceList.at(meshId).ind3 = begin_index+2;

	//the rest 
	for(int i=0; i< 3; i++){	
		mesh *m = new mesh(0, 0, 0);
		if(i == 0){
			m->ind1 = old2;	//v2
			m->ind2 = begin_index+1;	//v23
			m->ind3 = begin_index;		//v12
		}
		else if(i==1){
			m->ind1 = old3;	//v3
			m->ind2 = begin_index+2;	//v31
			m->ind3 = begin_index+1;	//v23
		}
		else{
			m->ind1 = begin_index;		//v12
			m->ind2 = begin_index+1;	//v23
			m->ind3 = begin_index+2;	//v31
		}
		m->colorId = 0;

		faceList.push_back(*m);	
	}
}

void interpolate(int id, float transx, float transy, float transz, int rotx, int roty){
	//multiply by inverse matrix
	float radian = rotx*2*3.14159265/360;
	
	float relativeTransx = transx*cos(radian);
	float relativeTransy = transy*cos(radian);
	float relativeTransz = transz*cos(radian);

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


		vertexList.at(index).x = relativeTransx/100+ prevx;
		vertexList.at(index).y = relativeTransy/100+ prevy;
		vertexList.at(index).z = relativeTransz/100+ prevz;

		printf("id: %d, tranx: %f, transy: %f, tranz: %f \n", id, relativeTransx, relativeTransy, relativeTransz);
	}

	for(int i=0; i< faceList.size(); i++){
		bool boo = checkSize(i);
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
		printf("id: %d, area: %f\n", i,area);

		
		if(area > maxArea) {
			m.colorId = 0;
			//subdivide(model, &(model->pList[i]), vlist, i);	//sudivide this mesh
			return true;
		}
		return false;

}