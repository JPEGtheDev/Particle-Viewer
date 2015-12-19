#include "settingsIO.hpp"
using namespace std;

SettingsIO::SettingsIO() 
{
	posFile = "/PosAndVel";
	statsFile = "/RunSetup";
}

SettingsIO::SettingsIO(string posName, string statsName)
{
	this->posName = posName;
	this->statsName = statsName;
	ifstream data;
	posFile = "/PosAndVel";
	statsFile = "/RunSetup";
	string name;
	string blank;
	errorCount = 0;
	isPlaying = false;
	data.open(statsName.c_str());

	if(data.good() && data.is_open())
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
	else
	{
		this->InitialPosition1.x = 100;
		this->InitialPosition1.y = 100;
		this->InitialPosition1.z = 100;
		this->InitialPosition2.x = 100;
		this->InitialPosition2.y = 100;
		this->InitialPosition2.z = 100;
		this->InitialVelocity1.x = 100;
		this->InitialVelocity1.y = 100;
		this->InitialVelocity1.z = 100;
		this->InitialVelocity2.x = 100;
		this->InitialVelocity2.y = 100;
		this->InitialVelocity2.z = 100;
		this->InitialSpin1.x = 100;
		this->InitialSpin1.y = 100;
		this->InitialSpin1.z = 100;
		this->InitialSpin1.w = 100;
		this->InitialSpin2.x = 100;
		this->InitialSpin2.y = 100;
		this->InitialSpin2.z = 100;
		this->InitialSpin2.w = 100;
		this->FractionEarthMassOfBody1 = 100;
		this->FractionEarthMassOfBody2 = 100;
		this->FractionFeBody1 = 100;
		this->FractionSiBody1 = 100;
		this->FractionFeBody2 = 100;
		this->FractionSiBody2 = 100;
		this->DampRateBody1 = 100;
		this->DampRateBody2 = 100;
		this->EnergyTargetBody1 = 100;
		this->EnergyTargetBody2 = 100;
		this->N = 100;
		this->TotalRunTime = 100;
		this->DampTime = 100;
		this->DampRestTime = 100;
		this->EnergyAdjustmentTime = 100;
		this->EnergyAdjustmentRestTime = 100;
		this->SpinRestTime = 100;
		this->Dt = 100;
		this->WriteToFile = 100;
		this->RecordRate = 100;
		this->DensityFe = 100;
		this->DensitySi = 100;
		this->KFe = 100;
		this->KSi = 100;
		this->KRFe = 100;
		this->KRSi = 100;
		this->SDFe = 100;
		this->SDSi = 100;
		this->DrawRate = 100;
		this->DrawQuality = 100;
		this->UseMultipleGPU = 100;
		this->UniversalGravity = 100;
		this->MassOfEarth = 100;
		this->Pi = 100;
	}
	data.close();
	frames = getFrames();
}

SettingsIO::~SettingsIO()
{
	//fclose(PosAndVelFile);
}

void SettingsIO::readPosVelFile(long frame, Particle *part,bool readVelocity)
{
	FILE *PosAndVelFile = fopen(posName.c_str(), "r");
	if(PosAndVelFile)
	{
		if(frame >= frames)
		{
			frame = frames-1;
			isPlaying = false;
		}
		if(frame < 0)
		{
			frame = 0;
			isPlaying = false;
		}
		fseek(PosAndVelFile, frame * sizeof(glm::vec4) * 2 * N, SEEK_CUR);
		glm::vec4 *pos = new glm::vec4[N];
		glm::vec4 *vel;
		fread(pos, sizeof(glm::vec4), (int)N, PosAndVelFile);
		part->changeTranslations(N,pos);
		if(readVelocity)
		{
			vel = new glm::vec4[N];
			fread(vel, sizeof(glm::vec4), N, PosAndVelFile);
			part->changeVelocities(vel);
			delete[] vel;
		}
		delete[] pos;
		fclose(PosAndVelFile);
		return;
	}
	//fclose(PosAndVelFile);
	errorCount++;
	if(errorCount < 5)
	{
		cout << "Error Reading File. Attempt: " << errorCount << endl;
	}
	else if (errorCount == 5)
	{
		cout << "Too many errors. stopping log here" << endl;
	}
	
}

void SettingsIO::seekReadPosVelFile(int frame, Particle *part, bool readVelocity)
{

	
	
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
long long int SettingsIO::getFrames()
{
	std::ifstream input;
	input.open(posName, std::ios::in | std::ios::binary);
	if(input)
	{
		unsigned long long int size = 0;
		input.seekg(0,std::ios::end);
		size = input.tellg();
		size = size / (sizeof(glm::vec4) * 2 * N);
		input.seekg(0,std::ios::beg);
		return size;
	}
	cout << "Error Getting File Size" << endl;
	return 1;
}
void SettingsIO::togglePlay()
{
	isPlaying = !isPlaying;
}
SettingsIO* SettingsIO::loadFile(Particle *part, bool readVelocity)
{
	string dialog = "Select Folder";
	const char* fol = tinyfd_selectFolderDialog (dialog.c_str() , "") ;
	
	string folder;
	if(fol != NULL)
	{
		folder = string(fol);
	}
	else
	{
		folder = "";
	}
	if(folder != "")
	{
		string posVel = folder;//strcat(folder,posFile.c_str());
		posVel = posVel + posFile;
		string settings = folder + statsFile;//strcat(folder, settingsFile.c_str());
		SettingsIO *set = new SettingsIO(posVel.c_str(),settings.c_str());
		set->readPosVelFile(0,part,readVelocity);
		return set;
	}
	cout << "Folder not selected" << endl;
	return this;
}
