#include "cPlayer.h"
#include "../../Engine/Math/Functions.h"
#include "../../Engine/Networking/Networking.h"
#include "../../Engine/UserInput/UserInput.h"
#include "../../Engine/Audio/Audio.h"

namespace {
	void RemotePlayerUpdate(eae6320::Networking::sPlayerData* i_remoteplayer);
	eae6320::Networking::sPlayerData* main_player;
	eae6320::Networking::sPlayerData* remote_player;
	const float MAX_STAMINA = 100;
	const eae6320::Math::cVector redFlagDefaultPos = eae6320::Math::cVector(250.0f, -185.0f, -1200.0f);
	const eae6320::Math::cVector redFlagWorldPos = eae6320::Math::cVector(250.0f, -185.0f, 1200.0f);
	 eae6320::Math::cVector blueflagDefaultPos = eae6320::Math::cVector(250.0f, -185.0f, 1200.0f);
	const eae6320::Math::cVector blueflagWorldPos = eae6320::Math::cVector(250.0f, -185.0f,-1200.0f);
	void UpdateOpponentFlagPosition(eae6320::Game::cPlayer* player);
	bool isSoundPlaying = false;
}

bool eae6320::Game::cPlayer::Initialize(eae6320::Networking::eSession i_sessionType, bool i_myPlayer)
{
	float filedOfView = Math::ConvertDegreesToRadians(60.0f);
	float aspectRatio = ((float)eae6320::UserSettings::GetResolutionWidth()) / eae6320::UserSettings::GetResolutionHeight();
	camera = Graphics::Camera(Math::cVector(0.0f, 150.0f, 0.0f), Math::cVector(), filedOfView, 0.1f, 10000.0f, aspectRatio);
	m_myPlayer = i_myPlayer;
	m_session = i_sessionType;
	opponentFlag = new Graphics::GameObject();

	if (i_sessionType == Networking::eSession::SERVER) {
		gameObject.Initialize(Math::cVector(250.0f, -185.0f, 1200.0f), Math::cVector(0, 180, 0), "data/meshes/player.mesh", "data/materials/blue.material");
		opponentFlag->Initialize(redFlagDefaultPos, Math::cVector(), "data/meshes/flag.mesh", "data/materials/red.material");
	}
	else
	{
		gameObject.Initialize(Math::cVector(250.0f, -185.0f, -1200.0f), Math::cVector(0, 180, 0), "data/meshes/player.mesh", "data/materials/red.material");
		opponentFlag->Initialize(blueflagDefaultPos, Math::cVector(), "data/meshes/flag.mesh", "data/materials/blue.material");
	}
	if (!m_myPlayer)
	{
		std::function<void(eae6320::Networking::sPlayerData*)> callback = RemotePlayerUpdate;
		eae6320::Networking::UpdateRemotePlayerData(callback);
	}
#ifdef _DEBUG

	if (i_sessionType == Networking::eSession::SERVER) {
	//	debugCylinder.initializeCylinderDebugObject(Math::cVector(500.0f, 10.0f, -1000.0f), Math::cVector(), 30, 30, 100, 20, 20, 0, 255, 0, 1);
	}
	else {
	//	debugCylinder.initializeCylinderDebugObject(Math::cVector(500.0f, 10.0f, -1000.0f), Math::cVector(), 30, 30, 100, 20, 20, 255, 0, 0, 1);
	}
	debugLine.initializeLineDebugObject(Math::cVector(0.0f, 0.0f, 0.0f), Math::cVector(), gameObject.transform.getPosition(), gameObject.transform.getPosition() + (eae6320::Math::cVector(gameObject.rigidBody.velocity.x, 0, gameObject.rigidBody.velocity.z)), 1, 1, 1, 1);
	//debugLine2.initializeLineDebugObject(Math::cVector(10.0f, 10.0f, 10.0f), Math::cVector(), Math::cVector(0, 0, -900), Math::cVector(100, 100, -1000), 0, 0, 0, 1);
#endif
	main_player = new Networking::sPlayerData;
	remote_player = new Networking::sPlayerData;
	return true;
}

