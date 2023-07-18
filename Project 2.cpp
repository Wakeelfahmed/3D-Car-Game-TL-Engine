#include <TL-Engine.h>	// TL-Engine include file and namespace
using namespace tle;	// T-L Engine definitions
#include<windows.h>
enum currentState { Demo, Count_Down, Stage_0_Complete, Stage_1_Complete, Stage_2, Race_Complete }; // Game states										 
enum carState { Hovering, WaitingForGo, Racing };
currentState GameState = Demo;
const int CheckPoint_count = 2, isle_count = 4;

IModel* myCar, * Track_isles[isle_count], * Check_Points[CheckPoint_count]; // Game Models/Assets
ISprite* GameState_BackdropSprite;	//Background sprite (used for Game State)
int myCarStatus = 0, currentGameStage = 0;
bool gamePaused, displayStage = false, collision = false;
float frameRate, myCarSpeed = 0.0f, rotationDegrees = 0.2f / 10.0f, speedLimit = 0.3f, speedIncrementer = 0.001f, rotationAmt = 0.0f, hoverSpeed = 2.0f;
const float maxRotation = 0.2f, steerAmt = 20.0f, carGround = 7.5f;

const float isle_Width = 2.68f, isle_Length = 3.42f;
float isles_minX[isle_count], isles_maxX[isle_count], isles_minZ[isle_count], isles_maxZ[isle_count];

