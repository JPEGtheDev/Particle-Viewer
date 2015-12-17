/*

Functions included in this file:
int findEarthAndMoon()
float getMassCollision(int bodyId)
float3 getCenterOfMassCollision(int bodyId)
float3 getLinearVelocityCollision(int bodyId)
float3 getAngularMomentumCollision(int bodyId)
void printCollisionStatsToScreen()

*/

#include <iostream>
#include "setup.cpp"

using namespace std;

int findEarthAndMoon()
{
	int groupId[settings->N], used[settings->N];
	float mag, dx, dy, dz;
	float touch = Diameter*1.5;
	int groupNumber, numberOfGroups;
	int k;
	
	for(int i = 0; i < settings->N; i++)
	{
		groupId[i] = -1;
		used[i] = 0;
	}
	
	groupNumber = 0;
	for(int i = 0; i < settings->N; i++)
	{
		if(groupId[i] == -1)
		{
			groupId[i] = groupNumber;
			//find all from this group
			k = i;
			while(k < settings->N)
			{
				if(groupId[k] == groupNumber && used[k] == 0)
				{
					for(int j = i; j < settings->N; j++)
					{
						dx = Pos[k].x - Pos[j].x;
						dy = Pos[k].y - Pos[j].y;
						dz = Pos[k].z - Pos[j].z;
						mag = sqrt(dx*dx + dy*dy + dz*dz);
						if(mag < touch)
						{
							groupId[j] = groupNumber;
						}
					}
					used[k] = 1;
					k = i;
				}
				else k++;	
			}
			
		}
		groupNumber++;
	}
	numberOfGroups = groupNumber;
	
	if(numberOfGroups == 1)
	{
		printf("\n No Moon found\n");
	}
	
	int count;
	int *groupSize = (int *)malloc(numberOfGroups*sizeof(int));
	for(int i = 0; i < numberOfGroups; i++)
	{
		count = 0;
		for(int j = 0; j < settings->N; j++)
		{
			if(i == groupId[j]) count++;
		}
		groupSize[i] = count;
	}
	
	int earthGroupId = -1;
	NumberOfEarthElements = 0;
	for(int i = 0; i < numberOfGroups; i++)
	{
		if(groupSize[i] > NumberOfEarthElements)
		{
			NumberOfEarthElements = groupSize[i];
			earthGroupId = i;
		}
	}
	
	int moonGroupId = -1;
	NumberOfMoonElements = 0;
	for(int i = 0; i < numberOfGroups; i++)
	{
		if(groupSize[i] > NumberOfMoonElements && i != earthGroupId)
		{
			NumberOfMoonElements = groupSize[i];
			moonGroupId = i;
		}
	}
	
	free(groupSize);
	EarthIndex = (int *)malloc(NumberOfEarthElements*sizeof(int));
	MoonIndex = (int *)malloc(NumberOfMoonElements*sizeof(int));
	
	int earthCount = 0;
	int moonCount = 0;
	for(int j = 0; j < settings->N; j++)
	{
		if(groupId[j] == earthGroupId) 
		{
			EarthIndex[earthCount] = j;
			earthCount++;
		}
		else if(groupId[j] == moonGroupId)  
		{
			MoonIndex[moonCount] = j;
			moonCount++;
		}
	}
	
	return(1);	
}

float getMassCollision(int scope)
{
	float mass = 0.0;
	
	if(scope == 0) // entire system
	{
		for(int i = 0; i < settings->N; i++)
		{
			if(i < NFe) mass += MassFe;
			else mass += MassSi;
		}
	}
	else if(scope == 1) // earth-moon syatem
	{
		for(int i = 0; i < NumberOfEarthElements; i++)
		{
			if(EarthIndex[i] < NFe) mass += MassFe;
			else mass += MassSi;
		}
		for(int i = 0; i < NumberOfMoonElements; i++)
		{
			if(MoonIndex[i] < NFe) mass += MassFe;
			else mass += MassSi;
		}
	}
	else if(scope == 2) // earth
	{
		for(int i = 0; i < NumberOfEarthElements; i++)
		{
			if(EarthIndex[i] < NFe) mass += MassFe;
			else mass += MassSi;
		}
	}
	else if(scope == 3) // moon
	{
		for(int i = 0; i < NumberOfMoonElements; i++)
		{
			if(MoonIndex[i] < NFe) mass += MassFe;
			else mass += MassSi;
		}
	}
	else
	{
		printf("\nTSU Error: In getMassCollision function bodyId invalid\n");
		exit(0);
	}
	return(mass);
}

