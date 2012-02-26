#include "stdafx.h"
#include <stdlib.h>
#include <gl/glut.h>
#include <fstream>
#include "mesh.h"
#include "vertex.h"
#include <deque>
#include "vmmodel.h"
#include "log.h"

using namespace std;

static deque<mesh> faceList;
static deque<vertex> vertexList;

static deque<mesh> exfaceList;
static deque<vertex> exvertexList;

int selectedMesh;

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
		vertex* v = getFaceNormal(vertexList.at(id1), vertexList.at(id2), vertexList.at(id3));	
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
	
	//subDivide
	for(int k=0; k< 3; k++){
		subDivide();
	}

	calVertexNormal();

	if(debug){
		pLog->Write("load model complete");
		printf("\n=================================================\n");
		printf("\tvertexList :: nVertex = %d\n", vertexList.size());
		printf("=================================================\n");
		//for(int i=0; i< vertexList.size(); i++){
		//	vertexList.at(i).printv();
		//	vertexList.at(i).printface();
		//}

		printf("\n=================================================\n");
		printf("\tfaceList :: nFace = %d\n", faceList.size());
		printf("=================================================\n");
		//for(int i=0; i< faceList.size(); i++){
		//	faceList.at(i).printmesh();
		//}
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

//recalculate nornal in all 
void recalNormal(){
	for(int i=0; i< faceList.size(); i++){
		int ind1 = faceList.at(i).ind1;
		int ind2 = faceList.at(i).ind2;
		int ind3 = faceList.at(i).ind3;

		vertex *v = getFaceNormal(vertexList.at(ind1), vertexList.at(ind2), vertexList.at(ind3));
		faceList.at(i).normalX = v->x;
		faceList.at(i).normalY = v->y;
		faceList.at(i).normalZ = v->z;
	}

	calVertexNormal();
}

//find normal of 3 vertices
//output pointer to norm vector
vertex* getFaceNormal(vertex vv1, vertex vv2, vertex vv3){

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
	norm->y = (v1->z * v2->x) - (v1->x* v2->z);
	norm->z = (v1->x * v2->y) - (v1->y * v2->x);	

	return norm;
}

void calVertexNormal(){
	for(int i=0; i< vertexList.size(); i++){
		int numFace = vertexList.at(i).nface;

		float nx =0, ny =0, nz=0;
		for(int j=0; j< numFace; j++){
			int n = vertexList.at(i).faceId[j];

			nx += faceList.at(n).normalX;
			ny += faceList.at(n).normalY;
			nz += faceList.at(n).normalZ;
		}
		vertex *v = new vertex(nx, ny, nz);
		v = normalizeV(v);
		vertexList.at(i).vnormx = v->x;
		vertexList.at(i).vnormy = v->y;
		vertexList.at(i).vnormz = v->z;
	}
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

void drawMesh(int meshId, bool shade){

	mesh m = faceList.at(meshId);

	vertex v1 = vertexList.at(m.ind1);
	vertex v2 = vertexList.at(m.ind2);
	vertex v3 = vertexList.at(m.ind3);

	glShadeModel(GL_SMOOTH);
	
	glBegin(GL_TRIANGLES);
	 //glNormal3f(m.normalX, m.normalY, m.normalZ);
	if(shade){
		setEffectColor(v1);
	}
	glNormal3f(v1.vnormx, v1.vnormy, v1.vnormz);
	 glVertex3f(v1.x, v1.y, v1.z);

	if(shade){
		setEffectColor(v2);
	 }
	glNormal3f(v2.vnormx, v2.vnormy, v2.vnormz);
	 glVertex3f(v2.x, v2.y, v2.z);

	if(shade){
		setEffectColor(v3);
	}
	glNormal3f(v3.vnormx, v3.vnormy, v3.vnormz);
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

void subDivide(){

	//subdivide: always assign faceId afterward
	int size = getFaceListSize();
	for(int i=0; i< size; i++){
		subDivideMesh(i);
	}
	//assign faceId
	for(int i=0; i< vertexList.size(); i++){
		vertexList.at(i).clearFaceId();

		for(int j=0; j< faceList.size(); j++){
			if(faceList.at(j).ind1 == i || 
				faceList.at(j).ind2 == i ||
				faceList.at(j).ind3 == i){
					vertexList.at(i).addFaceId(j);
			}
		}
	}
}

//divide a mesh into 4 new mesh
//handle the operation and restoration in the deque
void subDivideMesh(int meshId){

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
	//printf("old1 ");
	int old1 = faceList.at(meshId).ind1;
	//printf("indexv12 ");
	int old2 = faceList.at(meshId).ind2;
	//printf("indexv31 ");
	int old3 = faceList.at(meshId).ind3;

	//add the first triangle (replace the old one)
	//v1, v12, v31
	faceList.at(meshId).ind1 = old1;
	faceList.at(meshId).ind2 = indexv12;
	faceList.at(meshId).ind3 = indexv31;
	faceList.at(meshId).colorId = 5;

	//vertexList.at(old1).printv();
	//vertexList.at(indexv12).printv();
	//vertexList.at(indexv31).printv();

	//find normal
	vertex* v = getFaceNormal(vertexList.at(old1), vertexList.at(indexv12), vertexList.at(indexv31));	
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

		v = getFaceNormal(vertexList.at(old2), vertexList.at(indexv23), vertexList.at(indexv12));	
		

		}
		else if(i==1){
			m->ind1 = old3;			//v3
			m->ind2 = indexv31;		//v31
			m->ind3 = indexv23;		//v23

			v = getFaceNormal(vertexList.at(old3), vertexList.at(indexv31), vertexList.at(indexv23));	
		}
		else{
			m->ind1 = indexv12;		//v12
			m->ind2 = indexv23;		//v23
			m->ind3 = indexv31;		//v31

			v = getFaceNormal(vertexList.at(indexv12), vertexList.at(indexv23), vertexList.at(indexv31));	
		}

		m->normalX = v->x;
		m->normalY = v->y;
		m->normalZ = v->z;
		m->colorId = 5;

		faceList.push_back(*m);	
	}
}


//enforce the traslation in normal direction
void interpolate(int id, float transx, float transy, float transz, int rotx, int roty){

	//multiply by inverse matrix
	float radianx = rotx*2*3.14159265/360;
	float radiany = roty*2*3.14159265/360;
	
	

	float vectorx = transx;
	float vectory = transy;
	float vectorz = -transz;

	//rotate around y axis
	vectorx =  transx*cos(radianx) - vectorz*sin(radianx);	//x' = xcos0- zsin0
	vectorz =  -transx*sin(radianx) + transz*cos(radianx);	//z' = xsin0+ zcos0

	
	
	//Note:: mistake 
	//make sure the z value changes and it's updated in the second rotation
	//rotate around x axis
	vectory = transy*cos(radiany) - vectorz*sin(radiany);	// y' = ycos0 - zsin0
	vectorz = transy*sin(radiany) + vectorz*cos(radiany);	// z' = ysin0 + zcos0
	

	//smart angle?
	if (abs(vectorx) > abs(vectory) && abs(vectorx) > abs(vectorz)) {
		vectory = 0;
		vectorz = 0;
	}
	else if(abs(vectory) > abs(vectorx) && abs(vectory) > abs(vectorz)){
		vectorx = 0;
		vectorz = 0;
	}
	else {
		vectorx = 0;
		vectory = 0;
	}
	printf("x= %f, y=%f, z=%f\n", vectorx, vectory, vectorz);

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


		vertexList.at(index).x = vectorx/120+ prevx;
		vertexList.at(index).y = vectory/120+ prevy;
		vertexList.at(index).z = vectorz/120+ prevz;

		//printf("id: %d, tranx: %f, transy: %f, tranz: %f \n", id, transx, transy, transz);
	}
	*/

	float normalx = faceList.at(id).normalX;
	float normaly = faceList.at(id).normalY;
	float normalz = faceList.at(id).normalZ;

	softselection(id, vectorx/100, vectory/100, vectorz/100, normalx, normaly, normalz);

	bool once = false;
	for(int i=0; i< faceList.size(); i++){
		once = checkSize(i);
		if(once) break;	//only one large mesh would subdivide the whole model 
	}
}