const float checkpoint_pLength = 9.86f, cpWidth = 1.28f;
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
	gameFont->Draw("Speed: " + std::to_string(myCarSpeed * 10).substr(0, 5), 10, 670, kMagenta);
	if (gamePaused) {
		GameState_BackdropSprite->SetX(1040);
		gameFont->Draw("State: Paused", 1050, 670, kBlue);
	}
	else if (GameState == Demo) {
		GameState_BackdropSprite->SetX(1050);
		gameFont->Draw("State: Demo", 1075, 670, kBlue);
	}
	else if (GameState == Count_Down) {
		GameState_BackdropSprite->SetX(980);
		gameFont->Draw("State: Count Down", 990, 670, kBlue);
	}
	else if (GameState == Stage_0_Complete) {
		GameState_BackdropSprite->SetX(920);
		gameFont->Draw("State: Stage 0 Complete", 930, 670, kBlue);
	}
	else if (GameState == Stage_1_Complete) {
		GameState_BackdropSprite->SetX(920);
		gameFont->Draw("State: Stage 1 Complete", 930, 670, kBlue);
	}
	else if (GameState == Race_Complete) {
		GameState_BackdropSprite->SetX(930);
		gameFont->Draw("State: Race Complete", 945, 670, kBlue);
	}
}
void stop_car()
{
	if (myCarSpeed > 0.0f)	myCarSpeed = -myCarSpeed / 3.5f;
	else	myCarSpeed -= myCarSpeed * 1.5f;
}
void detect_collision(float objMinX, float objMaxX, float objMinZ, float objMaxZ)
{
	collision = (myCar->GetLocalX() < objMaxX && myCar->GetLocalX() > objMinX && myCar->GetLocalZ() < objMaxZ && myCar->GetLocalZ() > objMinZ);
	if (collision)
		stop_car();
}
void move_left(ICamera* myCamera)
{
	if (rotationAmt >= -maxRotation) rotationAmt -= 0.4f * frameRate;
	myCar->RotateLocalY(-steerAmt * frameRate);
}
void move_right(ICamera* myCamera)
{
	if (rotationAmt < maxRotation) rotationAmt += 0.4f * frameRate;
	myCar->RotateLocalY(steerAmt * frameRate);
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
		myCar->MoveLocalZ(myCarSpeed);
		angle = myCarSpeed / rotationDegrees;
		bank = rotationAmt / rotationDegrees;
		myCar->ResetOrientation();
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
	myEngine->AddMediaFolder("./media");	//Media Folder (all Meshes, fonts here)

	ISprite* backdropSprite = myEngine->CreateSprite("ui_backdrop.jpg", -450.0f, 660.0f, 0.0f);	//Background sprite (used for Speed o Meter)
	GameState_BackdropSprite = myEngine->CreateSprite("ui_backdrop.jpg", 950.0f, 660.0f, 0.0f);	//Background sprite (used for Game State)

	IFont* gameFont = myEngine->LoadFont("Monotype Corsiva", 45);	//Font
	IFont* SpeedFont = myEngine->LoadFont("Monotype Corsiva", 45);	//Font

	IMesh* groundMesh = myEngine->LoadMesh("ground.x");		IModel* ground = groundMesh->CreateModel(0.0f, 0.0f, 0.0f);
	IMesh* skyboxMesh = myEngine->LoadMesh("skybox 07.x");	IModel* skybox = skyboxMesh->CreateModel(0, -960, 0);

	IMesh* checkPointMesh = myEngine->LoadMesh("checkpoint.x");	Check_Points[0] = checkPointMesh->CreateModel(0, 0, 0);	Check_Points[1] = checkPointMesh->CreateModel(0, 0, 150);
	for (int i = 0; i < CheckPoint_count; i++)
	{
		Check_Points[i]->ScaleY(2.0f);
		cp_through_minX[i] = Check_Points[i]->GetX() - checkpoint_pLength;
		cp_through_maxX[i] = Check_Points[i]->GetX() + checkpoint_pLength;
		cp_through_minZ[i] = Check_Points[i]->GetZ() - cpWidth;
		cp_through_maxZ[i] = Check_Points[i]->GetZ() + cpWidth;
		cp_left_minX[i] = Check_Points[i]->GetX() - 12.0f;
		cp_left_maxX[i] = Check_Points[i]->GetX() - 7.0f;
		cp_left_minZ[i] = Check_Points[i]->GetZ() - 10.0f;
		cp_left_maxZ[i] = Check_Points[i]->GetZ() + 5.0f;
		cp_right_minX[i] = Check_Points[i]->GetX() + 7.0f;
		cp_right_maxX[i] = Check_Points[i]->GetX() + 12.0f;
		cp_right_minZ[i] = Check_Points[i]->GetZ() - 10.0f;
		cp_right_maxZ[i] = Check_Points[i]->GetZ() + 5.0f;
	}

	ICamera* myCamera = myEngine->CreateCamera(kManual, 0.0f, 20.0f, -60.0f); // Create a camera

	IMesh* isleMesh = myEngine->LoadMesh("islestraight.x");
	Track_isles[0] = isleMesh->CreateModel(-10, 0, 40);	Track_isles[1] = isleMesh->CreateModel(10, 0, 40);
	Track_isles[2] = isleMesh->CreateModel(10, 0, 53);	Track_isles[3] = isleMesh->CreateModel(-10, 0, 53);
	for (int i = 0; i < isle_count; i++)
	{
		isles_minX[i] = Track_isles[i]->GetX() - isle_Width;
		isles_maxX[i] = Track_isles[i]->GetX() + isle_Width;
		isles_minZ[i] = Track_isles[i]->GetZ() - isle_Length;
		isles_maxZ[i] = Track_isles[i]->GetZ() + isle_Length;
	}
	IMesh* hoverCarMesh = myEngine->LoadMesh("race2.x");	myCar = hoverCarMesh->CreateModel(0.0f, 2.0f, -20.0f);
	myCamera->AttachToParent(myCar);
	myCar->SetSkin("spengland.jpg");

	while (myEngine->IsRunning())
	{
		frameRate = myEngine->Timer();
		myEngine->DrawScene();
		print_BackDrop_CarSpeed_and_State(SpeedFont);

		if (!gamePaused)
		{
			switch (myCarStatus)
			{
			case Hovering:	//Demo State
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

			for (int loop = 0; loop < CheckPoint_count; loop++)
			{   // Check for collision between car and sides of the check point
				detect_collision(cp_left_minX[loop], cp_left_maxX[loop], cp_left_minZ[loop], cp_left_maxZ[loop]);
				detect_collision(cp_right_minX[loop], cp_right_maxX[loop], cp_right_minZ[loop], cp_right_maxZ[loop]);
				isSatgeCompleted(loop);
				if (1 == currentGameStage)
					GameState = Stage_1_Complete;
				if (2 == currentGameStage)
					GameState = Race_Complete;
			}

			for (int isle = 0; isle < isle_count; isle++)	// Check collision between car and isles
				detect_collision(isles_minX[isle], isles_maxX[isle], isles_minZ[isle], isles_maxZ[isle]);

			if (myEngine->KeyHeld(Key_W))//Forward
			{
				if (myCarSpeed < speedLimit && myCarStatus == Racing) myCarSpeed += speedIncrementer;
			}
			else if (myEngine->KeyHeld(Key_S) && myCarStatus == Racing)	//BackWard
			{
				if (myCarSpeed > -speedLimit / 3.0f) myCarSpeed -= speedIncrementer;
			}
			else myCarSpeed = 0.0f;

			if (myEngine->KeyHeld(Key_A) && myCarStatus == Racing)		//Left
				move_left(myCamera);
			else if (myEngine->KeyHeld(Key_D) && myCarStatus == Racing)	//Right
				move_right(myCamera);
			else
				remove_rotation();

			if (myEngine->KeyHit(Key_Space) && myCarStatus == Hovering)	//SpaceBar
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

			if (myEngine->KeyHit(Key_1))	//First-Person Camera
			{
				myCamera->ResetOrientation();
				myCamera->SetPosition(myCar->GetX(), myCar->GetY() + 3.0f, myCar->GetZ() + 3.0f);
			}
			if (myEngine->KeyHit(Key_C))	//Reset Camera
			{
				myCamera->ResetOrientation();
				myCamera->SetPosition(myCar->GetX(), 20.0f, myCar->GetZ() - 40.0f);
			}
			if (myEngine->KeyHit(Key_P))//Pause
				gamePaused = true;

			if (myEngine->KeyHit(Key_2))	//Free-Moving Camera
				myCamera->SetPosition(-30.0f, 80.0f, -180.0f);
			if (myEngine->KeyHit(Key_3))	//Third-Person Camera
				myCamera->SetPosition(myCar->GetX(), 20.0f, myCar->GetZ() - 40.0f);
			if (myEngine->KeyHit(Key_4))	//Surveillance Camera
				myCamera->SetPosition(-30.0f, 80.0f, -180.0f);
			if (myEngine->KeyHit(Key_Up) && myCamera->GetZ() <= cameraForwardsLimit)	//Camera Forward
				myCamera->MoveZ(cameraSpeed);
			if (myEngine->KeyHit(Key_Down) && myCamera->GetZ() >= -cameraBackwardsLimit)//Camera Backward
				myCamera->MoveZ(-cameraSpeed);
			if (myEngine->KeyHit(Key_Left) && myCamera->GetX() >= cameraLeftLimit)		//Camera left
				myCamera->MoveX(-cameraSpeed);
			if (myEngine->KeyHit(Key_Right) && myCamera->GetX() <= cameraRightLimit)	//Camera right
				myCamera->MoveX(cameraSpeed);
		}

		if (gamePaused && myEngine->KeyHit(Key_P))
			gamePaused = false;	//Game Pause/Resume

		if (myEngine->KeyHit(Key_Escape))
			myEngine->Stop();	// Quit the game
	}
	myEngine->Delete();
}