float3 getCenterOfMassCollision(int scope)
{
	float totalMass;
	float3 centerOfMass;
	centerOfMass.x = 0.0;
	centerOfMass.y = 0.0;
	centerOfMass.z = 0.0;
	
	if(scope == 0)
	{
		for(int i = 0; i < settings->N; i++)
		{
			if(i < NFe)
			{
		    		centerOfMass.x += Pos[i].x*MassFe;
				centerOfMass.y += Pos[i].y*MassFe;
				centerOfMass.z += Pos[i].z*MassFe;
			}
			else
			{
		    		centerOfMass.x += Pos[i].x*MassSi;
				centerOfMass.y += Pos[i].y*MassSi;
				centerOfMass.z += Pos[i].z*MassSi;
			}
		}
		totalMass = getMassCollision(0);
		centerOfMass.x /= totalMass;
		centerOfMass.y /= totalMass;
		centerOfMass.z /= totalMass;
	}
	else if(scope == 1)
	{
		for(int i = 0; i < NumberOfEarthElements; i++)
		{
			if(EarthIndex[i] < NFe)
			{
		    		centerOfMass.x += Pos[EarthIndex[i]].x*MassFe;
				centerOfMass.y += Pos[EarthIndex[i]].y*MassFe;
				centerOfMass.z += Pos[EarthIndex[i]].z*MassFe;
			}
			else
			{
		    		centerOfMass.x += Pos[EarthIndex[i]].x*MassSi;
				centerOfMass.y += Pos[EarthIndex[i]].y*MassSi;
				centerOfMass.z += Pos[EarthIndex[i]].z*MassSi;
			}
		}
		for(int i = 0; i < NumberOfMoonElements; i++)
		{
			if(MoonIndex[i] < NFe)
			{
		    		centerOfMass.x += Pos[MoonIndex[i]].x*MassFe;
				centerOfMass.y += Pos[MoonIndex[i]].y*MassFe;
				centerOfMass.z += Pos[MoonIndex[i]].z*MassFe;
			}
			else
			{
		    		centerOfMass.x += Pos[MoonIndex[i]].x*MassSi;
				centerOfMass.y += Pos[MoonIndex[i]].y*MassSi;
				centerOfMass.z += Pos[MoonIndex[i]].z*MassSi;
			}
		}
		totalMass = getMassCollision(1);
		centerOfMass.x /= totalMass;
		centerOfMass.y /= totalMass;
		centerOfMass.z /= totalMass;
		
	}
	else if(scope == 2)
	{
		for(int i = 0; i < NumberOfEarthElements; i++)
		{
			if(EarthIndex[i] < NFe)
			{
		    		centerOfMass.x += Pos[EarthIndex[i]].x*MassFe;
				centerOfMass.y += Pos[EarthIndex[i]].y*MassFe;
				centerOfMass.z += Pos[EarthIndex[i]].z*MassFe;
			}
			else
			{
		    		centerOfMass.x += Pos[EarthIndex[i]].x*MassSi;
				centerOfMass.y += Pos[EarthIndex[i]].y*MassSi;
				centerOfMass.z += Pos[EarthIndex[i]].z*MassSi;
			}
		}
		totalMass = getMassCollision(2);
		centerOfMass.x /= totalMass;
		centerOfMass.y /= totalMass;
		centerOfMass.z /= totalMass;
	}
	else if(scope == 3)
	{
		for(int i = 0; i < NumberOfMoonElements; i++)
		{
			if(MoonIndex[i] < NFe)
			{
		    		centerOfMass.x += Pos[MoonIndex[i]].x*MassFe;
				centerOfMass.y += Pos[MoonIndex[i]].y*MassFe;
				centerOfMass.z += Pos[MoonIndex[i]].z*MassFe;
			}
			else
			{
		    		centerOfMass.x += Pos[MoonIndex[i]].x*MassSi;
				centerOfMass.y += Pos[MoonIndex[i]].y*MassSi;
				centerOfMass.z += Pos[MoonIndex[i]].z*MassSi;
			}
		}
		totalMass = getMassCollision(3);
		centerOfMass.x /= totalMass;
		centerOfMass.y /= totalMass;
		centerOfMass.z /= totalMass;
	}
	else
	{
		printf("\nTSU Error: In getCenterOfMassCollision function scope invalid\n");
		exit(0);
	}
	return(centerOfMass);
}

