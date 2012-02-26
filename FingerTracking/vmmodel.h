

void import_vm();
void export_vm();


void drawMesh(int i);
void subDivide();
void subDivideMesh(int i);

int getFaceListSize();
int getVertexListSize();

//Normal
void recalNormal();
vertex* getFaceNormal(vertex v1, vertex v2, vertex v3);
vertex* normalizeV(vertex* norm);
void calVertexNormal();

void interpolate(int id, float transx, float transy, float transz, int rotx, int roty);
void interpolate(int* list, float transx, float transy, float transz, int rotx, int roty);

bool checkSize(int i);
void setColorPaint(int id);

void paintMesh(int mid, int cid);

void copy_vmmodel();
void undo_vmmodel();

void softselection(int id,float tx,float ty,float tz, float nx, float ny, float nz);
void extrude(int id,float tx,float ty,float tz, float nx, float ny, float nz);


//miniball
void findBoundingSphere();
vertex getCenter();
float getDiam();

void setGizmo(int k);
void drawGizmo();