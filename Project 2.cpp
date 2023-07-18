#include <TL-Engine.h>	// TL-Engine include file and namespace
#include <sstream>      // Writing to game screen
using namespace tle; // T-L Engine definitions (I guess)
using namespace std; // Standard definitions
#include<iostream>
#include <windows.h> // Include this header for Windows-specific Sleep()

enum currentState { Demo, Count_Down, Stage_0_Complete, Stage_1_Complete, Stage_2, Race_Complete }; // Game states										 
enum carState { Hovering, WaitingForGo, Racing };
enum hoverState { HoverUp, HoverDown, None };
currentState GameState = Demo;
const int CheckPoint_count = 2, isle_count = 4;

ISprite* backdrop;	// Background sprite (used for Speed-o-meter)
IModel* myCar, * Car_Temp, * track_isles[isle_count], * track_checkPoints[CheckPoint_count]; // Game Models/Assets

int myCarStatus = Hovering, currentGameStage = 0;
bool gamePaused, displayStage = false, collision = false;
float frameRate, myCarSpeed, rotationDegrees, speedLimit = 0.3f, speedIncrementer = 0.001f, rotationAmt = 0.0f, hoverSpeed = 2.0f;
const float maxRotation = 0.2f, steerAmt = 20.0f, carGround = 7.5f;

const float objFloor = 1.0f, isleWidth = 2.68f, isleLength = 3.42f;
float isles_minX[isle_count], isles_maxX[isle_count], isles_minZ[isle_count], isles_maxZ[isle_count];

const float cpLength = 9.86f, cpWidth = 1.28f;
float cp_left_minX[isle_count], cp_left_maxX[isle_count], cp_left_minZ[isle_count], cp_left_maxZ[isle_count];
float cp_right_minX[isle_count], cp_right_maxX[isle_count], cp_right_minZ[isle_count], cp_right_maxZ[isle_count];
float cp_through_minX[isle_count], cp_through_maxX[isle_count], cp_through_minZ[isle_count], cp_through_maxZ[isle_count];

const float cameraForwardsLimit = 620.0f, cameraBackwardsLimit = -140.0f, cameraLeftLimit = 0.0f, cameraRightLimit = 420.0f, cameraSpeed = 20.0f;

void isSatgeCompleted(int currentCP)
{
	if (myCar->GetX() >= cp_through_minX[currentCP] && myCar->GetX() <= cp_through_maxX[currentCP] &&
		myCar->GetZ() >= cp_through_minZ[currentCP] && myCar->GetZ() <= cp_through_maxZ[currentCP] &&
		currentGameStage == currentCP)
	{
		currentGameStage++;
		displayStage = true;
	}
}
void printGameCompletedStage(IFont* gameFont)
{
	if (displayStage)
	{
		if (currentGameStage < 2)
		{
			gameFont->Draw("Stage " + to_string(currentGameStage) + " Completed!", 500, 200, kBlack);
		}
		else
		{
			gameFont->Draw("Race Complete!", 510, 200, kBlack);
			myCarSpeed = 0.0f;
		}
	}
}
void print_BackDrop_CarSpeed_and_State(IFont* gameFont)
{
	gameFont->Draw("Speed: " + to_string(myCarSpeed), 5, 670, 0xFFFFA500);
	if (GameState == Demo)
		gameFont->Draw("State: Demo", 1075, 670, 0xFFFFA500);
	else if (GameState == Count_Down)
		gameFont->Draw("State: Count Down", 990, 670, 0xFFFFA500);
	else if (GameState == Stage_0_Complete)
		gameFont->Draw("State: Stage 0 Complete", 930, 670, 0xFFFFA500);
	else if (GameState == Stage_1_Complete)
		gameFont->Draw("State: Stage 1 Complete", 930, 670, 0xFFFFA500);
	else if (GameState == Race_Complete)
		gameFont->Draw("State: Race Complete", 940, 670, 0xFFFFA500);
	else
		gameFont->Draw("State: Unknown State", 1075, 670, 0xFFFFA500);
}

void stop_car()
{
	if (myCarSpeed > 0.0f)	myCarSpeed = -myCarSpeed / 3.5f;
	else	myCarSpeed -= myCarSpeed * 1.5f;
}

void detect_collision(float objMinX, float objMaxX, float objMinZ, float objMaxZ)
{
	collision = (Car_Temp->GetLocalX() < objMaxX && Car_Temp->GetLocalX() > objMinX && Car_Temp->GetLocalZ() < objMaxZ && Car_Temp->GetLocalZ() > objMinZ);
	if (collision)
		stop_car();
}

void move_left(ICamera* myCamera)
{
	if (rotationAmt >= -maxRotation) rotationAmt -= 0.4f * frameRate;
	Car_Temp->RotateLocalY(-steerAmt * frameRate);
}

void move_right(ICamera* myCamera)
{
	if (rotationAmt < maxRotation) rotationAmt += 0.4f * frameRate;
	Car_Temp->RotateLocalY(steerAmt * frameRate);
}