float3 getLinearVelocityCollision(int scope)
{
	float totalMass;
	float3 linearVelocity;
	linearVelocity.x = 0.0;
	linearVelocity.y = 0.0;
	linearVelocity.z = 0.0;
	
	if(scope == 0) // entire system
	{
		for(int i = 0; i < settings->N; i++)
		{
			if(i < NFe)
			{
		    		linearVelocity.x += Vel[i].x*MassFe;
				linearVelocity.y += Vel[i].y*MassFe;
				linearVelocity.z += Vel[i].z*MassFe;
			}
			else
			{
		    		linearVelocity.x += Vel[i].x*MassSi;
				linearVelocity.y += Vel[i].y*MassSi;
				linearVelocity.z += Vel[i].z*MassSi;
			}
		}
		totalMass = getMassCollision(0);
		linearVelocity.x /= totalMass;
		linearVelocity.y /= totalMass;
		linearVelocity.z /= totalMass;
	}	
	else if(scope == 1) // earth-moon system
	{
		for(int i = 0; i < NumberOfEarthElements; i++)
		{
			if(EarthIndex[i] < NFe)
			{
		    		linearVelocity.x += Vel[EarthIndex[i]].x*MassFe;
				linearVelocity.y += Vel[EarthIndex[i]].y*MassFe;
				linearVelocity.z += Vel[EarthIndex[i]].z*MassFe;
			}
			else
			{
		    		linearVelocity.x += Vel[EarthIndex[i]].x*MassSi;
				linearVelocity.y += Vel[EarthIndex[i]].y*MassSi;
				linearVelocity.z += Vel[EarthIndex[i]].z*MassSi;
			}
		}
		for(int i = 0; i < NumberOfMoonElements; i++)
		{
			if(MoonIndex[i] < NFe)
			{
		    		linearVelocity.x += Vel[MoonIndex[i]].x*MassFe;
				linearVelocity.y += Vel[MoonIndex[i]].y*MassFe;
				linearVelocity.z += Vel[MoonIndex[i]].z*MassFe;
			}
			else
			{
		    		linearVelocity.x += Vel[MoonIndex[i]].x*MassSi;
				linearVelocity.y += Vel[MoonIndex[i]].y*MassSi;
				linearVelocity.z += Vel[MoonIndex[i]].z*MassSi;
			}
		}
		totalMass = getMassCollision(1);
		linearVelocity.x /= totalMass;
		linearVelocity.y /= totalMass;
		linearVelocity.z /= totalMass;
	}
	else if(scope == 2) //earth
	{
		for(int i = 0; i < NumberOfEarthElements; i++)
		{
			if(EarthIndex[i] < NFe)
			{
		    		linearVelocity.x += Vel[EarthIndex[i]].x*MassFe;
				linearVelocity.y += Vel[EarthIndex[i]].y*MassFe;
				linearVelocity.z += Vel[EarthIndex[i]].z*MassFe;
			}
			else
			{
		    		linearVelocity.x += Vel[EarthIndex[i]].x*MassSi;
				linearVelocity.y += Vel[EarthIndex[i]].y*MassSi;
				linearVelocity.z += Vel[EarthIndex[i]].z*MassSi;
			}
		}
		totalMass = getMassCollision(2);
		linearVelocity.x /= totalMass;
		linearVelocity.y /= totalMass;
		linearVelocity.z /= totalMass;
	}
	else if(scope == 3) //moon
	{
		for(int i = 0; i < NumberOfMoonElements; i++)
		{
			if(MoonIndex[i] < NFe)
			{
		    		linearVelocity.x += Vel[MoonIndex[i]].x*MassFe;
				linearVelocity.y += Vel[MoonIndex[i]].y*MassFe;
				linearVelocity.z += Vel[MoonIndex[i]].z*MassFe;
			}
			else
			{
		    		linearVelocity.x += Vel[MoonIndex[i]].x*MassSi;
				linearVelocity.y += Vel[MoonIndex[i]].y*MassSi;
				linearVelocity.z += Vel[MoonIndex[i]].z*MassSi;
			}
		}
		totalMass = getMassCollision(3);
		linearVelocity.x /= totalMass;
		linearVelocity.y /= totalMass;
		linearVelocity.z /= totalMass;
	}
	else
	{
		printf("\nTSU Error: in getlinearVelocityEarthMoonSystem function scope invalid\n");
		exit(0);
	}
	return(linearVelocity);
}

