// Header Files
//=============

#include "cMyGame.h"
#include "../../Engine/Graphics/Graphics.h"
#include "../../Engine/UserInput/UserInput.h"
#include "../../Engine/Time/Time.h"
#include "../../Engine/Math/Functions.h"
#include "../../Engine/UserSettings/UserSettings.h"
#include "../../Engine/Physics/Physics.h"
#include "../../Engine/Physics/Octree.h"
#include <functional>
#include "../../Engine/Networking/Networking.h"
#include "../../Engine/Audio/Audio.h"

// Interface
//==========

// Initialization / Clean Up
//--------------------------

eae6320::cMyGame::~cMyGame()
{

}

namespace {
	std::vector<eae6320::Game::cPlayer*> s_players;
	void CreatePlayer(eae6320::Networking::eSession i_session, bool i_myPlayer);
}


// Inherited Implementation
//=========================

// Initialization / Clean Up
//--------------------------

bool eae6320::cMyGame::Initialize()
{
	float filedOfView = Math::ConvertDegreesToRadians(60.0f);
	float aspectRatio = ((float) eae6320::UserSettings::GetResolutionWidth()) / eae6320::UserSettings::GetResolutionHeight();
	
	Physics::Load("data/collisiondata/scene.cdata");
	flyCamera = Graphics::Camera(Math::cVector(0.0f, 0.0f, 10.0f), Math::cVector(), filedOfView, 0.1f, 10000.0f, aspectRatio);
	ceilingGameObject.Initialize(Math::cVector(), Math::cVector(), "data/meshes/ceiling.mesh", "data/materials/ceiling.material");
	floorGameObject.Initialize(Math::cVector(), Math::cVector(), "data/meshes/floor.mesh", "data/materials/floor.material");
	metalGameObject.Initialize(Math::cVector(), Math::cVector(), "data/meshes/metal.mesh", "data/materials/metal.material");
	propsGameObject.Initialize(Math::cVector(), Math::cVector(), "data/meshes/props.mesh", "data/materials/props.material");
	railingGameObject.Initialize(Math::cVector(), Math::cVector(), "data/meshes/railing.mesh", "data/materials/railing.material");
	wallsGameObject.Initialize(Math::cVector(), Math::cVector(), "data/meshes/walls.mesh", "data/materials/walls.material");
	worldObjects = { ceilingGameObject, floorGameObject, metalGameObject, propsGameObject, railingGameObject, wallsGameObject };
	Physics::Octree::Load("data/scene.octree");
	createDebugShapes();

	myScoreText.text = new Graphics::cText("My Score: ", -600, 350);
	myScoreText.material.Load("data/materials/sprite.material");

	opponentScoreText.text = new Graphics::cText("Opponent Score: ", 600, 350);
	opponentScoreText.material.Load("data/materials/sprite.material");

	stamina.text = new Graphics::cText("Stamina: ", -600, 325);
	stamina.material.Load("data/materials/sprite.material");

	Physics::Octree::Compute();

	std::function<void(eae6320::Networking::eSession, bool)> player_create = CreatePlayer;
	eae6320::Networking::Load("data\\NetworkSession.lua", player_create);
	Audio::Initialize();
	Audio::PlayMusic("data/sounds/starwarstheme.wav");

	Audio::ModifyMusicVolume(0.2f);
	Audio::ModifyEffectVolume(1.0f);

	Audio::PlayEffect("data/sounds/capturetheenemyflag.wav");
	return true;
}


void eae6320::cMyGame::createDebugShapes() {
#ifdef _DEBUG
	debugSphere.initializeSphereDebugObject(Math::cVector(250.0f, -185.0f, 1200.0f), Math::cVector(), 50, 20, 20, 0, 255, 0, 1);
	debugSphere2.initializeSphereDebugObject(Math::cVector(250.0f, -185.0f,-1200.0f), Math::cVector(), 50, 20, 20, 255, 0, 0, 1);

	Graphics::cText::LoadFontData("data/fontdata.txt");
	Graphics::cText::Initialize();

	fpsText.text = new Graphics::cText("FPS", -600, 350);
	fpsText.material.Load("data/materials/sprite.material");

	arrowText.text = new Graphics::cText(">", -580, 300);
	arrowText.material.Load("data/materials/sprite.material");

	checkBox = new Graphics::Checkbox("Checkbox [*]", -550, 325);

	sliderText.text = new Graphics::cText("Radius <------->", -550, 300);
	sliderText.material.Load("data/materials/sprite.material");
	
	sliderIndexText.text = new Graphics::cText("|", -390, 300);
	sliderIndexText.material.Load("data/materials/sprite.material");
	
	resetText.text = new Graphics::cText("Reset Radius 'R'", -550, 275);
	resetText.material.Load("data/materials/sprite.material");

	musicVolumeSliderText.text = new Graphics::cText("Music Volume <--------->", -550, 300);
	musicVolumeSliderText.material.Load("data/materials/sprite.material");
	musicVolumeSliderIndexText.text = new Graphics::cText("|", -230, 300);
	musicVolumeSliderIndexText.material.Load("data/materials/sprite.material");

	effectVolumeSliderText.text = new Graphics::cText("Effect Volume <--------->", -550 , 275);
	effectVolumeSliderText.material.Load("data/materials/sprite.material");
	effectVolumeSliderIndexText.text = new Graphics::cText("|", -92, 275);
	effectVolumeSliderIndexText.material.Load("data/materials/sprite.material");
#endif
}