void remove_rotation()
{
	if (rotationAmt > 0.0f)  rotationAmt -= 0.4f * frameRate;
	if (rotationAmt < 0.01f)  rotationAmt += 0.4f * frameRate;
}

void update_car()
{
	float bank, angle;
	if (!collision)
	{
		Car_Temp->MoveLocalZ(myCarSpeed);
		myCar->SetZ(Car_Temp->GetZ());
		myCar->SetX(Car_Temp->GetX());
		angle = myCarSpeed / rotationDegrees;
		bank = rotationAmt / rotationDegrees;
		myCar->ResetOrientation();
		myCar->RotateLocalX(-angle);
		myCar->RotateLocalZ(-bank * 2.0f);
		myCar->RotateLocalY(bank);
	}
}

void main()
{
	system("Hover Car Racing Console");			// Set TL Engine Title
	I3DEngine* myEngine = New3DEngine(kTLX);	// Create a 3D engine (using TLX engine here)
	myEngine->StartWindowed();
	myEngine->SetWindowCaption("Hover Car Racing");	// Set Window title
	myEngine->AddMediaFolder("./media");	//Media Folder (all Meshes, fonsts here)
	ISprite* backdropSprite = myEngine->CreateSprite("ui_backdrop.jpg", -450.0f, 660.0f, 0.0f);
	IFont* gameFont = myEngine->LoadFont("Monotype Corsiva", 45);
	IFont* SpeedFont = myEngine->LoadFont("Monotype Corsiva", 45);
	IMesh* groundMesh = myEngine->LoadMesh("ground.x");
	IModel* ground = groundMesh->CreateModel(0.0f, 0.0f, 0.0f);
	IMesh* skyboxMesh = myEngine->LoadMesh("skybox 07.x");
	IModel* skybox = skyboxMesh->CreateModel(0.0f, -960.0f, 0.0f);

	IMesh* checkPointMesh = myEngine->LoadMesh("checkpoint.x");
	track_checkPoints[0] = checkPointMesh->CreateModel(2.0f, objFloor, 30.0f);
	track_checkPoints[1] = checkPointMesh->CreateModel(2.0f, objFloor, 300.0f);
	for (int i = 0; i < CheckPoint_count; i++)
	{
		track_checkPoints[i]->ScaleY(2.0f);
		cp_through_minX[i] = track_checkPoints[i]->GetX() - cpLength;
		cp_through_maxX[i] = track_checkPoints[i]->GetX() + cpLength;
		cp_through_minZ[i] = track_checkPoints[i]->GetZ() - cpWidth;
		cp_through_maxZ[i] = track_checkPoints[i]->GetZ() + cpWidth;
		cp_left_minX[i] = track_checkPoints[i]->GetX() - 12.0f;
		cp_left_maxX[i] = track_checkPoints[i]->GetX() - 7.0f;
		cp_left_minZ[i] = track_checkPoints[i]->GetZ() - 10.0f;
		cp_left_maxZ[i] = track_checkPoints[i]->GetZ() + 5.0f;
		cp_right_minX[i] = track_checkPoints[i]->GetX() + 7.0f;
		cp_right_maxX[i] = track_checkPoints[i]->GetX() + 12.0f;
		cp_right_minZ[i] = track_checkPoints[i]->GetZ() - 10.0f;
		cp_right_maxZ[i] = track_checkPoints[i]->GetZ() + 5.0f;
	}

	ICamera* myCamera = myEngine->CreateCamera(kManual, 0.0f, 20.0f, -60.0f); // Create a camera

	IMesh* isleMesh = myEngine->LoadMesh("islestraight.x");
	track_isles[0] = isleMesh->CreateModel(-12.5f, objFloor, 190.0f);
	track_isles[1] = isleMesh->CreateModel(17.5f, objFloor, 190.0f);
	track_isles[2] = isleMesh->CreateModel(17.5f, objFloor, 203.0f);
	track_isles[3] = isleMesh->CreateModel(-12.5f, objFloor, 203.0f);
	for (int i = 0; i < isle_count; i++)
	{
		isles_minX[i] = track_isles[i]->GetX() - isleWidth;
		isles_maxX[i] = track_isles[i]->GetX() + isleWidth;
		isles_minZ[i] = track_isles[i]->GetZ() - isleLength;
		isles_maxZ[i] = track_isles[i]->GetZ() + isleLength;
	}

	IMesh* hoverCarMesh = myEngine->LoadMesh("race2.x");
	IMesh* dummyMesh = myEngine->LoadMesh("race2.x");

	myCar = hoverCarMesh->CreateModel(0.0f, 2.0f, -20.0f);
	myCarSpeed = 0.0f;
	Car_Temp = dummyMesh->CreateModel(0.0f, carGround, -20.0f);
	myCar->AttachToParent(Car_Temp);
	myCamera->AttachToParent(Car_Temp);
	myCar->SetSkin("spengland.jpg");

	rotationDegrees = 0.2f / 10.0f;

	while (myEngine->IsRunning())
	{
		frameRate = myEngine->Timer();
		myEngine->DrawScene();
		print_BackDrop_CarSpeed_and_State(SpeedFont);

		if (!gamePaused)
		{
			switch (myCarStatus)
			{
			case Hovering:
				myCamera->RotateY(myEngine->GetMouseMovementX() * 0.1f);
				gameFont->Draw("Press space bar to begin!", 500, 200, kBlack);
				break;
			case WaitingForGo:
				myCamera->ResetOrientation();
				myCarStatus = Racing;
				break;
			case Racing:
				update_car();
				break;
			default:
				break;
			}
			printGameCompletedStage(gameFont);

			for (int i = 0; i < CheckPoint_count; i++)
			{   // Check for collision between car and sides of the check point
				detect_collision(cp_left_minX[i], cp_left_maxX[i], cp_left_minZ[i], cp_left_maxZ[i]);
				detect_collision(cp_right_minX[i], cp_right_maxX[i], cp_right_minZ[i], cp_right_maxZ[i]);
				isSatgeCompleted(i);
				if (1 == currentGameStage)
					GameState = Stage_1_Complete;
				if (2 == currentGameStage)
					GameState = Race_Complete;
			}

			for (int isle = 0; isle < isle_count; isle++)	// Check collision between car and isles
				detect_collision(isles_minX[isle], isles_maxX[isle], isles_minZ[isle], isles_maxZ[isle]);

			if (myEngine->KeyHeld(Key_W))
			{
				if (myCarSpeed < speedLimit && myCarStatus == Racing) myCarSpeed += speedIncrementer;
			}
			else if (myEngine->KeyHeld(Key_S) && myCarStatus == Racing)
			{
				if (myCarSpeed > -speedLimit / 3.0f) myCarSpeed -= speedIncrementer;
			}
			else myCarSpeed = 0.0f;

			if (myEngine->KeyHeld(Key_A) && myCarStatus == Racing)
				move_left(myCamera);
			else if (myEngine->KeyHeld(Key_D) && myCarStatus == Racing)
				move_right(myCamera);
			else
				remove_rotation();

			if (myEngine->KeyHit(Key_Space) && myCarStatus == Hovering)
			{
				GameState = Count_Down;
				gameFont->Draw("", 500, 200, kBlack);
				print_BackDrop_CarSpeed_and_State(SpeedFont);
				myEngine->DrawScene();
				myCarStatus = WaitingForGo;
				int i = 3;
				while (i != 0) {
					gameFont->Draw(to_string(i), 1280 / 2, 200, kRed);
					print_BackDrop_CarSpeed_and_State(SpeedFont);
					myEngine->DrawScene();
					Sleep(1000);	//1 sec pause
					i--;
				}
				gameFont->Draw("Go, go, go!", 550, 200, kRed);
				print_BackDrop_CarSpeed_and_State(SpeedFont);
				myEngine->DrawScene();
				Sleep(1000); //1 sec pause
				GameState = Stage_0_Complete;

				myCamera->SetPosition(0.0f, 20.0f, -60.0f);
			}

			if (myEngine->KeyHit(Key_1))
			{
				myCamera->ResetOrientation();
				myCamera->SetPosition(myCar->GetX(), myCar->GetY() + 3.0f, myCar->GetZ() + 3.0f);
			}
			if (myEngine->KeyHit(Key_C))
			{
				myCamera->ResetOrientation();
				myCamera->SetPosition(myCar->GetX(), 20.0f, myCar->GetZ() - 40.0f);
			}
			if (myEngine->KeyHit(Key_P))
				gamePaused = true;

			if (myEngine->KeyHit(Key_3))
				myCamera->SetPosition(myCar->GetX(), 20.0f, myCar->GetZ() - 40.0f);
			if (myEngine->KeyHit(Key_4))
				myCamera->SetPosition(-30.0f, 80.0f, -180.0f);
			if (myEngine->KeyHit(Key_2))
				myCamera->SetPosition(-30.0f, 80.0f, -180.0f);
			if (myEngine->KeyHit(Key_Up) && myCamera->GetZ() <= cameraForwardsLimit)
				myCamera->MoveZ(cameraSpeed);
			if (myEngine->KeyHit(Key_Down) && myCamera->GetZ() >= -cameraBackwardsLimit)
				myCamera->MoveZ(-cameraSpeed);
			if (myEngine->KeyHit(Key_Left) && myCamera->GetX() >= cameraLeftLimit)
				myCamera->MoveX(-cameraSpeed);
			if (myEngine->KeyHit(Key_Right) && myCamera->GetX() <= cameraRightLimit)
				myCamera->MoveX(cameraSpeed);
		}

		if (gamePaused && myEngine->KeyHit(Key_P))
			gamePaused = false;		//Game Pause/Resume

		if (GameState == Race_Complete)
			gamePaused = true; // End the game
		if (myEngine->KeyHit(Key_Escape))
			myEngine->Stop();	     // Quit the game
	}
	myEngine->Delete();
}