#include "stdafx.h"
#include <stdlib.h>
#include <gl/glut.h>
#include <deque>
#include <fstream>
#include <iostream>
#include <math.h>


#include "model.h"
#include "undo.h"
#include "softSelection.h"
#include "miniball.h"
#include "picking.h"

#define check 1
#define red 2
#define blue 3
#define green 4

using std::ifstream;
using std::ofstream;

//back up 
float *pointt; //change number of points need to reflect in the data structure
int *poly;
int nPoly, nPoint;

//translate scene
float rollX = 0; 
float rollY = 0;
float zoomZ;

static GLuint texName[3];
static bool FLAG_LINE; //draw contour

int getnPoint(){ return nPoint;}
int getnPoly(){ return nPoly;}
float* getPoint(){ return pointt;}
int* getPoly(){return poly;}
ofstream myfile;

//open the file and load data into the array
void ImportModel(){
	
	myfile.open ("zoom.txt");

	ifstream indata; 
	float num;

	indata.open("modelinput.txt");
	if(!indata) {
		printf("Error: inpu file couldn't be opened\n");
		exit(1);
	}

	indata >> nPoint;
	pointt = (float*) malloc(sizeof(float)*3*nPoint);
	indata >> nPoly;
	poly = (int*) malloc(sizeof(int)*4*nPoly);
	indata >> num;
	
	for(int i=0; i< nPoint; i++){
		for(int j=0; j<3; j++){
			std::cout << num << " ";
			pointt[i*3+j] = num;
			indata >> num;
		}
		std::cout << std::endl;
	}

	//read quad (fix to triangle later)
	for(int i=0; i< nPoly; i++){
		for(int j=0; j<3; j++){
			std::cout << num << " ";
			poly[i*3+j] = num;
			indata >> num;
		}
		std::cout <<std::endl;
	}

	indata.close();

}

void ExportModel(){
	ofstream outdata; 
	float num;

	outdata.open("modeloutput.txt");
	if(!outdata) {
		printf("Error: output file couldn't be opened\n");
		exit(1);
	}

	outdata << nPoint << " ";
	outdata << nPoly << std::endl;
	
	for(int i=0; i< nPoint; i++){
		for(int j=0; j<3; j++){
			num = pointt[i*3+j];
			outdata << num << "\t";
		}
		outdata << std::endl;
	}
	outdata << std::endl;

	//read quad (fix to triangle later)
	for(int i=0; i< nPoly; i++){
		for(int j=0; j<3; j++){
			num = poly[i*3+j];
			outdata << num << "\t";
		}
		outdata <<std::endl;
	}

	outdata.close();

}

//load manually for now
void LoadModel(point_t* point, model_t* model){

	model->nPolygons = nPoly;
	point->nVertexs = nPoint;

	 //add all points to the list
	for(int i=0; i<nPoint; i++){
		 // allocate enough memory for this model
		 vertex_t* v = (vertex_t *)malloc(sizeof(point_t));
		
		 v->X = pointt[(3*i)];
		 v->Y = pointt[(3*i)+1];
		 v->Z = pointt[(3*i)+2];

		 point->pPoints[i] = (*v);

		 printf("point%d: %f,%f,%f\n", i, v->X, v->Y, v->Z);
	 } 

	 // add all polygons to the list
	 model->pList = (polygon_t *)malloc(nPoly * sizeof(polygon_t));
	 polygon_t* ptr = model->pList; 

	 for(int j=0; j<nPoly; j++){
		
		ptr[j].p[0] = poly[j*3];
		ptr[j].p[1] = poly[(j*3)+1];
		ptr[j].p[2] = poly[(j*3)+2];

		printf("poly: %d,%d,%d\n", poly[j*3], poly[j*3+1], poly[j*3+2]);
	 }

	calculateNormal(&samplePoint, &sampleModel);

	//set color
	polygon_t* mesh = model->pList;
	for(int i=0; i< nPoly; i++){
		mesh[i].colorID = 1;
	}
}

void calculateNormal(point_t* point, model_t* model){

	 polygon_t* ptr = model->pList; 

	 //calculate normal

	 for(int k=0; k<nPoly; k++){
	 
		 int index0 = ptr[k].p[0];
		 int index1 = ptr[k].p[1];
		 int index2 = ptr[k].p[2];

		 vertex_t *norm = findnormal(point->pPoints[index0], point->pPoints[index1], point->pPoints[index2]);
		 normalize(norm);

		 ptr[k].normal = (*norm);

		 //printf("norm %d: %f, %f, %f\n", k, norm->X, norm->Y, norm->Z);
	 }

}

