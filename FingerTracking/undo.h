


//action encapsulate all neccesry parameter
typedef struct model_st{
	int nPoint;
	int nPoly;
	float* point_st;
	int* poly_st;

}model_state;


void storeModelHist();
void undo_m();