float3 getAngularMomentumCollision(int scope)
{
	float3 centerOfMass, linearVelocity, angularMomentum;
	float3 r;
	float3 v;
	angularMomentum.x = 0.0;
	angularMomentum.y = 0.0;
	angularMomentum.z = 0.0;
	
	if(scope == 0) //Entire system
	{
		centerOfMass = getCenterOfMassCollision(0);
		linearVelocity = getLinearVelocityCollision(0);
		for(int i = 0; i < settings->N; i++)
		{
			r.x = Pos[i].x - centerOfMass.x;
			r.y = Pos[i].y - centerOfMass.y;
			r.z = Pos[i].z - centerOfMass.z;
		
			v.x = Vel[i].x - linearVelocity.x;
			v.y = Vel[i].y - linearVelocity.y;
			v.z = Vel[i].z - linearVelocity.z;
			if(i < NFe)
			{
		    		angularMomentum.x +=  (r.y*v.z - r.z*v.y)*MassFe;
				angularMomentum.y += -(r.x*v.z - r.z*v.x)*MassFe;
				angularMomentum.z +=  (r.x*v.y - r.y*v.x)*MassFe;
			}
			else
			{
				angularMomentum.x +=  (r.y*v.z - r.z*v.y)*MassSi;
				angularMomentum.y += -(r.x*v.z - r.z*v.x)*MassSi;
				angularMomentum.z +=  (r.x*v.y - r.y*v.x)*MassSi;
			}
		}
	}
	else if(scope == 1) //Earth-Moon system
	{
		centerOfMass = getCenterOfMassCollision(1);
		linearVelocity = getLinearVelocityCollision(1);
		for(int i = 0; i < NumberOfEarthElements; i++)
		{
			r.x = Pos[EarthIndex[i]].x - centerOfMass.x;
			r.y = Pos[EarthIndex[i]].y - centerOfMass.y;
			r.z = Pos[EarthIndex[i]].z - centerOfMass.z;
		
			v.x = Vel[EarthIndex[i]].x - linearVelocity.x;
			v.y = Vel[EarthIndex[i]].y - linearVelocity.y;
			v.z = Vel[EarthIndex[i]].z - linearVelocity.z;
			if(EarthIndex[i] < NFe)
			{
		    		angularMomentum.x +=  (r.y*v.z - r.z*v.y)*MassFe;
				angularMomentum.y += -(r.x*v.z - r.z*v.x)*MassFe;
				angularMomentum.z +=  (r.x*v.y - r.y*v.x)*MassFe;
			}
			else
			{
				angularMomentum.x +=  (r.y*v.z - r.z*v.y)*MassSi;
				angularMomentum.y += -(r.x*v.z - r.z*v.x)*MassSi;
				angularMomentum.z +=  (r.x*v.y - r.y*v.x)*MassSi;
			}
		}
		for(int i = 0; i < NumberOfMoonElements; i++)
		{
			r.x = Pos[MoonIndex[i]].x - centerOfMass.x;
			r.y = Pos[MoonIndex[i]].y - centerOfMass.y;
			r.z = Pos[MoonIndex[i]].z - centerOfMass.z;
		
			v.x = Vel[MoonIndex[i]].x - linearVelocity.x;
			v.y = Vel[MoonIndex[i]].y - linearVelocity.y;
			v.z = Vel[MoonIndex[i]].z - linearVelocity.z;
			if(MoonIndex[i] < NFe)
			{
		    		angularMomentum.x +=  (r.y*v.z - r.z*v.y)*MassFe;
				angularMomentum.y += -(r.x*v.z - r.z*v.x)*MassFe;
				angularMomentum.z +=  (r.x*v.y - r.y*v.x)*MassFe;
			}
			else
			{
				angularMomentum.x +=  (r.y*v.z - r.z*v.y)*MassSi;
				angularMomentum.y += -(r.x*v.z - r.z*v.x)*MassSi;
				angularMomentum.z +=  (r.x*v.y - r.y*v.x)*MassSi;
			}
		}
	}
	else if(scope == 2) //Earth
	{
		centerOfMass = getCenterOfMassCollision(2);
		linearVelocity = getLinearVelocityCollision(2);
		for(int i = 0; i < NumberOfEarthElements; i++)
		{
			r.x = Pos[EarthIndex[i]].x - centerOfMass.x;
			r.y = Pos[EarthIndex[i]].y - centerOfMass.y;
			r.z = Pos[EarthIndex[i]].z - centerOfMass.z;
		
			v.x = Vel[EarthIndex[i]].x - linearVelocity.x;
			v.y = Vel[EarthIndex[i]].y - linearVelocity.y;
			v.z = Vel[EarthIndex[i]].z - linearVelocity.z;
			if(EarthIndex[i] < NFe)
			{
		    		angularMomentum.x +=  (r.y*v.z - r.z*v.y)*MassFe;
				angularMomentum.y += -(r.x*v.z - r.z*v.x)*MassFe;
				angularMomentum.z +=  (r.x*v.y - r.y*v.x)*MassFe;
			}
			else
			{
				angularMomentum.x +=  (r.y*v.z - r.z*v.y)*MassSi;
				angularMomentum.y += -(r.x*v.z - r.z*v.x)*MassSi;
				angularMomentum.z +=  (r.x*v.y - r.y*v.x)*MassSi;
			}
		}
	}
	else if(scope == 3) //Moon
	{
		centerOfMass = getCenterOfMassCollision(3);
		linearVelocity = getLinearVelocityCollision(3);
		for(int i = 0; i < NumberOfMoonElements; i++)
		{
			r.x = Pos[MoonIndex[i]].x - centerOfMass.x;
			r.y = Pos[MoonIndex[i]].y - centerOfMass.y;
			r.z = Pos[MoonIndex[i]].z - centerOfMass.z;
		
			v.x = Vel[MoonIndex[i]].x - linearVelocity.x;
			v.y = Vel[MoonIndex[i]].y - linearVelocity.y;
			v.z = Vel[MoonIndex[i]].z - linearVelocity.z;
			if(MoonIndex[i] < NFe)
			{
		    		angularMomentum.x +=  (r.y*v.z - r.z*v.y)*MassFe;
				angularMomentum.y += -(r.x*v.z - r.z*v.x)*MassFe;
				angularMomentum.z +=  (r.x*v.y - r.y*v.x)*MassFe;
			}
			else
			{
				angularMomentum.x +=  (r.y*v.z - r.z*v.y)*MassSi;
				angularMomentum.y += -(r.x*v.z - r.z*v.x)*MassSi;
				angularMomentum.z +=  (r.x*v.y - r.y*v.x)*MassSi;
			}
		}
	}
	else
	{
		printf("\nTSU Error: in getAngularMomentumCollision function scope invalid\n");
		exit(0);
	}
	return(angularMomentum);
}

