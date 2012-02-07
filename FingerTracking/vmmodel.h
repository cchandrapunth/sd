#include <deque>

void import_vm();
void export_vm();
vertex* getNormal(vertex v1, vertex v2, vertex v3);

void drawMesh(int i);
void subDivide();

int getFaceListSize();
int getVertexListSize();

vertex* normalizeV(vertex* norm);