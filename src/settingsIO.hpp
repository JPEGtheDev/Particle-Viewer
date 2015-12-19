#ifndef SETTINGSIO_H 
#define SETTINGSIO_H 
#include <iostream>
#include <fstream>
#include <sstream>
#include "glm/glm.hpp"
#include "glad/glad.h"
#include "particle.hpp"
#include "tinyFileDialogs/tinyfiledialogs.h"
class SettingsIO 
{ 
public:
	//FILE *PosAndVelFile;
	std::string posName;
	std::string statsName;
	bool isPlaying;
	int errorCount;
	long N;
	long frames;
	SettingsIO();
	SettingsIO(std::string,std::string);
	~SettingsIO();
	void readPosVelFile(long, Particle*,bool);
	void seekReadPosVelFile(int, Particle*,bool);
	void togglePlay();
	glm::vec3 getInitialPosition1();
	glm::vec3 getInitialPosition2();
	glm::vec3 getInitialVelocity1();
	glm::vec3 getInitialVelocity2();
	glm::vec4 getInitialSpin1();
	glm::vec4 getInitialSpin2();
	double getFractionEarthMassOfBody1();
	double getFractionEarthMassOfBody2();
	double getFractionFeBody1();
	double getFractionSiBody1();
	double getFractionFeBody2();
	double getFractionSiBody2();
	float getDampRateBody1();
	float getDampRateBody2();
	float getEnergyTargetBody1();
	float getEnergyTargetBody2();
	float getTotalRunTime();
	float getDampTime();
	float getDampRestTime();
	float getEnergyAdjustmentTime();
	float getEnergyAdjustmentRestTime();
	float getSpinRestTime();
	float getDt();
	int getWriteToFile();
	int getRecordRate();
	SettingsIO* loadFile(Particle*, bool);
	double getDensityFe();
	double getDensitySi();
	double getKFe();
	double getKSi();
	double getKRFe();
	double getKRSi();
	double getSDFe();
	double getSDSi();
	int getDrawRate();
	int getDrawQuality();
	int getUseMultipleGPU();
	double getUniversalGravity();
	double getMassOfEarth();
	double getPi();
	long long int getFrames();
private: 
	//Note: most of these aren't used, but kept for posterity's sake and/or if the actual slam programs want to use this to simplify reading in stuff
	int RecordRate;
	int DrawRate;
	int DrawQuality;
	int WriteToFile;
	int UseMultipleGPU;	
	float DampRateBody1;
	float DampRateBody2;
	float EnergyTargetBody1;
	float EnergyTargetBody2;
	float TotalRunTime;
	float DampTime;
	float DampRestTime;
	float EnergyAdjustmentTime;
	float EnergyAdjustmentRestTime;
	float SpinRestTime;
	float SetupTime;
	float Dt;
	double FractionEarthMassOfBody1;	
	double FractionEarthMassOfBody2;	
	double FractionFeBody1;			
	double FractionSiBody1;			
	double FractionFeBody2;			
	double FractionSiBody2;	
	double DensityFe;			
	double DensitySi;			
	double KFe;
	double KSi;
	double KRFe;
	double KRSi;
	double SDFe;
	double SDSi;
	double UniversalGravity;	
	double MassOfEarth;
	double Pi;
	glm::vec3 InitialPosition1;
	glm::vec3 InitialPosition2;
	glm::vec3 InitialVelocity1;
	glm::vec3 InitialVelocity2;
	glm::vec4 InitialSpin1;
	glm::vec4 InitialSpin2;
	std::string posFile;
	std::string statsFile;
};

#endif /* SETTINGSIO_H */ 
