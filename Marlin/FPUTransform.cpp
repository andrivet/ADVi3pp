#include "FPUTransform.h"

#if defined(UMFPUSUPPORT) && (UMFPUSUPPORT > -1)

#include "MatrixMath.h"

float MasterTransform[4][4]; // this is the transform that describes how to move from
							 // ideal coordinates to real world coords

// private functions
void loadMatrix(float X1, float Y1, float Z1, float Y2, float Z2, float X3, float Z3);
void transformDestination(float &X, float &Y, float &Z);

bool FPUEnabled; // this is a bypass switch so that with one command the FPU can be
				 // turned off

void loadMatrix(float X1, float Y1, float Z1, float Y2, float Z2, float X3, float Z3)
{
float Xdiff = X3 - X1;
	serialPrintFloat(Xdiff);
	SERIAL_ECHOLN("");
float Ydiff = Y2 - Y1;
	serialPrintFloat(Ydiff);
	SERIAL_ECHOLN("");
//clockwise
float ZdiffX = Z3 - Z1;
	serialPrintFloat(ZdiffX);
	SERIAL_ECHOLN("");
//anti clockwise
float ZdiffY = Z1 - Z2;
	serialPrintFloat(ZdiffY);
	SERIAL_ECHOLN("");


//modified to take advantage of small angle trig.
float Xtheta = ZdiffX / Xdiff;
//	serialPrintFloat(Xtheta);
//	SERIAL_ECHOLN("");
float Ytheta = ZdiffY / Ydiff;
//	serialPrintFloat(Ytheta);
//	SERIAL_ECHOLN("");
float cosxtheta = 1-(Xtheta*Xtheta)/2;
//	serialPrintFloat(cosxtheta);
//	SERIAL_ECHOLN("");
float sinxtheta = Xtheta;
//	serialPrintFloat(sinxtheta);
//	SERIAL_ECHOLN("");
float cosytheta = 1-(Ytheta*Ytheta)/2;
//	serialPrintFloat(cosytheta);
//	SERIAL_ECHOLN("");
float sinytheta = Ytheta;
//	serialPrintFloat(sinytheta);
//	SERIAL_ECHOLN("");


// Start by moving X and Y to 0 (seperating this out as we want to undo this later)
float Translate00Z[4][4] = {{1.0, 0.0, 0.0, -X1},
						    {0.0, 1.0, 0.0, -Y1}, 
						    {0.0, 0.0, 1.0, 0.0}, 
						    {0.0, 0.0, 0.0, 1.0}};

//now move Z to 0
float TranslateZ0[4][4] = {{1.0, 0.0, 0.0, 0.0},
						   {0.0, 1.0, 0.0, 0.0}, 
						   {0.0, 0.0, 1.0, -Z1}, 
						   {0.0, 0.0, 0.0, 1.0}};

float Transform0[4][4];
matrixMaths.MatrixMult((float*)Translate00Z, (float*)TranslateZ0, 4, 4, 4, (float*)Transform0);

//first rotate in Y using XZ 
//[cos(t), 0, -sin(t), 0]
//[0     , 1, 0      , 0]
//[sin{t}, 0,  cos(t), 0]
//[0     , 0, 0      , 1]

float Transform1[4][4] = {{cosxtheta, 0.0, sinxtheta, 0.0},
						  {      0.0, 1.0,        0.0, 0.0}, 
						  {-sinxtheta, 0.0,  cosxtheta, 0.0}, 
						  {      0.0, 0.0,        0.0, 1.0}};
//matrixMaths.MatrixPrint((float*)Transform1, 4, 4, "Transform1");
float rotMatrix1[4][4];
matrixMaths.MatrixMult((float*)Transform1, (float*)Transform0, 4, 4, 4, (float*)rotMatrix1);

//first rotate in X using YZ 
//[1,       0, 0     , 0]
//[0,  cos(t), sin(t), 0]
//[0, -sin{t}, cos(t), 0]
//[0,       0, 0     , 1]

float Transform2[4][4] = {{ 1.0,         0.0,         0.0, 0.0},
						  { 0.0, cosytheta, sinytheta, 0.0}, 
						  { 0.0,sinytheta, cosytheta, 0.0}, 
						  { 0.0,         0.0,         0.0, 1.0}};
//matrixMaths.MatrixPrint((float*)Transform2, 4, 4, "Transform2");
//float rotMatrix[4][4];
//matrixMaths.MatrixMult((float*)Transform1, (float*)Transform2, 4, 4, 4, (float*)rotMatrix);
float rotMatrix2[4][4];
matrixMaths.MatrixMult((float*)Transform2, (float*)rotMatrix1, 4, 4, 4, (float*)rotMatrix2);
//translate back to XY
//matrixMaths.MatrixInvert((float*)Translate00Z, 4);
Translate00Z[0][3] = X1;
Translate00Z[1][3] = Y1;
matrixMaths.MatrixMult((float*)rotMatrix2, (float*)Translate00Z, 4, 4, 4, (float*)MasterTransform);
//matrixMaths.MatrixPrint((float*)rotMatrix, 4, 4, "rotMatrix");

//now we have the object in a plane that is parallel to the x axes, but we do not have the z height correct.
//transform the x15 y15 position using our rotation matrix and use the result to determine where z0 is and
//add this translate function to the transform matrix.

//float zError[4][1]={{X1},{Y1},{Z1},{1.0}};
//float zErrNew[1][4];
//matrixMaths.MatrixMult((float*)rotMatrix, (float*)zError, 1, 4, 4, (float*)zErrNew);

//float zTranslate[4][4] = {{ 1.0, 0.0, 0.0, 0.0},
//						  { 0.0, 1.0, 0.0, 0.0}, 
//						  { 0.0, 0.0, 1.0, 0.0}, 
//						  { 15-zErrNew[0][0],15-zErrNew[1][0],-zErrNew[2][0], 1.0}};
//matrixMaths.MatrixMult((float*)rotMatrix, (float*)zTranslate, 4, 4, 4, (float*)MasterTransform);

// We now have a way to translate from real-world coordinates to idealised coortdinates, // but what we actually want is a way to transform from the idealised g-code coordinates
// to real world coordinates. 
// This is simply the inverse.
matrixMaths.MatrixInvert((float*)MasterTransform, 4);
matrixMaths.MatrixPrint((float*)MasterTransform, 4, 4, "MasterTransform");
}