void printCollisionStatsToScreen()
{	
	double mag, angle, x, y, z, lengthConverter, velocityConverter, momentumConverter;
	
	findEarthAndMoon();
	int earthFeCountBody1 = 0;
	int earthFeCountBody2 = 0;
	int earthSiCountBody1 = 0;
	int earthSiCountBody2 = 0;
	int moonFeCountBody1 = 0;
	int moonFeCountBody2 = 0;
	int moonSiCountBody1 = 0;
	int moonSiCountBody2 = 0;
	
	float massUniversalSystem = getMassCollision(0);
	float massEarthMoonSystem = getMassCollision(1);
	float massEarth = getMassCollision(2);
	float massMoon = getMassCollision(3);
	
	float3 centerOfMassUniversalSystem = getCenterOfMassCollision(0);
	float3 centerOfMassEarthMoonSystem = getCenterOfMassCollision(1);
	float3 centerOfMassEarth = getCenterOfMassCollision(2);
	float3 centerOfMassMoon = getCenterOfMassCollision(3);
	
	float3 linearVelocityUniversalSystem = getLinearVelocityCollision(0);
	float3 linearVelocityEarthMoonSystem = getLinearVelocityCollision(1);
	float3 linearVelocityEarth = getLinearVelocityCollision(2);
	float3 linearVelocityMoon = getLinearVelocityCollision(3);
	
	float3 angularMomentumUniversalSystem = getAngularMomentumCollision(0);
	float3 angularMomentumEarthMoonSystem = getAngularMomentumCollision(1);
	float3 angularMomentumEarth = getAngularMomentumCollision(2);
	float3 angularMomentumMoon = getAngularMomentumCollision(3);
	
	for(int i = 0; i < NumberOfEarthElements; i++)
	{
		if(EarthIndex[i] < NFe1) 			earthFeCountBody1++;
		else if(EarthIndex[i] < NFe1 + NFe2) 		earthFeCountBody2++;
		else if(EarthIndex[i] < NFe1 + NFe2 + NSi1) 	earthSiCountBody1++;
		else 						earthSiCountBody2++;
	}
	
	for(int i = 0; i < NumberOfMoonElements; i++)
	{
		if(MoonIndex[i] < NFe1) 			moonFeCountBody1++;
		else if(MoonIndex[i] < NFe1 + NFe2) 		moonFeCountBody2++;
		else if(MoonIndex[i] < NFe1 + NFe2 + NSi1) 	moonSiCountBody1++;
		else 						moonSiCountBody2++;
	}
	
	cout << "\n\n\n*************************************************************************\n\n\n";
	cout << "Distance is measured in Kilometers\n";
	cout << "Mass is measured in Kilograms\n";
	cout << "Time is measured in seconds\n";
	cout << "Velocity is measured in Kilometers/second\n";
	cout << "Angular momentum is measured in Kilograms*Kilometers*Kilometers/seconds\n\n";
	
	printf("The mass of Earth 		= %e\n", massEarth*UnitMass);
	printf("The mass of Moon 		= %e\n", massMoon*UnitMass);
	if(massMoon != 0.0) printf("The mass ratio Earth/Moon 	= %f\n\n", massEarth/massMoon);
	
	printf("\nMoon iron from body 1 		= %d", moonFeCountBody1);
	printf("\nMoon silicate from body 1 	= %d", moonSiCountBody1);
	printf("\nMoon iron from body 2 		= %d", moonFeCountBody2);
	printf("\nMoon silicate from body 2 	= %d", moonSiCountBody2);
	if((moonFeCountBody2 + moonSiCountBody2) == 0)
	{
		printf("\nThe Moon is only composed of elements from body 1\n");
	}
	else if((moonFeCountBody1 + moonSiCountBody1) == 0)
	{
		printf("\nThe Moon is only composed of elements from body 2\n");
	}
	else
	{
		printf("\nMoon ratio body1/body2 		= %f\n", (float)(moonFeCountBody1 + moonSiCountBody1)/(float)(moonFeCountBody2 + moonSiCountBody2));
	}
	
	//It is always assumed that the ecliptic plane is the xz-plane.
	momentumConverter = UnitMass*UnitLength*UnitLength/UnitTime;
	x = angularMomentumEarthMoonSystem.x*momentumConverter;
	y = angularMomentumEarthMoonSystem.y*momentumConverter;
	z = angularMomentumEarthMoonSystem.z*momentumConverter;
	printf("\nAngular momentum of the Earth Moon system 		= (%e, %e, %e)", x, y, z);
	mag = sqrt(x*x + y*y + z*z);
	printf("\nMagnitude of the angular momentum of the system 	= %e", mag);
	angle = acos(y/mag);
	printf("\nAngle off ecliptic plane of the system's rotation 	= %f\n", angle*180.0/settings->getPi());
	
	x = angularMomentumEarth.x*momentumConverter;
	y = angularMomentumEarth.y*momentumConverter;
	z = angularMomentumEarth.z*momentumConverter;
	printf("\nAngular momentum of the Earth 				= (%e, %e, %e)", x, y, z);
	mag = sqrt(x*x + y*y + z*z);
	printf("\nMagnitude of the angular momentum of the Earth 		= %e", mag);
	angle = acos(y/mag);
	printf("\nAngle off ecliptic plane of the Earth's rotation 	= %f\n", angle*180.0/settings->getPi());
	
	x = angularMomentumMoon.x*momentumConverter;
	y = angularMomentumMoon.y*momentumConverter;
	z = angularMomentumMoon.z*momentumConverter;
	printf("\nAngular momentum of the Moon 				= (%e, %e, %e)", x, y, z);
	mag = sqrt(x*x + y*y + z*z);
	printf("\nMagnitude of the angular momentum of the Moon 		= %e", mag);
	angle = acos(y/mag);
	printf("\nAngle off ecliptic plane of the Moon's rotation 	= %f\n", angle*180.0/settings->getPi());
	
	printf("\nStats of the entire system to check the numerical scheme's validity\n");
	
	lengthConverter = UnitLength;
	x = centerOfMassUniversalSystem.x*lengthConverter;
	y = centerOfMassUniversalSystem.y*lengthConverter;
	z = centerOfMassUniversalSystem.z*lengthConverter;
	printf("\nCenter of mass of the entire system system 		= (%f, %f, %f)\n", x, y, z);
	
	velocityConverter = UnitLength/UnitTime;
	x = linearVelocityUniversalSystem.x*velocityConverter;
	y = linearVelocityUniversalSystem.y*velocityConverter;
	z = linearVelocityUniversalSystem.z*velocityConverter;
	printf("\nLinear velocity of the entire system system 		= (%f, %f, %f)", x, y, z);
	mag = sqrt(x*x + y*y + z*z);
	printf("\nMagnitude of the linear velocity of the entire system 	= %f\n", mag);
	
	x = angularMomentumUniversalSystem.x*momentumConverter;
	y = angularMomentumUniversalSystem.y*momentumConverter;
	z = angularMomentumUniversalSystem.z*momentumConverter;
	printf("\nAngular momentum of the entire system system 		= (%e, %e, %e)", x, y, z);
	mag = sqrt(x*x + y*y + z*z);
	printf("\nMagnitude of the angular momentum of the entire system 	= %e\n", mag);
	
	printf("\n\n\n*************************************************************************\n\n\n");
}
