#include <deque>

void import_vm();
void export_vm();
vertex* getNormal(vertex v1, vertex v2, vertex v3);

void drawMesh(int i);
void subDivide(int i);

int getFaceListSize();
int getVertexListSize();

vertex* normalizeV(vertex* norm);
void interpolate(int id, float transx, float transy, float transz, int rotx, int roty);
bool checkSize(int i);
void setColorPaint(int id);

void paintMesh(int mid, int cid);

void copy_vmmodel();

void undo_vmmodel();


void softselection(int id,float relativeTransx,float relativeTransy,float relativeTransz);