//white
void setColor(model_t *poly, int polyId, int cid){
	poly->pList[polyId].colorID = cid;
}

void loadColor(polygon_t poly){
	if(poly.colorID == 0) 
		glBindTexture(GL_TEXTURE_2D, 1);
	else if(poly.colorID == 1) 
		glBindTexture(GL_TEXTURE_2D, 2);
	else if(poly.colorID == 2)
		glBindTexture(GL_TEXTURE_2D, 3);
	else if(poly.colorID == 3)
		glBindTexture(GL_TEXTURE_2D, 4);

}

void DrawPolygon(polygon_t p, point_t* poly){

	//draw mesh
	glBegin(GL_TRIANGLES);
	 glNormal3f(p.normal.X, p.normal.Y, p.normal.Z);
	 glTexCoord2f(0.0, 0.0);
	 glVertex3f(poly->pPoints[p.p[0]].X, poly->pPoints[p.p[0]].Y, poly->pPoints[p.p[0]].Z);
	 glTexCoord2f(0.0, 1.0);
	 glVertex3f(poly->pPoints[p.p[1]].X, poly->pPoints[p.p[1]].Y, poly->pPoints[p.p[1]].Z);
	 glTexCoord2f(1.0, 1.0);
	 glVertex3f(poly->pPoints[p.p[2]].X, poly->pPoints[p.p[2]].Y, poly->pPoints[p.p[2]].Z);
	glEnd();

		//Draw contour line
	if(FLAG_LINE){
		glEnable(GL_LINE_SMOOTH);
		glBindTexture(GL_TEXTURE_2D, 3);

		glBegin(GL_LINES);
		for(int i=0; i< 2; i++){
			vertex_t v1 = poly->pPoints[p.p[i]];
			vertex_t v2;
			if(i < 2){
				 v2 = poly->pPoints[p.p[i+1]];
			}
			else{
				v2 = poly->pPoints[p.p[0]];
			}
			//line
			glVertex3f(v1.X, v1.Y, v1.Z);
			glVertex3f(v2.X, v2.Y, v2.Z);
		}
		 
		glEnd();
	}

	

}

void handleRoll(){
	//store new roll input 
	int rotX, rotY;
	float zoom, rate = 500;

	rotX = restoreMatX()+getMatX()+ (int)rollX;
	rotY = restoreMatY()+getMatY()+ (int)rollY;
	zoom = restoreMatZ()+getMatZ()+zoomZ;

	vertex_t c = getCenterSphere();
	glTranslated(c.X, c.Y, c.Z);
	glRotated(-rotX, 0, 1, 0);	//rotate around y axis
	glTranslated(-c.X, -c.Y, -c.Z);

	glTranslated(c.X, c.Y, c.Z);
	glRotated(rotY, 1, 0, 0);	//rotate around x axis
	glTranslated(-c.X, -c.Y, -c.Z);
	
	//when hand lost -> #INF
	if(abs(zoom) >1){
		glTranslated(c.X, c.Y, c.Z);
		glScalef(1+(float)zoom/rate, 1+(float)zoom/rate, 1+(float)zoom/rate);
		glTranslated(-c.X, -c.Y, -c.Z);
		myfile << restoreMatZ() <<"\t" << getMatZ() << "\t" <<zoomZ << "\n";
	}

	addMatrix((int)rollX, (int)rollY, zoomZ);

	//reset
	rollX=0;
	rollY=0;
	zoomZ=0;
}
// DrawModel(); draws a model
void drawMe (model_t *model, point_t* vertexList)
{
    glLoadIdentity();
	glEnable(GL_TEXTURE_2D);
	polygon_t *ptr = model->pList;

	for(int j=0; j<nPoly; j++){
			glPushMatrix();
			glPushName(j);

			handleRoll();
			polygon_t p = ptr[j];

			if(getSelection() == j){
				glBindTexture(GL_TEXTURE_2D, 4);
			}
			else{
				loadColor(p);
			}
			DrawPolygon(p, vertexList);
			//subdivide(p, vertexList);

			glPopName();
			glPopMatrix();
	}
}

