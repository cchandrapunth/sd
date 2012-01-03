
//OpenGL setting for the scene 



//callback to draw window 
//void draw(bool isgrab, int id, float transX, float transY); 
//void drawPickingMode(bool isgrab, int id, float transX, float transY);

//callback when picking occurs
void picked(GLuint name, int sw);

//keyboard callback 
void processKeyboard(unsigned char key, int x, int y);
void processPick(int cursorX, int cursorY);
int getSelection();
static int pickMe = 0;	