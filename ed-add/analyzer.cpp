/*

Note: this version only works with float4s that store the ID of the body in the 4th position,
so older .bin files probably won't work properly anymore.

*/

#include "get.cpp"

using namespace std;

void displayNew()
{
	glClear(GL_COLOR_BUFFER_BIT);
	glClear(GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	float3 centerOfMassEarthMoonSystem = getCenterOfMassCollision(0);
	LookAtX = centerOfMassEarthMoonSystem.x;
	LookAtY = centerOfMassEarthMoonSystem.y;
	LookAtZ = centerOfMassEarthMoonSystem.z;
	gluLookAt(EyeX, EyeY, EyeZ, LookAtX, LookAtY, LookAtZ, EyeUpX, EyeUpY, EyeUpZ);
	for(int i = 0; i < settings->N; i++)
	{
		if(Pos[i].w == 0)
		{
			glColor3d(1.0,0.0,0.0);
		}
		else if (Pos[i].w == 1)
		{
			glColor3d(1.0,1.0,0.5);
		}
		else if (Pos[i].w == 2)
		{
			glColor3d(1.0,0.0,1.0);
		}
		else
		{
			glColor3d(0.0,0.5,0.0);
		}
		
		glBegin( GL_POINTS );
		{
			glVertex3f(Pos[i].x,Pos[i].y,Pos[i].z);
			glEnd();
		}
	}
	glutSwapBuffers();
}

static void signalHandler(int signum) {
	int command;
	bool exitMenu = 0;
	
    	while (exitMenu == 0) {
    		cout << "You are currently at " << FrameCount << " out of " << NumFrames << "." << endl;
	    	cout << "Enter 0 to exit the program." << endl;
		cout << "Enter 1 to change where the camera looks at." << endl;
		cout << "Enter 2 to change the draw quality (not valid in the setup stage)." << endl;
		cout << "Enter 3 to set your eye location (EyeX, EyeY, EyeZ)." << endl;
		cout << "Enter 4 to print the run stats (not valid in the setup stage)." << endl;
		cout << "Enter 5 to seek to a certain point in the file." << endl;
		cout << "Enter 6 to exit the menu and resume the simulation." << endl;
		
		cin >> command;
		
		if(command == 0) {
			running = false;
			exitMenu = 1;
		} else if(command == 1) {
			cout << "Enter where you want to look at in the x direction (as a double): ";
		    	cin >> LookAtX;
		    	cout << "Enter where you want to look at in the y direction (as a double): ";
		    	cin >> LookAtY;
		    	cout << "Enter where you want to look at in the z direction (as a double): ";
		    	cin >> LookAtZ;
		    	
		    	cout << "Your eye now looks at:: x: " << LookAtX << "; y: " << LookAtY << "; z: " << LookAtZ << endl;
		    	
		    	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glLoadIdentity();
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glFrustum(-0.2, 0.2, -0.2, 0.2, Near, Far);
			glMatrixMode(GL_MODELVIEW);
			gluLookAt(EyeX, EyeY, EyeZ, LookAtX, LookAtY, LookAtZ, EyeUpX, EyeUpY, EyeUpZ);
			displayNew();
		} else if(command == 2) {
			cout << "Entered the desired draw quality (1 for analysis, 2 for regular): ";
			cout << "EXCEPT NOTHING HAPPENS YET, DEAL WITH IT." << endl;
			cout << "ty said i was a bully :(" << endl;
		} else if (command == 3) {   	
		    	cout << "Enter the desired x location of your eye (as a double): ";
		    	cin >> EyeX;
		    	cout << "Enter the desired y location of your eye (as a double): ";
		    	cin >> EyeY;
		    	cout << "Enter the desired z location of your eye (as a double): ";
		    	cin >> EyeZ;
		    	
		    	cout << "Your eye is now set at:: x: " << EyeX << "; y: " << EyeY << "; z: " << EyeZ << endl;
		    	
		    	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glLoadIdentity();
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glFrustum(-0.2, 0.2, -0.2, 0.2, Near, Far);
			glMatrixMode(GL_MODELVIEW);
			gluLookAt(EyeX, EyeY, EyeZ, LookAtX, LookAtY, LookAtZ, EyeUpX, EyeUpY, EyeUpZ);
			displayNew();
			
		} else if (command == 4) {
			printCollisionStatsToScreen();
		} else if (command == 5) {
			cout << "Enter how far into the file you wish to seek ahead (or a negative number for going back): ";
		    	cin >> Skip;
		    	
		    	if((Skip+FrameCount)>=NumFrames || (Skip+FrameCount)<0) {
		    		cout << "Invalid skip. Please try again." << endl;
		    	} else {
			    	cout << "Skipping ahead " << Skip << " frames." << endl;
			    	FrameCount = Skip + FrameCount;
				settings->seekReadPosVelFile(Skip, Pos, Vel);
				displayNew();
				cout << "You are currently at " << FrameCount << " out of " << NumFrames << "." << endl;
			}
		}
		else if (command == 6) {
			exitMenu = 1;
			cout << "Exiting menu." << endl;
		} else {
			cout << "Invalid command." << endl;
		}
		
		cout << endl;
	}
}

void control() {	
	struct sigaction sa;
	
	sa.sa_handler = signalHandler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART; // Restart functions if interrupted by handler
	
	if (sigaction(SIGINT, &sa, NULL) == -1) {
		cout << "TSU Error: sigaction error." << endl;
	}
	
	while(!feof(settings->PosAndVelFile) && running == true) {
		settings->readPosVelFile(Pos, Vel);
		displayNew();
		
		if(wait == true)
		{
			WaitFor(3);
		}
		FrameCount++;
	}
	
	cout << "Displaying final stats..." << endl;
	printCollisionStatsToScreen();
	freeMem();
	cout << "\n DONE \n" << endl;
	delete(settings);
	exit(0);
}

void Display(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(Left, Right, Bottom, Top, Front, Back);
	glMatrixMode(GL_MODELVIEW);
	gluLookAt(EyeX, EyeY, EyeZ, LookAtX, LookAtY, LookAtZ, EyeUpX, EyeUpY, EyeUpZ);
	
	control();
}

/*
Reshapes the OpenGL display
*/

void reshape(GLint w, GLint h) {
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(Left, Right, Bottom, Top, Front, Back);
	glMatrixMode(GL_MODELVIEW);
}

/*
Initializes the display
*/

void init() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(Left, Right, Bottom, Top, Front, Back);
	glMatrixMode(GL_MODELVIEW);
	gluLookAt(EyeX, EyeY, EyeZ, LookAtX, LookAtY, LookAtZ, EyeUpX, EyeUpY, EyeUpZ);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

/*
This is the main. It does the main stuff.
Main. Main main main. Main? Main. Main main main MAIN.
*/

int main(int argc, char** argv) {
	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB);
	glutInitWindowSize(XWindowSize,YWindowSize);
	glutInitWindowPosition(0,0);
	glutCreateWindow("MOON");
	
	glutReshapeFunc(reshape);
	
	cout << "Enter the folder name with the run: ";
	cin >> FolderNameTemp;
	const char* posNameTemp = FolderNameTemp.c_str();
	const char* statsNameTemp = FolderNameTemp.c_str();
	char posName[120]; 
	char statsName[120];
	strcpy(posName, posNameTemp);
	strcpy(statsName, statsNameTemp);
	strcat(posName,"/PosAndVel");
	strcat(statsName,"/RunSetup");
	
	readRunParameters(posName, statsName);
	
	init();
	
	glShadeModel(GL_SMOOTH);
	glClearColor(0.0, 0.0, 0.0, 0.0);
	
	glutDisplayFunc(Display);
	glutReshapeFunc(reshape);
	glutMainLoop();
	return 0;
}
