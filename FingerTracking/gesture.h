
//OpenNI and NITE setting for the scene

using namespace xn; 

void debugGesture();

//-----------------------------------------
//			error checking
//-----------------------------------------
#define CHECK_RC(rc, what)										\
if(rc != XN_STATUS_OK){											\
	printf("%s failed: %s\n", what, xnGetStatusString(rc));		\
	exit(1);													\
}			

//register the generator 
xn::DepthGenerator* getDepthGenerator(Context);
xn::HandsGenerator* getHandGenerator(Context);
xn::GestureGenerator* getGestureGenerator(Context);


//callback to initialize gesture
void XN_CALLBACK_TYPE Gesture_Recognized(GestureGenerator& generator, const XnChar* strGesture, 
	const XnPoint3D* pIDPosition, const XnPoint3D* pEndPosition,void* pCookie);

void XN_CALLBACK_TYPE Gesture_Process(GestureGenerator& generator, const XnChar* strGesture, 
	const XnPoint3D* pPosition, XnFloat fProgress,void* pCookie);

//callback to initialize hand
void XN_CALLBACK_TYPE Hand_Create(HandsGenerator& generator,XnUserID nId,const XnPoint3D* pPosition,
		XnFloat fTime, void* pCookie);

void XN_CALLBACK_TYPE Hand_Update(HandsGenerator& generator,XnUserID nId, const XnPoint3D* pPosition, 
	XnFloat fTime, void* pCookie);

void XN_CALLBACK_TYPE Hand_Destroy(HandsGenerator& generator,XnUserID nId, XnFloat fTime,void* pCookie);


//read hand area 
void drawHand(XnPoint3D* handPointList);

void getEdge(XnPoint3D* List, int nNumberOfPoints);

//helper
bool isGrab();
XnPoint3D getPalm();
void switchHandMode();
void estimateGrab(XnPoint3D* list, int n, XnPoint3D* highest, XnPoint3D* lowest, XnPoint3D* leftmost, XnPoint3D* rightmost);