/*

Functions/structures included in this file:
void freeMem()
void setRunParameters()
void readRunParameters()
void WaitFor (unsigned int secs)

*/

#include <GL/glut.h>
#include <GL/glu.h>
#include <GL/gl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <cuda.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <curand.h>
#include <curand_kernel.h>
#include <signal.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <time.h>
#include "settingsIO.cpp"
using namespace std;

//Globals to hold positions, velocities, and forces on both the GPU and CPU
float4 *Pos, *Vel;

//Globals to setup the kernals

//Globals to be set by the setRunParameters function
double UnitLength = -1.0;
double Diameter = 1.0;
double UnitMass = -1.0;
double MassSi = -1.0;
double MassFe = -1.0;
double MassOfBody1 = -1.0;
double MassOfBody2 = -1.0;
double UnitTime = -1.0;
double Gravity = -1.0;

int NSi = -1; 
int NSi1 = -1; 
int NSi2 = -1;
int NFe = -1; 
int NFe1 = -1; 
int NFe2 = -1;

//Globals to be set by the findEarthAndMoon function
int NumberOfEarthElements = -1;
int NumberOfMoonElements = -1;
int *EarthIndex;
int *MoonIndex;

//Globals for the run to be read in from the runSetup file
SettingsIO *settings;

int DrawChoice = 0;
int FrameCount = 0;

int NumFeBody1;
int NumSiBody1;
int NumFeBody2;
int NumSiBody2;

int Skip;

long long int SizeFile;
long int NumFrames;

FILE *PosAndVelFile;
string FolderNameTemp;

bool wait = false;
bool running = true;

//Globals for setting up the viewing window 
int XWindowSize = 2500;
int YWindowSize = 2500; 
double Near = 0.2;
double Far = 600.0;

double ViewBoxSize = 300.0;

GLdouble Left = -ViewBoxSize;
GLdouble Right = ViewBoxSize;
GLdouble Bottom = -ViewBoxSize;
GLdouble Top = ViewBoxSize;
GLdouble Front = ViewBoxSize;
GLdouble Back = -ViewBoxSize;

//Direction here your eye is located location

double EyeX = 40.0;
double EyeY = 40.0;
double EyeZ = 40.0;

//Where you are looking

double LookAtX = 0.0;
double LookAtY = 0.0;
double LookAtZ = 0.0;

//Up vector for viewing

double EyeUpX = 0.0;
double EyeUpY = 1.0;
double EyeUpZ = 0.0;

void freeMem()
{
	free(Pos);
	free(Vel);
}

void setRunParameters()
{
	double massBody1 = settings->getMassOfEarth()*settings->getFractionEarthMassOfBody1();
	double massBody2 = settings->getMassOfEarth()*settings->getFractionEarthMassOfBody2();
	if(settings->getFractionFeBody1() + settings->getFractionSiBody1() != 1.0) 
	{
		printf("\nTSU Error: body1 fraction don't add to 1\n");
		exit(0);
	}
	if(settings->getFractionFeBody2() + settings->getFractionSiBody2() != 1.0) 
	{
		printf("\nTSU Error: body2 fraction don't add to 1\n");
		exit(0);
	}
	double totalMassOfFeBody1 = settings->getFractionFeBody1()*massBody1;
	double totalMassOfSiBody1 = settings->getFractionSiBody1()*massBody1;
	double totalMassOfFeBody2 = settings->getFractionFeBody2()*massBody2;
	double totalMassOfSiBody2 = settings->getFractionSiBody2()*massBody2;
	double totalMassOfFe = totalMassOfFeBody1 + totalMassOfFeBody2;
	double totalMassOfSi = totalMassOfSiBody1 + totalMassOfSiBody2;
	double massFe;
	double massSi;
	double diameterOfElement;
	
	if(totalMassOfFe != 0.0) NFe = (double)settings->N*(settings->getDensitySi()/settings->getDensityFe())/(totalMassOfSi/totalMassOfFe + settings->getDensitySi()/settings->getDensityFe());
	else NFe = 0;
	NSi = settings->N - NFe;
	
	if(totalMassOfFe != 0.0) NFe1 = NFe*totalMassOfFeBody1/totalMassOfFe; 
	else NFe1 = 0;
	
	NFe2 = NFe - NFe1;
	
	if(totalMassOfSi != 0.0) NSi1 = NSi*totalMassOfSiBody1/totalMassOfSi; 
	else NSi1 = 0;
	
	NSi2 = NSi - NSi1;
	
	if(NFe != 0) massFe = totalMassOfFe/NFe;
	else massFe = 0.0;
	if(NSi != 0) massSi = totalMassOfSi/NSi;
	else massSi = 0.0;
	
	if(NSi != 0) diameterOfElement = pow((6.0*massSi)/(settings->getPi()*settings->getDensitySi()), (1.0/3.0));
	else diameterOfElement = pow((6.0*massFe)/(settings->getPi()*settings->getDensityFe()), (1.0/3.0));
	
	UnitLength = diameterOfElement;
	
	if(NSi != 0) UnitMass = massSi;
	else UnitMass = massFe;
	
	if(NSi != 0) UnitTime = sqrt((6.0*massSi*(double)NSi)/(settings->getUniversalGravity()*settings->getPi()*settings->getDensitySi()*totalMassOfSi));
	else if(NFe != 0) UnitTime = sqrt((6.0*massFe*(double)NFe)/(settings->getUniversalGravity()*settings->getPi()*settings->getDensityFe()*totalMassOfFe));
	else 
	{
		printf("TSU Error: No mass, function setRunParameters\n");
		exit(0);
	}
	
	//In this system this is what sets the length unit, the time unit, and the mass unit. 
	Diameter = 1.0;
	Gravity = 1.0;

	if(NSi != 0)
	{
		MassSi = 1.0;
		MassFe = settings->getDensityFe()/settings->getDensitySi();
	}
	else if(NFe != 0)
	{
		MassFe = 1.0;
	}
	else 
	{
		printf("TSU Error: No mass, function setRunParameters\n");
		exit(0);
	}
}

void readRunParameters(char* posName, char* statsName)
{
	settings = new SettingsIO(posName, statsName);
	
	Pos = (float4*)malloc(settings->N*sizeof(float4));
	Vel = (float4*)malloc(settings->N*sizeof(float4));
	
	fseek(settings->PosAndVelFile, 0L, SEEK_END);
	SizeFile = ftell(settings->PosAndVelFile);
	rewind(settings->PosAndVelFile);
	cout << "Size of file: " << SizeFile << endl;
	
	NumFrames = SizeFile/(2 * settings->N * sizeof(float4)) + 1;
	cout << "Number of frames: " << NumFrames << endl;
	
	setRunParameters();
}

void WaitFor (unsigned int secs) 
{
    int retTime = time(0) + secs;     // Get finishing time.
    while (time(0) < retTime);    // Loop until it arrives.
}
