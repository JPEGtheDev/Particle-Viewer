#include "settingsIO.hpp"
using namespace std;

SettingsIO::SettingsIO() 
{
	//not really sure I need this tbh
	//but it looks nice to have
}

SettingsIO::SettingsIO(const char* posName, const char* statsName)
{
	this->PosAndVelFile = fopen(posName, "r");
	ifstream data;
	string name;
	string blank;
	data.open(statsName);
	if(data.is_open())
	{
		getline(data,name,'=');
		data >> this->InitialPosition1.x;
		getline(data,name,'=');
		data >> this->InitialPosition1.y;
		getline(data,name,'=');
		data >> this->InitialPosition1.z;
		getline(data,name,'=');
		data >> this->InitialPosition2.x;
		getline(data,name,'=');
		data >> this->InitialPosition2.y;
		getline(data,name,'=');
		data >> this->InitialPosition2.z;
		
		getline(data,name,'=');
		data >> this->InitialVelocity1.x;
		getline(data,name,'=');
		data >> this->InitialVelocity1.y;
		getline(data,name,'=');
		data >> this->InitialVelocity1.z;
		getline(data,name,'=');
		data >> this->InitialVelocity2.x;
		getline(data,name,'=');
		data >> this->InitialVelocity2.y;
		getline(data,name,'=');
		data >> this->InitialVelocity2.z;
		
		getline(data,name,'=');
		data >> this->InitialSpin1.x;
		getline(data,name,'=');
		data >> this->InitialSpin1.y;
		getline(data,name,'=');
		data >> this->InitialSpin1.z;
		getline(data,name,'=');
		data >> this->InitialSpin1.w;
		
		getline(data,name,'=');
		data >> this->InitialSpin2.x;
		getline(data,name,'=');
		data >> this->InitialSpin2.y;
		getline(data,name,'=');
		data >> this->InitialSpin2.z;
		getline(data,name,'=');
		data >> this->InitialSpin2.w;
		
		getline(data,name,'=');
		data >> this->FractionEarthMassOfBody1;
		getline(data,name,'=');
		data >> this->FractionEarthMassOfBody2;
		
		getline(data,name,'=');
		data >> this->FractionFeBody1;
		getline(data,name,'=');
		data >> this->FractionSiBody1;
		getline(data,name,'=');
		data >> this->FractionFeBody2;
		getline(data,name,'=');
		data >> this->FractionSiBody2;
		
		getline(data,name,'=');
		data >> this->DampRateBody1;
		getline(data,name,'=');
		data >> this->DampRateBody2;
		
		getline(data,name,'=');
		data >> this->EnergyTargetBody1;
		getline(data,name,'=');
		data >> this->EnergyTargetBody2;
		
		getline(data,name,'=');
		data >> N;
		
		getline(data,name,'=');
		data >> this->TotalRunTime;
		getline(data,name,'=');
		data >> this->DampTime;
		getline(data,name,'=');
		data >> this->DampRestTime;
		getline(data,name,'=');
		data >> this->EnergyAdjustmentTime;
		getline(data,name,'=');
		data >> this->EnergyAdjustmentRestTime;
		getline(data,name,'=');
		data >> this->SpinRestTime;
		
		getline(data,name,'=');
		data >> this->Dt;
		
		getline(data,name,'=');
		data >> this->WriteToFile;
		
		getline(data,name,'=');
		data >> this->RecordRate;
		
		getline(data,name,'=');
		data >> this->DensityFe;
		getline(data,name,'=');
		data >> this->DensitySi;
		
		getline(data,name,'=');
		data >> this->KFe;
		getline(data,name,'=');
		data >> this->KSi;
		getline(data,name,'=');
		data >> this->KRFe;
		getline(data,name,'=');
		data >> this->KRSi;
		getline(data,name,'=');
		data >> this->SDFe;
		getline(data,name,'=');
		data >> this->SDSi;
		
		getline(data,name,'=');
		data >> this->DrawRate;
		getline(data,name,'=');
		data >> this->DrawQuality;
		
		getline(data,name,'=');
		data >> this->UseMultipleGPU;
		
		getline(data,name,'=');
		data >> this->UniversalGravity;
		getline(data,name,'=');
		data >> this->MassOfEarth;
		getline(data,name,'=');
		data >> this->Pi;
	}
	data.close();
}

SettingsIO::~SettingsIO()
{
	fclose(PosAndVelFile);
}

