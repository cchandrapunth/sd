#include "stdafx.h"
#include <stdlib.h>
#include <gl/glut.h>
#include <deque>

#include "model.h"

float coor[] = //change number of points need to reflect in the data structure
	{200, 200, -50, 
	600, 200, -50, 
	600, 300, -100,
	200, 300, -100,
	0, 100, 0, 
	800, 100, 0, 
	800, 400, -150, 
	0, 400, -150,
	400, 200, -50,
	400, 100, 0, 
	400, 300, -100, 
	400, 400, -150};

int poly[] =  
	{11,10,2,6,
	7,3,10,11,
	8,9,5,1,
	0,4,9,8,
	2,1,5,6,
	7,4,0,3,
	10,8,1,2,
	3,0,8,10 };

//load manually for now
void LoadModel(point_t* point, model_t* model){

   	 model->nPolygons = 8;
	 point->nVertexs = 12;

	 //add 12 points to the list
	 for(int i=0; i<12; i++){
		 // allocate enough memory for this model
		 vertex_t* v = (vertex_t *)malloc(sizeof(point_t));
		
		 v->X = coor[(3*i)];
		 v->Y = coor[(3*i)+1];
		 v->Z = coor[(3*i)+2];

		 point->pPoints[i] = (*v);

		 printf("coor%d: %f,%f,%f\n", i, v->X, v->Y, v->Z);
	 } 

	 // add 8 polygons to the list
	 model->pList = (polygon_t *)malloc(8 * sizeof(polygon_t));
	 polygon_t* ptr = model->pList; 

	 for(int j=0; j<8; j++){
		
		ptr[j].p[0] = poly[j*4];
		ptr[j].p[1] = poly[(j*4)+1];
		ptr[j].p[2] = poly[(j*4)+2];
		ptr[j].p[3] = poly[(j*4)+3];

		printf("poly: %d,%d,%d,%d\n", poly[j*4], poly[j*4+1], poly[j*4+2], poly[j*4+3]);
	 }
}

void calculateNormal(point_t* point, model_t* model){

	 polygon_t* ptr = model->pList; 

	 //calculate normal

	 vertex_t* norm = (vertex_t *)malloc(sizeof(vertex_t));
	 for(int k=0; k<8; k++){
	 
		 //construct 2 vecter 
		 vertex_t v1, v2; 
		 int index0 = ptr[k].p[0];
		 int index1 = ptr[k].p[1];
		 int index2 = ptr[k].p[2];
		 v1.X = point->pPoints[index2].X - point->pPoints[index0].X;
		 v1.Y = point->pPoints[index2].Y - point->pPoints[index0].Y;
		 v1.Z = point->pPoints[index2].Z - point->pPoints[index0].Z;

		 v2.X = point->pPoints[index1].X - point->pPoints[index0].X;
		 v2.Y = point->pPoints[index1].Y - point->pPoints[index0].Y;
		 v2.Z = point->pPoints[index1].Z - point->pPoints[index0].Z;

		 //find normal using cross product
		 norm->X = (v1.Y * v2.Z) - (v1.Z * v2.Y);
		 norm->Y = -((v2.Z * v1.X) - (v2.X * v1.Z));
		 norm->Z = (v1.X * v2.Y) - (v1.Y * v2.X);	


		 float CombinedSquares = (norm->X * norm->X) +(norm->Y * norm->Y) +(norm->Z * norm->Z);
		 float NormalisationFactor = sqrt(CombinedSquares);
		 norm->X = norm->X / NormalisationFactor;		
		 norm->Y = norm->Y / NormalisationFactor;
		 norm->Z = norm->Z / NormalisationFactor;


		 ptr[k].normal = (*norm);

		 printf("norm %d: %f, %f, %f\n", k, norm->X, norm->Y, norm->Z);

	 }

}

void DrawPolygon(polygon_t p, point_t* poly){

	glBegin(GL_QUADS);
	 glNormal3f(p.normal.X, p.normal.Y, p.normal.Z);
	 glVertex3f(poly->pPoints[p.p[0]].X, poly->pPoints[p.p[0]].Y, poly->pPoints[p.p[0]].Z);
	 glVertex3f(poly->pPoints[p.p[1]].X, poly->pPoints[p.p[1]].Y, poly->pPoints[p.p[1]].Z);
	 glVertex3f(poly->pPoints[p.p[2]].X, poly->pPoints[p.p[2]].Y, poly->pPoints[p.p[2]].Z);
	 glVertex3f(poly->pPoints[p.p[3]].X, poly->pPoints[p.p[3]].Y, poly->pPoints[p.p[3]].Z);
	glEnd();	
}

// DrawModel(); draws a model
void drawMe (model_t *model, point_t* vertexList)
{
    glLoadIdentity();

	polygon_t *ptr = model->pList;
	for(int j=0; j<2; j++){
	    for (int i=0; i< 4; i++){
			glPushMatrix();
			glPushName(j*2+i);

			DrawPolygon(ptr[(j*4)+i], vertexList);

			glPopName();
			glPopMatrix();
		}
	}
}

void drawPickMe(model_t *model, point_t* vertexList){

		glDisable(GL_DITHER); //disable blending color function
		glDisable(GL_LIGHT0);
		glDisable(GL_LIGHTING);


		polygon_t *ptr = model->pList;
		for(int j=0; j<2; j++){
			for (int i=0; i< 4; i++){
				glPushMatrix();

				switch(j*4+i){
				case 0: glColor3ub(255, 0, 0); break;
				case 1: glColor3ub(0, 255, 0); break;
				case 2: glColor3ub(0, 0, 255); break; 
				case 3: glColor3ub(255, 255, 0); break;
				case 4: glColor3ub(0, 255, 255); break;
				case 5: glColor3ub(255, 0, 255); break;
				case 6: glColor3ub(130, 0, 130); break;
				case 7: glColor3ub(0, 130, 130); break;
				}

				DrawPolygon(ptr[(j*4)+i], vertexList);
	
				glPopMatrix();
			}
		}

		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
		glEnable(GL_DITHER);
}

void FreeModel (model_t *model)
{
    if (model->pList)
    {
        free(model->pList);
        model->pList = NULL;
    }
}

void translatePoly(polygon_t p, point_t* vertexlist,float transx, float transy){

	//translate x and y of the points in the chosen poly
	for(int k=0; k<4; k++){
		int index = p.p[k];
		
		float prevx = vertexlist->pPoints[index].X;
		float prevy = vertexlist->pPoints[index].Y;

		vertexlist->pPoints[index].X = prevx+transx;
		vertexlist->pPoints[index].Y = prevy+transy;
	}
}