void transformDestination(float &X, float &Y, float &Z)
{
float oldPoint[4][1]={{X}, {Y}, {Z}, {1.0}};
float newPoint[1][4]={{0.0,0.0,0.0,0.0}};
matrixMaths.MatrixMult((float*)MasterTransform, (float*)oldPoint, 4, 4, 1, (float*)newPoint);
X=newPoint[0][0];
Y=newPoint[0][1];
Z=newPoint[0][2];
}

void FPUTransform_init()
{
if (FPUEnabled == true)
  {
  // It is important to ensure that if the bed levelling routine has not been called the 
  // printer behaves as if the real world and idealised world are one and the same
  matrixMaths.MatrixIdentity((float*)MasterTransform,4,4);
  SERIAL_ECHO("transform configured to identity");
  }
else
  {
  SERIAL_ECHO("transform correction not enabled");
  }
}

void FPUEnable()
{
	FPUEnabled = true;
	FPUTransform_init();
}

void FPUReset()
{
	FPUTransform_init();
}

void FPUDisable()
{
	FPUEnabled = false;
}

void FPUTransform_determineBedOrientation()
{
float Z1;  
float Y2 = Y_MAX_LENGTH - 15;  
float Z2; 
float X3 = X_MAX_LENGTH - 15; 
float Z3;

//get Z for X15 Y15, X15 Y(Y_MAX_LENGTH - 15) and X(X_MAX_LENGTH - 15) Y15
Z1 = Probe_Bed(15,15);
Z2 = Probe_Bed(15,Y2);
Z3 = Probe_Bed(X3,15);
if(FPUEnabled)
	{
	loadMatrix(15, 15, Z1, Y2, Z2, X3, Z3);
	}
}

void FPUTransform_transformDestination()
{
float XPoint = destination[X_AXIS];          // float variable 
float YPoint = destination[Y_AXIS];          // float variable 
float ZPoint = destination[Z_AXIS];          // float variable 
if(FPUEnabled)
	{
	transformDestination(XPoint, YPoint, ZPoint);
	}    
modified_destination[X_AXIS] = XPoint;       // float variable 
modified_destination[Y_AXIS] = YPoint;       // float variable 
modified_destination[Z_AXIS] = ZPoint;       // float variable 
}	

#endif //UMFPUSUPPORT
