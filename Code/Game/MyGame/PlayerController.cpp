#include "PlayerController.h"
#include "../../Engine/UserInput/UserInput.h"
#include "../../Engine/Time/Time.h"
#include "../../Engine/Physics/Physics.h"


void eae6320::PlayerController::UpdateCamera(Graphics::Camera &camera, Graphics::GameObject gameObject)
{
	Math::cVector target = gameObject.transform.getPosition() - (gameObject.transform.getForward() * 200) + (gameObject.transform.getUp() * 50);
	Math::cVector current = camera.transform.getPosition();
	Math::cVector increment = (target - current)* 0.01f;
	camera.Move(current + increment);
	Math::cVector targetRot = gameObject.transform.getRotation();
	Math::cVector currentRot = camera.transform.getRotation();
	Math::cVector incrementRot = (targetRot - currentRot) * 0.01f;
	camera.Rotate(currentRot + incrementRot);
	Graphics::SetCamera(camera);
}


void eae6320::PlayerController::Update(Graphics::GameObject & gameObject,  Graphics::Camera &camera)
{
	const float cameraSpeed = MAXSPEED * Time::GetElapsedSecondCount_duringPreviousFrame()*5.0f;
	{
		if (UserInput::IsKeyPressed(68))
			gameObject.rigidBody.velocity += camera.transform.getRight() * cameraSpeed;
		if (UserInput::IsKeyPressed(65))
			gameObject.rigidBody.velocity -= camera.transform.getRight() * cameraSpeed;
		if (UserInput::IsKeyPressed(83))
			gameObject.rigidBody.velocity -= camera.transform.getForward() * cameraSpeed;
		if (UserInput::IsKeyPressed(87))
			gameObject.rigidBody.velocity += camera.transform.getForward() * cameraSpeed;
		const float rotationSpeed = 100.0f;
	/*	if (UserInput::IsKeyPressed(VK_RIGHT))
		{
			camera.Rotate(camera.transform.getRotation() + Math::cVector(0, rotationSpeed * Time::GetElapsedSecondCount_duringPreviousFrame(), 0));
		}
		if (UserInput::IsKeyPressed(VK_LEFT))
		{
			camera.Rotate(camera.transform.getRotation() - Math::cVector(0, rotationSpeed * Time::GetElapsedSecondCount_duringPreviousFrame(), 0));
		}*/
	}
	{
		float deltaTime = Time::GetElapsedSecondCount_duringPreviousFrame();
		gameObject.rigidBody.acceleration = gameObject.rigidBody.velocity * (-gameObject.rigidBody.drag);
		Math::cVector gravity(0.0f, -100, 0.0f);
		Math::cVector position = gameObject.transform.getPosition();
		if (Math::cVector(gameObject.rigidBody.velocity.x, 0, gameObject.rigidBody.velocity.z).GetLength() > 1)
		{
			gameObject.rigidBody.toVelocityPoint = gameObject.transform.getPosition() + (Math::cVector(gameObject.rigidBody.velocity.x, 0, gameObject.rigidBody.velocity.z)).CreateNormalized() * 30;
		}
		gameObject.rigidBody.toFloorPoint = gameObject.transform.getPosition() - Math::cVector(0, gameObject.rigidBody.height/2.0f, 0);
		gameObject.rigidBody.acceleration += gravity;
		gameObject.rigidBody.velocity += gameObject.rigidBody.acceleration * deltaTime;
		position = gameObject.transform.getPosition() + ((gameObject.rigidBody.velocity * deltaTime) + (gameObject.rigidBody.acceleration * (0.5f * deltaTime * deltaTime)));
		gameObject.Move(position);
		Physics::CheckCollision(&gameObject);
	}
}