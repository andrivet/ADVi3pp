#include "FPUTransform.h"

#if defined(UMFPUSUPPORT) && (UMFPUSUPPORT > -1)

#include "MatrixMath.h"

float MasterTransform[4][4]; // this is the transform that describes how to move from
							 // ideal coordinates to real world coords

// private functions
void loadMatrix(float X4, float Y3, float Z1, float X2, float Y2, float Z2, float X3, float Z3, float Z4);
void transformDestination(float &X, float &Y, float &Z);

bool FPUEnabled; // this is a bypass switch so that with one command the FPU can be
				 // turned off

void loadMatrix(float X4, float Y1, float Z1, float X2, float Y2, float Z2, float X3, float Z3, float Z4)
{
float Xdiff = X4 - X3;
	serialPrintFloat(Xdiff);
	SERIAL_ECHOLN("");
float Ydiff = Y2 - Y1;
	serialPrintFloat(Ydiff);
	SERIAL_ECHOLN("");
//clockwise
float ZdiffX = Z4 - Z3;
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

//these transforms are to set the origin for each rotation
float TranslateX0[4][4] = {{1.0, 0.0, 0.0, -X3},
						    {0.0, 1.0, 0.0, -Y1}, 
						    {0.0, 0.0, 1.0, -Z3}, 
						    {0.0, 0.0, 0.0, 1.0}};

float TranslateY0[4][4] = {{1.0, 0.0, 0.0, -X2},
						   {0.0, 1.0, 0.0, -Y1}, 
						   {0.0, 0.0, 1.0, -Z1}, 
						   {0.0, 0.0, 0.0, 1.0}};

//rotate in Y using XZ 
float TransformY[4][4] = {{cosxtheta, 0.0, sinxtheta, 0.0},
						  {      0.0, 1.0,        0.0, 0.0}, 
						  {-sinxtheta, 0.0,  cosxtheta, 0.0}, 
						  {      0.0, 0.0,        0.0, 1.0}};
//rotate in X using YZ 
float TransformX[4][4] = {{ 1.0,         0.0,         0.0, 0.0},
						  { 0.0, cosytheta, sinytheta, 0.0}, 
						  { 0.0,sinytheta, cosytheta, 0.0}, 
						  { 0.0,         0.0,         0.0, 1.0}};


// first translate point1 to 0 then rotate in Y then translate back
float MatrixStage1[4][4];
float MatrixStage2[4][4];
//matrixMaths.MatrixMult((float*)TranslateY0, (float*)TransformX, 4, 4, 4, (float*)MatrixStage1);
//matrixMaths.MatrixPrint((float*)MatrixStage1, 4, 4, "MatrixStage1");
//TranslateY0[0][3] = -TranslateY0[0][3];
//TranslateY0[1][3] = -TranslateY0[1][3];
//TranslateY0[2][3] = -TranslateY0[2][3];
//matrixMaths.MatrixPrint((float*)TranslateY0, 4, 4, "TranslateY0");
//matrixMaths.MatrixMult((float*)MatrixStage1, (float*)TranslateY0, 4, 4, 4, (float*)MatrixStage2);
//matrixMaths.MatrixPrint((float*)MatrixStage2, 4, 4, "MatrixStage2");
//Now translate point3 to 0 and rotate in x before translating back
float MatrixStage3[4][4];
float MatrixStage4[4][4];
//matrixMaths.MatrixMult((float*)MatrixStage2, (float*)TranslateX0, 4, 4, 4, (float*)MatrixStage3);
//matrixMaths.MatrixPrint((float*)MatrixStage3, 4, 4, "MatrixStage3");
//matrixMaths.MatrixMult((float*)MatrixStage3, (float*)TransformY, 4, 4, 4, (float*)MatrixStage4);
matrixMaths.MatrixMult((float*)TransformX, (float*)TransformY, 4, 4, 4, (float*)MasterTransform);
matrixMaths.MatrixPrint((float*)MatrixStage4, 4, 4, "MatrixStage4");
//TranslateX0[0][3] = -TranslateX0[0][3];
//TranslateX0[1][3] = -TranslateX0[1][3];
//TranslateX0[2][3] = -TranslateX0[2][3];
//matrixMaths.MatrixPrint((float*)TranslateX0, 4, 4, "TranslateX0");
//matrixMaths.MatrixMult((float*)MatrixStage4, (float*)TranslateX0, 4, 4, 4, (float*)MasterTransform);
//matrixMaths.MatrixPrint((float*)MasterTransform, 4, 4, "MasterTransform (pre-invert)");

// We now have a way to translate from real-world coordinates to idealised coortdinates, 
// but what we actually want is a way to transform from the idealised g-code coordinates
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
int X3 = 15;
float X4 = X_MAX_LENGTH - 20; 
float X2 = (X4 + X3) / 2;
int Y1 = 15;
float Y2 = Y_MAX_LENGTH - 5;  
float Z1;
float Z2; 
float Z3;
float Z4;

//get Z for X15 Y15, X15 Y(Y_MAX_LENGTH - 15) and X(X_MAX_LENGTH - 15) Y15
Z3 = Probe_Bed(X3,Y1,PROBE_N);
Z4 = Probe_Bed(X4,Y1,PROBE_N);
Z1 = (Z3 + Z4) / 2;
Z2 = Probe_Bed(X2,Y2,PROBE_N);
if(FPUEnabled)
	{
	loadMatrix(X4, Y1, Z1, X2, Y2, Z2, X3, Z3, Z4);
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
