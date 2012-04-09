


void mode_selection(XnPoint3D* handPointList, hand_h* rhand, hand_h* lhand);
void checkCursor(int func, hand_h* rhand, hand_h* lhand);

//screen resolution
void set_nRes(XnUInt16 XRes, XnUInt16 nYRes);
void set_cursor(float x, float y);

//helper
void rotate();
void switch_buffer();
bool get_buffer();