void drawPickMe(model_t *model, point_t* vertexList){

		glDisable(GL_DITHER); //disable blending color function
		glDisable(GL_LIGHT0);
		glDisable(GL_LIGHTING);
		glDisable(GL_TEXTURE_2D);

		polygon_t *ptr = model->pList;
		for(int j=0; j<(nPoly/4); j++){
			for (int i=0; i< 4; i++){
				glPushMatrix();

				handleRoll();
				
				int nMesh = j*4+i;
				if(nMesh < 255){
					glColor3ub(255,255, nMesh);	
				}
				else printf("TOO MANY MASH\n");

				DrawPolygon(ptr[(j*4)+i], vertexList);
				
				glPopMatrix();
			}
		}

		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
		glEnable(GL_DITHER);
		glEnable(GL_TEXTURE_2D);
}

void FreeModel (model_t *model)
{
    if (model->pList)
    {
        free(model->pList);
        model->pList = NULL;
    }
}

void translatePoly(model_t* model, int id, point_t* vertexlist,float transx, float transy){

	softSelect(model, id, vertexlist,transx, transy, pointt);
}

void translateScene(float transx, float transy, float z){
	//set variable for rotate/zoom
	rollX = transx;
	rollY = transy;
	zoomZ = z;
}

vertex_t* findnormal(vertex_t vv1, vertex_t vv2, vertex_t vv3){

	vertex_t v1, v2; 
	v1.X = vv3.X - vv1.X;
	v1.Y = vv3.Y - vv1.Y;
	v1.Z = vv3.Z - vv1.Z;

	v2.X = vv2.X - vv1.X;
	v2.Y = vv2.Y - vv1.Y;
	v2.Z = vv2.Z - vv1.Z;

	vertex_t* norm = (vertex_t *)malloc(sizeof(vertex_t));
	//find normal using cross product
	norm->X = (v1.Y * v2.Z) - (v1.Z * v2.Y);
	norm->Y = -((v2.Z * v1.X) - (v2.X * v1.Z));
	norm->Z = (v1.X * v2.Y) - (v1.Y * v2.X);	

	return norm;
}

void normalize(vertex_t *norm){
	 float CombinedSquares = (norm->X * norm->X) +(norm->Y * norm->Y) +(norm->Z * norm->Z);
		 float NormalisationFactor = sqrt(CombinedSquares);
		 norm->X = norm->X / NormalisationFactor;		
		 norm->Y = norm->Y / NormalisationFactor;
		 norm->Z = norm->Z / NormalisationFactor;
}

void drawtriangle(vertex_t v1, vertex_t v2, vertex_t v3){
	vertex_t *norm = findnormal(v1, v2, v3);
	normalize(norm);

	glBegin(GL_TRIANGLES);
	 glNormal3f(norm->X, norm->Y, norm->Z);
	 glVertex3f(v1.X, v1.Y, v1.Z);
	 glVertex3f(v2.X, v2.Y, v2.Z);
	 glVertex3f(v3.X, v3.Y, v3.Z);
	glEnd();
	 
}

void subdivide(polygon_t p, point_t* poly){

	vertex_t v1 = poly->pPoints[p.p[0]];
	vertex_t v2 = poly->pPoints[p.p[1]];
	vertex_t v3 = poly->pPoints[p.p[2]];

	vertex_t* v12 = (vertex_t *)malloc(sizeof(vertex_t));
	vertex_t* v23 = (vertex_t *)malloc(sizeof(vertex_t));
	vertex_t* v31 = (vertex_t *)malloc(sizeof(vertex_t));

	v12->X = (v1.X+v2.X)/2.0;
	v12->Y = (v1.Y+v2.Y)/2.0;
	v12->Z = (v1.Z+v2.Z)/2.0;

	v23->X = (v2.X+v3.X)/2.0;
	v23->Y = (v2.Y+v3.Y)/2.0;
	v23->Z = (v2.Z+v3.Z)/2.0;

	v31->X = (v3.X+v1.X)/2.0;
	v31->Y = (v3.Y+v1.Y)/2.0;
	v31->Z = (v3.Z+v1.Z)/2.0;

	normalize(v12);
	normalize(v23);
	normalize(v31);
	
	drawtriangle(v1, *v12, *v31);
	drawtriangle(v2, *v23, *v12);
	drawtriangle(v3, *v31, *v23);
	drawtriangle(*v12, *v23, *v31);
}

void enable_line(){
	FLAG_LINE = !FLAG_LINE;
}