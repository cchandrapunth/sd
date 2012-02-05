


//callback when picking occurs
void picked(GLuint name, int sw);
void processPick(float cursorX, float cursorY);

static int pickMe = -1;	

int getSelection();
void setNullSelection();	//show no mesh response when hand release