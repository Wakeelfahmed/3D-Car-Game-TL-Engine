#include <TL-Engine.h>
using namespace tle;
#include <windows.h>

enum class CurrentState { Demo, Count_Down, Stage_0_Complete, Stage_1_Complete, Stage_2, Race_Complete }; // Game states										 
enum class CarState { Hovering, WaitingForGo, Racing };// Define car states

class HoverCarRacingGame
{
private:
	I3DEngine* engine;
	IFont* gameFont, * speedFont;
	ISprite* backdropSprite, * gameStateBackdropSprite;	// Background sprite for Game State & Speedometer
	ICamera* camera;
	IMesh* groundMesh, * skyboxMesh;
	IMesh* checkPointMesh, * isleMesh, * hoverCarMesh;
	IModel* skybox, * ground, * checkPoints[2], * trackIsles[4], * myCar;	// Game models/assets
	CurrentState gameState;
	CarState carState;
	int currentGameStage;	//Stage Counter
	bool gamePaused, displayStage, collision;
	float frameRate, myCarSpeed, rotationDegrees, speedLimit, speedIncrementer, rotationAmt, hoverSpeed, maxRotation, steerAmt, carGround, isleWidth, isleLength, checkpointPLength, cpWidth, cameraForwardsLimit, cameraBackwardsLimit, cameraLeftLimit, cameraRightLimit, cameraSpeed, islesMinX[4], islesMaxX[4], islesMinZ[4], islesMaxZ[4], cpLeftMinX[2], cpLeftMaxX[2], cpLeftMinZ[2], cpLeftMaxZ[2], cpRightMinX[2], cpRightMaxX[2], cpRightMinZ[2], cpRightMaxZ[2], cpThroughMinX[2], cpThroughMaxX[2], cpThroughMinZ[2], cpThroughMaxZ[2];
	// Constants for isles
	// Constants for checkpoints
	// Camera constants
public:
	HoverCarRacingGame() : gameState(CurrentState::Demo), carState(CarState::Hovering), currentGameStage(0), gamePaused(false),
		displayStage(false), collision(false), frameRate(0.0f), myCarSpeed(0.0f), rotationDegrees(0.2f / 10.0f), speedLimit(0.3f),
		speedIncrementer(0.001f), rotationAmt(0.0f), hoverSpeed(2.0f), maxRotation(0.2f), steerAmt(20.0f), carGround(7.5f),
		isleWidth(2.68f), isleLength(3.42f), checkpointPLength(9.86f), cpWidth(1.28f), cameraForwardsLimit(620.0f),
		cameraBackwardsLimit(-140.0f), cameraLeftLimit(0.0f), cameraRightLimit(420.0f), cameraSpeed(20.0f)
	{
		engine = New3DEngine(kTLX);
		engine->StartWindowed();
		engine->SetWindowCaption("Hover Car Racing");
		engine->AddMediaFolder("./media");
		backdropSprite = engine->CreateSprite("ui_backdrop.jpg", -450.0f, 660.0f, 0.0f);
		gameStateBackdropSprite = engine->CreateSprite("ui_backdrop.jpg", 950.0f, 660.0f, 0.0f);
		gameFont = engine->LoadFont("Monotype Corsiva", 45);
		speedFont = engine->LoadFont("Monotype Corsiva", 45);
		groundMesh = engine->LoadMesh("ground.x");
		ground = groundMesh->CreateModel(0.0f, 0.0f, 0.0f);
		skyboxMesh = engine->LoadMesh("skybox 07.x");
		skybox = skyboxMesh->CreateModel(0, -960, 0);
		checkPointMesh = engine->LoadMesh("checkpoint.x");
		checkPoints[0] = checkPointMesh->CreateModel(0, 0, 0);
		checkPoints[1] = checkPointMesh->CreateModel(0, 0, 150);
		for (int i = 0; i < 2; i++)
		{
			checkPoints[i]->ScaleY(2.0f);
			cpThroughMinX[i] = checkPoints[i]->GetX() - checkpointPLength;
			cpThroughMaxX[i] = checkPoints[i]->GetX() + checkpointPLength;
			cpThroughMinZ[i] = checkPoints[i]->GetZ() - cpWidth;
			cpThroughMaxZ[i] = checkPoints[i]->GetZ() + cpWidth;
			cpLeftMinX[i] = checkPoints[i]->GetX() - 12.0f;
			cpLeftMaxX[i] = checkPoints[i]->GetX() - 7.0f;
			cpLeftMinZ[i] = checkPoints[i]->GetZ() - 10.0f;
			cpLeftMaxZ[i] = checkPoints[i]->GetZ() + 5.0f;
			cpRightMinX[i] = checkPoints[i]->GetX() + 7.0f;
			cpRightMaxX[i] = checkPoints[i]->GetX() + 12.0f;
			cpRightMinZ[i] = checkPoints[i]->GetZ() - 10.0f;
			cpRightMaxZ[i] = checkPoints[i]->GetZ() + 5.0f;
		}
		isleMesh = engine->LoadMesh("islestraight.x");
		trackIsles[0] = isleMesh->CreateModel(-10, 0, 40);
		trackIsles[1] = isleMesh->CreateModel(10, 0, 40);
		trackIsles[2] = isleMesh->CreateModel(10, 0, 53);
		trackIsles[3] = isleMesh->CreateModel(-10, 0, 53);
		for (int i = 0; i < 4; i++)
		{
			islesMinX[i] = trackIsles[i]->GetX() - isleWidth;
			islesMaxX[i] = trackIsles[i]->GetX() + isleWidth;
			islesMinZ[i] = trackIsles[i]->GetZ() - isleLength;
			islesMaxZ[i] = trackIsles[i]->GetZ() + isleLength;
		}
		hoverCarMesh = engine->LoadMesh("race2.x");
		myCar = hoverCarMesh->CreateModel(0.0f, 2.0f, -20.0f);
		camera = engine->CreateCamera(kManual, 0.0f, 20.0f, -60.0f);
		camera->AttachToParent(myCar);
		myCar->SetSkin("spengland.jpg");
	}
	~HoverCarRacingGame()
	{
		engine->Delete();
	}
	void isStageCompleted(int currentCP)
	{
		if (myCar->GetX() >= cpThroughMinX[currentCP] && myCar->GetX() <= cpThroughMaxX[currentCP] &&
			myCar->GetZ() >= cpThroughMinZ[currentCP] && myCar->GetZ() <= cpThroughMaxZ[currentCP] &&
			currentGameStage == currentCP)
		{
			currentGameStage++;
			displayStage = true;
		}
	}
	void printGameCompletedStage()
	{
		if (displayStage)
		{
			if (currentGameStage < 2)
			{
				gameFont->Draw("Stage " + std::to_string(currentGameStage) + " Completed!", 500, 200, kBlack);
			}
			else
			{
				gameFont->Draw("Race Complete!", 510, 200, kBlack);
				myCarSpeed = 0.0f;
			}
		}
	}
	void printBackdropCarSpeedAndState()
	{
		speedFont->Draw("Speed: " + std::to_string(myCarSpeed * 10).substr(0, 5), 10, 670, kMagenta);
		if (gamePaused) {
			gameStateBackdropSprite->SetX(1040);
			speedFont->Draw("State: Paused", 1050, 670, kBlue);
		}
		else if (gameState == CurrentState::Demo) {
			gameStateBackdropSprite->SetX(1050);
			speedFont->Draw("State: Demo", 1075, 670, kBlue);
		}
		else if (gameState == CurrentState::Count_Down) {
			gameStateBackdropSprite->SetX(980);
			speedFont->Draw("State: Count Down", 990, 670, kBlue);
		}
		else if (gameState == CurrentState::Stage_0_Complete) {
			gameStateBackdropSprite->SetX(920);
			speedFont->Draw("State: Stage 0 Complete", 930, 670, kBlue);
		}
		else if (gameState == CurrentState::Stage_1_Complete) {
			gameStateBackdropSprite->SetX(920);
			speedFont->Draw("State: Stage 1 Complete", 930, 670, kBlue);
		}
		else if (gameState == CurrentState::Race_Complete) {
			gameStateBackdropSprite->SetX(930);
			speedFont->Draw("State: Race Complete", 945, 670, kBlue);
		}
	}
	void stopCar()
	{
		if (myCarSpeed > 0.0f) myCarSpeed = -myCarSpeed / 3.5f;
		else myCarSpeed -= myCarSpeed * 1.5f;
	}
	void detectCollision(float objMinX, float objMaxX, float objMinZ, float objMaxZ)
	{
		collision = (myCar->GetLocalX() < objMaxX && myCar->GetLocalX() > objMinX && myCar->GetLocalZ() < objMaxZ && myCar->GetLocalZ() > objMinZ);
		if (collision)
			stopCar();
	}
	void moveLeft()
	{
		if (rotationAmt >= -maxRotation) rotationAmt -= 0.4f * frameRate;
		myCar->RotateLocalY(-steerAmt * frameRate);
	}
	void moveRight()
	{
		if (rotationAmt < maxRotation) rotationAmt += 0.4f * frameRate;
		myCar->RotateLocalY(steerAmt * frameRate);
	}
	void removeRotation()
	{
		if (rotationAmt > 0.0f)  rotationAmt -= 0.4f * frameRate;
		if (rotationAmt < 0.01f)  rotationAmt += 0.4f * frameRate;
	}
	void updateCar()
	{
		float bank, angle;
		if (!collision)
		{
			myCar->MoveLocalZ(myCarSpeed);
			angle = myCarSpeed / rotationDegrees;
			bank = rotationAmt / rotationDegrees;
			myCar->ResetOrientation();
			//myCar->RotateLocalX(-angle); // Cause to fly
			myCar->RotateLocalZ(-bank * 2.0f);
			myCar->RotateLocalY(bank);
		}
	}
	void run()
	{
		while (engine->IsRunning())
		{
			frameRate = engine->Timer();
			engine->DrawScene();
			printBackdropCarSpeedAndState();

			if (!gamePaused)
			{
				switch (carState)
				{
				case CarState::Hovering:
					camera->RotateY(engine->GetMouseMovementX() * 0.1f);
					gameFont->Draw("Press space bar to begin!", 500, 200, kBlack);
					break;
				case CarState::WaitingForGo:
					camera->ResetOrientation();
					carState = CarState::Racing;
					break;
				case CarState::Racing:
					updateCar();
					break;
				default:
					break;
				}
				printGameCompletedStage();

				for (int i = 0; i < 2; i++)
				{
					detectCollision(cpLeftMinX[i], cpLeftMaxX[i], cpLeftMinZ[i], cpLeftMaxZ[i]);
					detectCollision(cpRightMinX[i], cpRightMaxX[i], cpRightMinZ[i], cpRightMaxZ[i]);
					isStageCompleted(i);
					if (1 == currentGameStage)
						gameState = CurrentState::Stage_1_Complete;
					if (2 == currentGameStage)
						gameState = CurrentState::Race_Complete;
				}

				for (int isle = 0; isle < 4; isle++)
					detectCollision(islesMinX[isle], islesMaxX[isle], islesMinZ[isle], islesMaxZ[isle]);

				if (engine->KeyHeld(Key_W))
				{
					if (myCarSpeed < speedLimit && carState == CarState::Racing)
						myCarSpeed += speedIncrementer;
				}
				else if (engine->KeyHeld(Key_S) && carState == CarState::Racing)
				{
					if (myCarSpeed > -speedLimit / 3.0f)
						myCarSpeed -= speedIncrementer;
				}
				else
					myCarSpeed = 0.0f;

				if (engine->KeyHeld(Key_A) && carState == CarState::Racing)
					moveLeft();
				else if (engine->KeyHeld(Key_D) && carState == CarState::Racing)
					moveRight();
				else
					removeRotation();

				if (engine->KeyHit(Key_Space) && carState == CarState::Hovering)
				{
					gameState = CurrentState::Count_Down;
					gameFont->Draw("", 500, 200, kBlack);
					printBackdropCarSpeedAndState();
					engine->DrawScene();
					carState = CarState::WaitingForGo;
					int i = 3;
					while (i != 0)
					{
						gameFont->Draw(std::to_string(i), 1280 / 2, 200, kRed);
						printBackdropCarSpeedAndState();
						engine->DrawScene();
						Sleep(1000); // 1 sec pause
						i--;
					}
					gameFont->Draw("Go, go, go!", 550, 200, kRed);
					printBackdropCarSpeedAndState();
					engine->DrawScene();
					Sleep(1000); // 1 sec pause
					gameState = CurrentState::Stage_0_Complete;

					camera->SetPosition(0.0f, 20.0f, -60.0f);
				}

				if (engine->KeyHit(Key_1))
				{
					camera->ResetOrientation();
					camera->SetPosition(myCar->GetX(), myCar->GetY() + 3.0f, myCar->GetZ() + 3.0f);
				}
				if (engine->KeyHit(Key_C))
				{
					camera->ResetOrientation();
					camera->SetPosition(myCar->GetX(), 20.0f, myCar->GetZ() - 40.0f);
				}
				if (engine->KeyHit(Key_P))
					gamePaused = true;

				if (engine->KeyHit(Key_3))
					camera->SetPosition(myCar->GetX(), 20.0f, myCar->GetZ() - 40.0f);
				if (engine->KeyHit(Key_4))
					camera->SetPosition(-30.0f, 80.0f, -180.0f);
				if (engine->KeyHit(Key_2))
					camera->SetPosition(-30.0f, 80.0f, -180.0f);
				if (engine->KeyHit(Key_Up) && camera->GetZ() <= cameraForwardsLimit)
					camera->MoveZ(cameraSpeed);
				if (engine->KeyHit(Key_Down) && camera->GetZ() >= cameraBackwardsLimit)
					camera->MoveZ(-cameraSpeed);
				if (engine->KeyHit(Key_Left) && camera->GetX() >= cameraLeftLimit)
					camera->MoveX(-cameraSpeed);
				if (engine->KeyHit(Key_Right) && camera->GetX() <= cameraRightLimit)
					camera->MoveX(cameraSpeed);
			}

			if (gamePaused && engine->KeyHit(Key_P))
				gamePaused = false; // Game Pause/Resume

			//if (gameState == CurrentState::Race_Complete)
				//gamePaused = true; // End the game

			if (engine->KeyHit(Key_Escape))
				engine->Stop(); // Quit the game
		}
	}
};

int main()
{
	HoverCarRacingGame game;
	game.run();
	return 0;
}