bool eae6320::cMyGame::CleanUp()
{
	Networking::CleanUp();
	Audio::CleanUp();
	floorGameObject.cleanUp();
	ceilingGameObject.cleanUp();
	metalGameObject.cleanUp();
	propsGameObject.cleanUp();
	railingGameObject.cleanUp();
	wallsGameObject.cleanUp();
	player.CleanUp();
#ifdef _DEBUG
	fpsText.material.CleanUp();
	fpsText.text->CleanUp();
	sliderIndexText.text->CleanUp();
	sliderText.text->CleanUp();
	resetText.text->CleanUp();
	cleanUpDebugShapes();
#endif
	return true;
}

void eae6320::cMyGame::cleanUpDebugShapes()
{
		debugSphere.cleanUp();
		debugSphere2.cleanUp();
		Physics::Octree::CleanUp();
}

void eae6320::cMyGame::SetUpFrame() {
	Networking::Update();
	Audio::Update();
	if (UserInput::IsKeyPressedOnce(VK_OEM_3))
	{
		isDebug = !isDebug;
	}

	if (!isDebug) {
		if (UserInput::IsKeyPressedOnce(VK_LSHIFT))
		{
			enableFlyCam = !enableFlyCam;
		}
		for (auto player : s_players)
		{
			//	Graphics::SetMesh(player->debugCylinder.meshObject);
			Graphics::SetMesh(player->gameObject.meshObject);
			Graphics::SetMesh(player->opponentFlag->meshObject);
			if (enableFlyCam)
			{
				flyCam.Update(flyCamera);
			}
			else
			{
				player->Update();
			}
			if (player->m_myPlayer) {

				std::string myScore = "My Score: " + std::to_string(player->m_score);
				char * msStr = new char[myScore.length() + 1];
				std::strcpy(msStr, myScore.c_str());
				myScoreText.text = new Graphics::cText(msStr, -600, 350);

				std::string myStamina = "Stamina: " + std::to_string((int)(player->m_stamina));
				char * sStr = new char[myStamina.length() + 1];
				std::strcpy(sStr, myStamina.c_str());
				stamina.text = new Graphics::cText(sStr, -600, 325);
			}
			else {
				std::string opponentScore = "Opponent Score: " + std::to_string(player->m_score);
				char * osStr = new char[opponentScore.length() + 1];
				std::strcpy(osStr, opponentScore.c_str());
				opponentScoreText.text = new Graphics::cText(osStr, 200, 350);
			}
			for (auto other_player : s_players) {
				if (player == other_player)
					continue;
				if (eae6320::Math::DistanceSq(player->gameObject.transform.getPosition(), other_player->gameObject.transform.getPosition()) < 1000)
					player->ResetOpponentFlag();
			}
		}
	}
	Graphics::SetMesh(floorGameObject.meshObject);
	Graphics::SetMesh(ceilingGameObject.meshObject);
	Graphics::SetMesh(metalGameObject.meshObject);
	Graphics::SetMesh(railingGameObject.meshObject);
	Graphics::SetMesh(wallsGameObject.meshObject);
	Graphics::SetMesh(propsGameObject.meshObject);
#ifdef _DEBUG
	if (isDebug) {
		std::string str;
		str = "FPS: " + std::to_string(Time::FPS);
		char * cstr = new char[str.length() + 1];
		std::strcpy(cstr, str.c_str());
		fpsText.text = new Graphics::cText(cstr, -600, 350);

		if (UserInput::IsKeyPressedOnce(VK_DOWN))
		{
			optionsIndex = static_cast<DebugOptions>((optionsIndex + 1) % 3);
			arrowText.text = new Graphics::cText(">", -580, 325 - (optionsIndex * 25));
		}
		else if (UserInput::IsKeyPressedOnce(VK_UP))
		{
			optionsIndex = static_cast<DebugOptions>((optionsIndex + 3) % 3);
			arrowText.text = new Graphics::cText(">", -580, 325 - (optionsIndex * 25));
		}

		switch (optionsIndex)
		{
		case eae6320::cMyGame::CHECKBOX:
			if (UserInput::IsKeyPressedOnce(VK_RIGHT))
			{
				checkBox = new Graphics::Checkbox("Checkbox [*]", -550, 325);
				checkBox->checked = true;
			}
			else if (UserInput::IsKeyPressedOnce(VK_LEFT))
			{
				checkBox = new Graphics::Checkbox("Checkbox [ ]", -550, 325);
				checkBox->checked = false;
			}
			break;
		case eae6320::cMyGame::SLIDER:
			if (UserInput::IsKeyPressedOnce(VK_RIGHT))
			{
				sliderIndex++;
				sliderIndex %= sliderMax;
				sliderIndexText.text = new Graphics::cText("|", -390 + (sliderIndex * 30), 300);
				debugSphere2.initializeSphereDebugObject(Math::cVector(-100.0f, -150.0f, -300.0f), Math::cVector(), sphereRadius * ((sliderIndex + 1) / (float)sliderMax), 20, 20, 0, 0, 255, 1);
			}
			else if (UserInput::IsKeyPressedOnce(VK_LEFT))
			{
				sliderIndex += sliderMax - 1;
				sliderIndex %= sliderMax;
				sliderIndexText.text = new Graphics::cText("|", -390 + (sliderIndex * 30), 300);
				Graphics::Mesh* debugSphere2Mesh = new Graphics::Mesh();
				debugSphere2.initializeSphereDebugObject(Math::cVector(-100.0f, -150.0f, -300.0f), Math::cVector(), sphereRadius * ((sliderIndex + 1) / (float)sliderMax), 20, 20, 0, 0, 255, 1);
			}
			break;
		case eae6320::cMyGame::RESET:
			if (UserInput::IsKeyPressedOnce(82))
			{
				sliderIndex = 0;
				sliderIndexText.text = new Graphics::cText("|", -390 + (sliderIndex * 30), 300);
				Graphics::Mesh* debugSphere2Mesh = new Graphics::Mesh();
				debugSphere2.initializeSphereDebugObject(Math::cVector(-100.0f, -150.0f, -300.0f), Math::cVector(), sphereRadius * ((sliderIndex + 1) / (float)sliderMax), 20, 20, 0, 0, 255, 1);
			}
			break;
		default:
			break;
		}

		Graphics::AddUIText(fpsText);
		Graphics::AddUIText(checkBox->Text);
		Graphics::AddUIText(sliderText);
		Graphics::AddUIText(sliderIndexText);
		Graphics::AddUIText(resetText);
		Graphics::AddUIText(arrowText);
	}
	else {
		Graphics::AddUIText(myScoreText);
		Graphics::AddUIText(opponentScoreText);
		Graphics::AddUIText(stamina);

		if (UserInput::IsKeyPressedOnce(VK_DOWN))
		{
			vOpsIndex = static_cast<VolumeOptions>((vOpsIndex + 1) % 2);
			arrowText.text = new Graphics::cText(">", -580, 300 - (vOpsIndex * 25));
		}
		else if (UserInput::IsKeyPressedOnce(VK_UP))
		{
			vOpsIndex = static_cast<VolumeOptions>((vOpsIndex + 1) % 2);
			arrowText.text = new Graphics::cText(">", -580, 300 - (vOpsIndex * 25));
		}
		switch (vOpsIndex)
		{
		case MUSIC:
			if (UserInput::IsKeyPressedOnce(VK_RIGHT))
			{
				if (musicSliderIndex < sliderMax) {
					musicSliderIndex++;
					musicVolumeSliderIndexText.text = new Graphics::cText("|", -260 + (musicSliderIndex * 30), 300);
					Audio::ModifyMusicVolume(0.2f);
				}
			}
			else if (UserInput::IsKeyPressedOnce(VK_LEFT))
			{
				if (musicSliderIndex > 0) 
				{
					musicSliderIndex--;
					musicVolumeSliderIndexText.text = new Graphics::cText("|", -260 + (musicSliderIndex * 30), 300);
					Audio::ModifyMusicVolume(-0.2f);
				}
			}
			break;
		case EFFECT:
			if (UserInput::IsKeyPressedOnce(VK_RIGHT))
			{
				if (effectSliderIndex < sliderMax) {
					effectSliderIndex++;
					effectVolumeSliderIndexText.text = new Graphics::cText("|", -242 + (effectSliderIndex * 30), 275);
					Audio::ModifyEffectVolume(0.2f);
				}
			}
			else if (UserInput::IsKeyPressedOnce(VK_LEFT))
			{
				if (effectSliderIndex > 0) {
					effectSliderIndex--;
					effectVolumeSliderIndexText.text = new Graphics::cText("|", -242 + (effectSliderIndex * 30), 275);
					Audio::ModifyEffectVolume(-0.2f);
				}
			}
			break;
		}
		Graphics::AddUIText(arrowText);
		Graphics::AddUIText(musicVolumeSliderText);
		Graphics::AddUIText(musicVolumeSliderIndexText);
		Graphics::AddUIText(effectVolumeSliderText);
		Graphics::AddUIText(effectVolumeSliderIndexText);
	}
	setDebugShapes();
#endif // _DEBUG
}

void eae6320::cMyGame::setDebugShapes() {
	if (checkBox->checked)
		Graphics::SetMesh(debugSphere.meshObject);
	Graphics::SetMesh(debugSphere2.meshObject);
//	Graphics::SetMesh(debugLine2.meshObject);
	Physics::Octree::displayOctree();
}

void eae6320::cMyGame::SetUpCamera() {
}

namespace 
{
	void CreatePlayer(eae6320::Networking::eSession i_session, bool i_myPlayer) 
	{
		eae6320::Game::cPlayer* player = new eae6320::Game::cPlayer;
		player->Initialize(i_session, i_myPlayer);
		s_players.push_back(player);
	}
}