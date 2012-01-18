


//action encapsulate all neccesry parameter
typedef struct model_st{
	int nPoint;
	int nPoly;
	float* point_st;
	int* poly_st;
}model_state;

typedef struct matrix_state{
	int rollX;
	int rollY;
}matrix_state;


//getter 
int getMatX();
int getMatY();
int restoreMatX();
int restoreMatY();

//storage
void addMatrix(int x, int y);
void pushMatrix();

void storeModelHist();
void undo_m();