
//OpenGL setting for the scene 

typedef struct cameras {
	float pos[3];
	float lookAt[3];
	float lookUp[3];
}camera;

void init(camera *cam); 

//callback to draw window 
//void draw(bool isgrab, int id, float transX, float transY); 
//void drawPickingMode(bool isgrab, int id, float transX, float transY);

//callback when picking occurs
void picked(GLuint name, int sw);

//keyboard callback 
void processKeyboard(unsigned char key, int x, int y);

//callback for quiting
void quit();