void eae6320::Game::cPlayer::Update()
{
	if (!m_myPlayer)
	{
		gameObject.transform.Move(remote_player->m_position);
		if(!isSoundPlaying && remote_player->m_speed>100)
		{
			if (gameObject.transform.getPosition().y > 0.0f) 
			{
				Audio::AddStaticEmitter(gameObject.transform, "data/sounds/running-hard-surface.wav", &isSoundPlaying);
			} 
			else
			{
				Audio::AddStaticEmitter(gameObject.transform, "data/sounds/running-gravel-or-dry-leaves-loop.wav", &isSoundPlaying);
			}
		}
		gameObject.transform.Rotate(remote_player->m_rotation);
		gameObject.meshObject.position = gameObject.transform.getPosition();
		gameObject.meshObject.rotation = gameObject.transform.getOrientation();

		if (remote_player->m_hasFalg&&!m_hasFlag)
			Audio::PlayEffect("data/sounds/theenemyhastakenyourflag.wav");

		m_hasFlag = remote_player->m_hasFalg;
		if (m_hasFlag)
		{
			opponentFlag->meshObject.position = gameObject.transform.getPosition();
		}
		else
		{
			ResetOpponentFlag();
		}

		if (remote_player->m_score > m_score) {
			Audio::PlayEffect("data/sounds/enemyhascapturedyourflag.wav");
			m_score = remote_player->m_score;
		}

		return;
	}
	UpdateOpponentFlagPosition(this);
	if (m_hasFlag)
	{
		opponentFlag->meshObject.position = gameObject.transform.getPosition();
		UpdateScore();
	}
	UpdateStamina();
	controller.Update(gameObject, camera);
	controller.UpdateCamera(camera, gameObject);
	Audio::UpdateListener(camera.transform);

	if (gameObject.rigidBody.velocity.GetLength() > 100)
	{
		if (gameObject.transform.getPosition().y > 0.0f)
			Audio::AddStaticEmitter(gameObject.transform, "data/sounds/running-hard-surface.wav", &isSoundPlaying);
		else
			Audio::AddStaticEmitter(gameObject.transform, "data/sounds/running-gravel-or-dry-leaves-loop.wav", &isSoundPlaying);
	}

	main_player->m_position = gameObject.transform.getPosition();
	main_player->m_rotation = gameObject.transform.getRotation();
	main_player->m_hasFalg = m_hasFlag;
	main_player->m_score = m_score;
	main_player->m_speed = gameObject.rigidBody.velocity.GetLength();

//	debugCylinder.Move(gameObject.transform.getPosition());
//	debugCylinder.Rotate(gameObject.transform.getRotation());
	Networking::SubmitMainPlayerData(main_player);
	if (eae6320::Math::cVector(gameObject.rigidBody.velocity.x, 0, gameObject.rigidBody.velocity.z).GetLength() > 1)
	{
		debugLine.updateLine(gameObject.transform.getPosition(), gameObject.rigidBody.toVelocityPoint, 255, 255, 255, 1);
		Graphics::SetMesh(debugLine.meshObject);
	}
	//debugLine2.updateLine(player.transform.getPosition(), player.rigidBody.toFloorPoint, 255, 255, 255, 1);
}


bool eae6320::Game::cPlayer::CleanUp()
{
	debugLine.cleanUp();
	//debugLine2.cleanUp();
//	debugCylinder.cleanUp();
	return true;
}
void eae6320::Game::cPlayer::UpdateStamina()
{
	if (UserInput::IsKeyPressed('Q'))
	{
		m_stamina += -0.1f;
		controller.MAXSPEED = 600;
	}
	else
	{
		controller.MAXSPEED = 200;
		if (m_stamina < MAX_STAMINA)
		{
			m_stamina += 0.02f;
		}
	}

	if (m_stamina > MAX_STAMINA)
		m_stamina = MAX_STAMINA;

	if (m_stamina <= 0)
	{
		m_stamina = 0;
		controller.MAXSPEED = 200;
	}
}
void eae6320::Game::cPlayer::UpdateScore()
{
	if (!m_myPlayer)
		return;
	eae6320::Math::cVector destination;
	if (m_session == eae6320::Networking::CLIENT) {
		destination = blueflagWorldPos;
	}
	else {
		destination = redFlagWorldPos;
	}
	if (eae6320::Math::DistanceSq(destination, gameObject.transform.getPosition()) < 2000) {
		++m_score;
		Audio::PlayEffect("data/sounds/victory.wav");
		ResetOpponentFlag();
	}
}

void eae6320::Game::cPlayer::ResetOpponentFlag()
{
	m_hasFlag = false;
	if (m_session == eae6320::Networking::eSession::CLIENT) 
	{
		opponentFlag->meshObject.position = blueflagDefaultPos;
	} 
	else
	{
		opponentFlag->meshObject.position = redFlagDefaultPos;
	}
}

namespace 
{
	void RemotePlayerUpdate(eae6320::Networking::sPlayerData* i_remoteplayer)
	{
		remote_player->m_position = i_remoteplayer->m_position;
		remote_player->m_rotation = i_remoteplayer->m_rotation;
		remote_player->m_hasFalg = i_remoteplayer->m_hasFalg;
		remote_player->m_score = i_remoteplayer->m_score;
		remote_player->m_speed = i_remoteplayer->m_speed;
	}

	void UpdateOpponentFlagPosition(eae6320::Game::cPlayer* player)
	{
		if (!player->m_myPlayer)
			return;
		eae6320::Math::cVector default_world_pos;
		if (player->m_session == eae6320::Networking::CLIENT)
		{
			default_world_pos = blueflagDefaultPos;
		}
		else {
			default_world_pos = redFlagDefaultPos;
		}

		if (eae6320::Math::DistanceSq(player->gameObject.transform.getPosition(), default_world_pos) < 1000) {
			if (!player->m_hasFlag)
			{
				eae6320::Audio::PlayEffect("youhavetheflag.wav");
			}
			player->m_hasFlag = true;
		}
	}
}