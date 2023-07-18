

#include <TL-Engine.h>	// TL-Engine include file and namespace
using namespace tle;


void main()
{
	// Create a 3D engine (using TLX engine here) and open a window for it
	I3DEngine* myEngine = New3DEngine(kTLX);
	myEngine->StartWindowed();

	// Add default folder for meshes and other media
	myEngine->AddMediaFolder("C:\\Program Files\\TL-Engine\\Media");

	ICamera* myCamera;
	// set camera according to 2D camera perspective values
	myCamera = myEngine->CreateCamera(kManual);
	myCamera->SetPosition(0, 200, 0);
	myCamera->RotateLocalX(90);

	// The main game loop, repeat until engine is stopped
	while (myEngine->IsRunning())
	{
		// Draw the scene
		myEngine->DrawScene();



		//An array is used to store the skin codes. The game will move through the array to change the skins of each sphere or cube.
		const int numSkins = 2;
		string skins[numSkins] = { "regularsphere.jpg", "enemysphere.jpg" };

		//Background Floor 
		IMesh* backfloorMesh;
		IModel* backfloor;
		backfloorMesh = myEngine->LoadMesh("water.x");
		backfloor = backfloorMesh->CreateModel(0, -5, 0);
		backfloor->SetSkin("water.jpg");

		//Sky
		IMesh* skyfloorMesh;
		IModel* skyfloor;
		skyfloorMesh = myEngine->LoadMesh("sky.x");
		skyfloor = skyfloorMesh->CreateModel(0, -960, 0);
		skyfloor->SetSkin("Sky.png");

		//Floor 
		IMesh* floorMesh;
		IModel* floor;
		floorMesh = myEngine->LoadMesh("island.x");
		floor = floorMesh->CreateModel(0, -5, 0);
		floor->SetSkin("Sand.png");

		// create fonts to display win, lose and score text
		IFont* defaultFont = myEngine->LoadFont("Comic Sans MS");
		IFont* scoreFont = myEngine->LoadFont("Comic Sans MS");

		//Create Mesh for Sphere and cubes
		IMesh* SphereMesh = myEngine->LoadMesh("spheremesh.x");
		IMesh* EnemyMesh = myEngine->LoadMesh("minicube.x");

		//Create sphere and assign starting position.
		IModel* mySphere;
		mySphere = SphereMesh->CreateModel(0.f, 10.f, 0.f);
		mySphere->SetSkin(skins[0]);

		//Create Enemy cubes and assign starting position.
		const int numEnemies = 12.5;
		IModel* enemy[numEnemies];//Enemy cubes are created using an array.
		// Use current time as seed for random generator
		srand(time(0));
		for (int i = 0; i < numEnemies; i++)
		{
			// generate random x-axis and z-axis values for Enemy cubes and y-axis value is fixed for 2D scene
			int xValue = rand() % 161 - 80;
			int yValue = rand() % 161 - 80;
			enemy[i] = EnemyMesh->CreateModel(xValue, 10.f, yValue);
			enemy[i]->SetSkin(skins[1]);
		}

		const float kSphereSpeed = (0.1f);//Base speed of the spheres.
		//Values used to set the boundaries of x-axis and z-axis movement of the sphere.
		const float minXLimit = (-98.f);//Lower bound of X axis movement.
		const float maxXlimit = (98.f);	//Upper bound of X axis movement.
		const float minZLimit = (-98.f);//Lower bound of Z axis movement.
		const float maxZLimit = (98.f);	//Upper bound of Z axis movement.


		/*Control state of game
		and state of spheres.*/
		bool pause = false;	// variable to display Pause message
		bool bWon = false;	// variable to display game Win message
		bool bLose = false;	// variable to display game Lose message
		int playerPoints = 0;	// variable to store score of player

		//Allow game to check if keys are pressed.
		bool pauseKeyPressed;				//P key, pause game.
		bool upperMovementKeyPressed;		//W key, speed up spheres	.										
		bool downwardMovementKeyPressed;	//S key, slow down spheres.
		bool leftMovementKeyPressed;		//A key, move spheres up.
		bool rightMovementKeyPressed;		//D key, move spheres down.
		bool upCameraMovementKeyPressed;	//Up Arrow key, move the camera upward.
		bool downCameraMovementKeyPressed;	//Down Arrow key, move the camera downward.
		bool rightCameraMovementKeyPressed;	//Right Arrow key, move the camera right.
		bool leftCameraMovementKeyPressed;	//Left Arrow key, move the camera left.
		bool updateCameraKeyPressed;		//2 key, change camera 

		// The main game loop, repeat until engine is stopped
		while (myEngine->IsRunning())
		{
			// Draw the scene
			myEngine->DrawScene();

			/**** Update your scene each frame here ****/
			//Allow game to be quit without using alt+f4.
			if (myEngine->KeyHit(Key_Escape))
			{
				myEngine->Stop();
			}
			/*Variables used to monitor when keys are hit.
			Change bool values to enable/disable function of the keys.*/
			pauseKeyPressed = (myEngine->KeyHit(Key_P));
			upperMovementKeyPressed = (myEngine->KeyHit(Key_W) || myEngine->KeyHeld(Key_W));
			downwardMovementKeyPressed = (myEngine->KeyHit(Key_S) || myEngine->KeyHeld(Key_S));
			rightMovementKeyPressed = (myEngine->KeyHit(Key_D) || myEngine->KeyHeld(Key_D));
			leftMovementKeyPressed = (myEngine->KeyHit(Key_A) || myEngine->KeyHeld(Key_A));
			upCameraMovementKeyPressed = (myEngine->KeyHit(Key_Up) || myEngine->KeyHeld(Key_Up));
			downCameraMovementKeyPressed = (myEngine->KeyHit(Key_Down) || myEngine->KeyHeld(Key_Down));
			rightCameraMovementKeyPressed = (myEngine->KeyHit(Key_Right) || myEngine->KeyHeld(Key_Right));
			leftCameraMovementKeyPressed = (myEngine->KeyHit(Key_Left) || myEngine->KeyHeld(Key_Left));
			updateCameraKeyPressed = myEngine->KeyHit(Key_2);

			//check if player presses pause key, invert pause variable to make game pause.
			if (pauseKeyPressed)
			{
				pause = !pause;
			}

			/*Variables used for changing and limiting movement of sphere.
			Used to find co-ordinates of sphere.
			This data is used to move sphere around circuit */
			float xPosSphere = mySphere->GetX();
			float yPosSphere = mySphere->GetY();
			float zPosSphere = mySphere->GetZ();

			if (!pause)
			{

				//control the camera movement
				//move the camera in positive Z axis or upward
				if (upCameraMovementKeyPressed)
				{
					myCamera->MoveZ(kSphereSpeed);
				}

				//move the camera in negative Z axis or downward
				if (downCameraMovementKeyPressed)
				{
					myCamera->MoveZ(-kSphereSpeed);
				}

				//move the camera in negative X axis or left
				if (leftCameraMovementKeyPressed)
				{
					myCamera->MoveX(-kSphereSpeed);
				}

				//move the camera in positive X axis or right
				if (rightCameraMovementKeyPressed)
				{
					myCamera->MoveX(kSphereSpeed);
				}

				// display score of player user scoreFont created earlier
				scoreFont->Draw("score: " + to_string(playerPoints), (myEngine->GetWidth() / 2 + 580), myEngine->GetHeight() / 2 - 340, kBlack, kCentre, kVCentre);

				// Check if all enemies cubes are finished
				// we move enemy cube to a specific location if eaten by sphere
				bWon = true;
				for (int j = 0; j < numEnemies; j++)
				{
					if ((enemy[j]->GetX()) != -2000)
						bWon = false;
				}

				// check if game is won or lose to limit the movement of sphere
				if (!bWon && !bLose)
				{
					// check if sphere is moving in valid space inside of x-axis and z-axis limits
					if ((xPosSphere >= minXLimit && xPosSphere <= maxXlimit) && (zPosSphere >= minZLimit && zPosSphere <= maxZLimit))
					{
						// following loop is checking is sphere move to any enemy cube space
						// if yes then move the enemy cube to specified position outside the camera 
						// and increase the score of playe
						// and increase the size of sphere
						for (int i = 0; i < numEnemies; i++)
						{
							if ((xPosSphere >= enemy[i]->GetX() - 8) && (xPosSphere <= enemy[i]->GetX() + 8) && (zPosSphere >= enemy[i]->GetZ() - 8) && (zPosSphere <= enemy[i]->GetZ() + 8))
							{
								enemy[i]->SetX(-2000);
								playerPoints += 10;
								//To increase the size in order to scale 1.2 every 4
								mySphere->Scale(1.05f);
							}
						}

						//control the sphere movement
						// move the sphere in positive Z direction or upperward
						if (upperMovementKeyPressed)
						{
							mySphere->MoveZ(kSphereSpeed);

						}

						// move the sphere in negative Z direction or downward
						if (downwardMovementKeyPressed)
						{
							mySphere->MoveZ(-kSphereSpeed);
							mySphere->RotateY(180.f);
						}

						// move the sphere in positive x direction or right
						if (rightMovementKeyPressed)
						{
							mySphere->MoveX(kSphereSpeed);
							mySphere->RotateY(90.f);
						}

						// move the sphere in negative x direction or left
						if (leftMovementKeyPressed)
						{
							mySphere->MoveX(-kSphereSpeed);
							mySphere->RotateY(270.f);
						}
					}
					else
					{
						// if sphere is moving outside of valid space that is outside of x-axis and z-axis limits
						// then declare Game is loss
						if (xPosSphere < minXLimit || xPosSphere > maxXlimit)
							bLose = true;

						if (zPosSphere < minZLimit || zPosSphere > maxZLimit)
							bLose = true;
					}
				}
				else
				{
					// display the message of win or lose respectively
					if (bWon)
					{
						defaultFont->Draw("GameWOn", myEngine->GetWidth() / 2, myEngine->GetHeight() / 2, kBlack, kCentre, kVCentre);
					}
					else if (bLose)
					{
						defaultFont->Draw("GameOver", myEngine->GetWidth() / 2, myEngine->GetHeight() / 2, kRed, kCentre, kVCentre);
					}
				}
			}
			else
			{
				// display paused game message
				defaultFont->Draw("PAUSED", myEngine->GetWidth() / 2, myEngine->GetHeight() / 2, kBlack, kCentre, kVCentre);
			}
		}

		// Delete the 3D engine now we are finished with it
		myEngine->Delete();
	}
}