void interpolate(int* list, float transx, float transy, float transz, int rotx, int roty){
	printf("GROUP\n");
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
float s = 0.5;
float denom = 2*pow(s,2);
float e= 2.71828183;

void softselection(int id,float tx,float ty,float tz, float nx, float ny, float nz){

	//function
	//f(x,y) = Ae^ -((x-x0)^2/2sx^2  + (y-y0)^2/2sy^2)
	//A = amplitude
	//x0, y0 = center
	//sx, sy = x and y spreads

	float *f = getCenterSelection();

	//center
	float x0 = f[0];	
	float y0 = f[1];
	float z0 = f[2];

	float exp, coef= 1;
	for(int i=0; i< vertexList.size(); i++){
		vertex v = vertexList.at(i);

		exp = (pow(v.x-x0, 2) + pow(v.y-y0, 2) + pow(v.z-z0, 2))/denom;
		coef = pow(e, -exp);

		//printf("v: %d \t| coef = %f\n", i, coef);


		//translate the point
		vertexList.at(i).x = vertexList.at(i).x + tx*coef;	//x
		vertexList.at(i).y = vertexList.at(i).y + ty*coef;	//y
		vertexList.at(i).z = vertexList.at(i).z + tz*coef;	//z
	}
}

void setEffectColor(vertex v){
	//calculate distance 
	float *f = getCenterSelection();

	//center
	float x0 = f[0];	
	float y0 = f[1];
	float z0 = f[2];

	float exp = (pow(v.x-x0, 2) + pow(v.y-y0, 2) + pow(v.z-z0, 2))/denom;
	float coef = pow(e, -exp);

	//shade effect 
	//red->orange->yellow->green->blue->black
	//1------>0

	float r =1, g=1, b=1; 
	if(coef > 0.5 && coef <1){
		r = (coef-0.5)*2;
	}
	if(coef <0.8 && coef >0.3){
		g = (coef-0.3)*2;
	}
	if(coef >0 && coef < 0.5){
		b = (coef*2);
	}
	
	glColor3f(r, g, b);
}
/*------------------------Extrude---------------------------------------
void extrude(int id,float tx,float ty,float tz, float nx, float ny, float nz){
	//find the center 
	int ind1 = faceList.at(id).ind1;
	int ind2 = faceList.at(id).ind2;
	int ind3 = faceList.at(id).ind3;
	//3 vertex that form base of the extrusion
	vertex v1 = vertexList.at(ind1);
	vertex v2 = vertexList.at(ind2);
	vertex v3 = vertexList.at(ind3);


	//3 vertex that will be the top 
	vertex v11 = new vertex(v1.x+ nx*ty, v1.y+ ny*ty, v1.z +nz*ty);
	vertex v22 = new vertex(v2.x+ nx*ty, v2.y+ ny*ty, v2.z +nz*ty);
	vertex v33 = new vertex(v3.x+ nx*ty, v3.y+ ny*ty, v3.z +nz*ty);

	//contruct rectangle 
	glBegin(GL_QUADS);
	//1 2 22 11
	glVertex3f(v1.x, v1.y, v1.z);
	glVertex3f(v2.x, v2.y, v2.z);
	glVertex3f(v22.x, v22.y, v22.z);
	glVertex3f(v11.x, v11.y, v11.z);

	//2 3 33 22
	glVertex3f(v2.x, v2.y, v2.z);
	glVertex3f(v3.x, v3.y, v3.z);
	glVertex3f(v33.x, v33.y, v33.z);
	glVertex3f(v22.x, v22.y, v22.z);


	//3 1 11 33
	glVertex3f(v3.x, v3.y, v3.z);
	glVertex3f(v1.x, v1.y, v1.z);
	glVertex3f(v11.x, v11.y, v11.z);
	glVertex3f(v33.x, v33.y, v33.z);
	glEnd();

}
*/

//------------------------Bounding sphere---------------------------------
#define MIN_F -1000000;
#define MAX_F 1000000;

vertex *center;
float radius =0;

void findBoundingSphere(){

	//find the bound
	float top = MIN_F; 
	float right = MIN_F;
	float front= MIN_F;
	float bottom = MAX_F;
	float left = MAX_F;
	float back = MAX_F;

	int numVertex = getVertexListSize();

	for(int i=0; i< numVertex; i++){
		float x = vertexList.at(i).x;
		float y = vertexList.at(i).y; 
		float z = vertexList.at(i).z;

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

	for(int i =0; i < numVertex; i++){
		float x = vertexList.at(i).x;
		float y = vertexList.at(i).y; 
		float z = vertexList.at(i).z;

		float dis = sqrt(pow(x-center->x, 2)+pow(y-center->y, 2)+pow(z-center->z, 2));
		if(dis >radius) radius = dis;
	}
	printf("radius: %f\n", radius);

	float result[4] = {center->x, center->y, center->z, radius}; 
} 

vertex getCenter(){
	return (*center);
}

float getDiam(){
	return radius*2;
}


//-------------------------gizmo/center-----------------------------------
void setGizmo(int k){
	selectedMesh = k;
}
float* getCenterSelection(){
	//find the center 
	int ind1 = faceList.at(selectedMesh).ind1;
	int ind2 = faceList.at(selectedMesh).ind2;
	int ind3 = faceList.at(selectedMesh).ind3;
	//center
	float *f = (float *) malloc(sizeof(float));
	f[0] = (vertexList.at(ind1).x + vertexList.at(ind2).x + vertexList.at(ind3).x )/3;	
	f[1] = (vertexList.at(ind1).y + vertexList.at(ind2).y + vertexList.at(ind3).y )/3; 
	f[2] = (vertexList.at(ind1).z + vertexList.at(ind2).z + vertexList.at(ind3).z )/3; 

	return f; 
	
}

