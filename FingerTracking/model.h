

typedef struct vertex_s
{
	float X,Y,Z;
} vertex_t;

//list of points' name 
typedef struct point_s
{
	int nVertexs; 
	vertex_t pPoints[12]; 
} point_t;

extern point_t samplePoint;

//contain 4 points' name that make the polygon
typedef struct polygon_s
{
	int p[4];
	vertex_t normal;	//surface normal
} polygon_t;

typedef struct model_s
{
	int nPolygons;
	polygon_t *pList;
	
} model_t;


extern model_t sampleModel;

//render polygon
void DraPolygon (vertex_t vert, polygon_t *p);

//model ops
void LoadModel(point_t* point, model_t* model);
void drawMe (model_t *model, point_t* poly);
void drawPickMe(model_t* model, point_t* poly);
void FreeModel (model_t* model);
void calculateNormal(point_t* point, model_t* model);

//translate
void translatePoly(polygon_t p, point_t* vertexlist,float transx, float transy);