glm::vec3 *SettingsIO::readPosVelFile()
{
	glm::vec4 *pos = new glm::vec4[N];
	glm::vec3 *conv = new glm::vec3[N];
	fread(pos, sizeof(glm::vec4), (int)N, PosAndVelFile);

	for(long i =0; i < N; i++)
	{
		conv[i] = glm::vec3(pos[i].x,pos[i].y,pos[i].z);
	}

	return conv;
}
GLuint *SettingsIO::readColor()
{

	glm::vec4 *pos = new glm::vec4[N];
	GLuint *conv = new GLuint[N];
	fread(pos, sizeof(glm::vec4), (int)N, PosAndVelFile);

	for(long i =0; i < N; i++)
	{
		conv[i] = (GLuint) pos[i].w;
	}

	return conv;
}

void SettingsIO::seekReadPosVelFile(int skip, glm::vec4 *pos, glm::vec4 *vel)
{
	fseek(PosAndVelFile, skip * sizeof(glm::vec4) * 2 * N, SEEK_CUR);
	fread(pos, sizeof(glm::vec4), N, PosAndVelFile);
	fread(vel, sizeof(glm::vec4), N, PosAndVelFile);
}

glm::vec3 SettingsIO::getInitialPosition1()
{
	return InitialPosition1;
}

glm::vec3 SettingsIO::getInitialPosition2()
{
	return InitialPosition2;
}

glm::vec3 SettingsIO::getInitialVelocity1()
{
	return InitialVelocity1;
}

glm::vec3 SettingsIO::getInitialVelocity2()
{
	return InitialVelocity2;
}

glm::vec4 SettingsIO::getInitialSpin1()
{
	return InitialSpin1;
}

glm::vec4 SettingsIO::getInitialSpin2()
{
	return InitialSpin2;
}

double SettingsIO::getFractionEarthMassOfBody1()
{
	return FractionEarthMassOfBody1;
}

double SettingsIO::getFractionEarthMassOfBody2()
{
	return FractionEarthMassOfBody2;
}

double SettingsIO::getFractionFeBody1()
{
	return FractionFeBody1;
}

double SettingsIO::getFractionSiBody1()
{
	return FractionSiBody1;
}

double SettingsIO::getFractionFeBody2()
{
	return FractionFeBody2;
}

double SettingsIO::getFractionSiBody2()
{
	return FractionSiBody2;
}

float SettingsIO::getDampRateBody1()
{
	return DampRateBody1;
}

float SettingsIO::getDampRateBody2()
{
	return DampRateBody2;
}

float SettingsIO::getEnergyTargetBody1()
{
	return EnergyTargetBody1;
}

float SettingsIO::getEnergyTargetBody2()
{
	return EnergyTargetBody2;
}

float SettingsIO::getTotalRunTime()
{
	return TotalRunTime;
}

float SettingsIO::getDampTime()
{
	return DampTime;
}

float SettingsIO::getDampRestTime()
{
	return DampRestTime;
}

float SettingsIO::getEnergyAdjustmentTime()
{
	return EnergyAdjustmentTime;
}

float SettingsIO::getEnergyAdjustmentRestTime()
{
	return EnergyAdjustmentRestTime;
}

float SettingsIO::getSpinRestTime()
{
	return SpinRestTime;
}

float SettingsIO::getDt()
{
	return Dt;
}

int SettingsIO::getWriteToFile()
{
	return WriteToFile;
}

int SettingsIO::getRecordRate()
{
	return RecordRate;
}

double SettingsIO::getDensityFe()
{
	return DensityFe;
}

double SettingsIO::getDensitySi()
{
	return DensitySi;
}

double SettingsIO::getKFe()
{
	return KFe;
}

double SettingsIO::getKSi()
{
	return KSi;
}

double SettingsIO::getKRFe()
{
	return KRFe;
}

double SettingsIO::getKRSi()
{
	return KRSi;
}

double SettingsIO::getSDFe()
{
	return SDFe;
}

double SettingsIO::getSDSi()
{
	return SDSi;
}

int SettingsIO::getDrawRate()
{
	return DrawRate;
}

int SettingsIO::getDrawQuality()
{
	return DrawQuality;
}

int SettingsIO::getUseMultipleGPU()
{
	return UseMultipleGPU;
}

double SettingsIO::getUniversalGravity()
{
	return UniversalGravity;
}
	
double SettingsIO::getMassOfEarth()
{
	return MassOfEarth;
}

double SettingsIO::getPi()
{
	return Pi;
}
