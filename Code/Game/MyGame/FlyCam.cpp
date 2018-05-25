#include "FlyCam.h"
#include "../../Engine/Graphics/Camera.h"
#include "../../Engine/UserInput/UserInput.h"
#include "../../Engine/Time/Time.h"
#include "../../Engine/Graphics/Graphics.h"

void eae6320::FlyCamera::Update(Graphics::Camera &camera)
{

	Math::cVector cameraOffset(0.0f, 0.0f, 0.0f);
	const float cam_speed_unitsPerSecond = 100.0f;
	const float camoffsetModifier = cam_speed_unitsPerSecond * Time::GetElapsedSecondCount_duringPreviousFrame();
	{
		if (UserInput::IsKeyPressed(68))
			cameraOffset += camera.transform.getRight();
		if (UserInput::IsKeyPressed(65))
			cameraOffset -= camera.transform.getRight();
		if (UserInput::IsKeyPressed(69))
			cameraOffset += camera.transform.getUp();
		if (UserInput::IsKeyPressed(70))
			cameraOffset -= camera.transform.getUp();
		if (UserInput::IsKeyPressed(83))
			cameraOffset -= camera.transform.getForward();
		if (UserInput::IsKeyPressed(87))
			cameraOffset += camera.transform.getForward();
	}
	camera.Move(camera.transform.getPosition() + (cameraOffset)* camoffsetModifier);// += (cameraOffset)* camoffsetModifier;
	float rotationSpeed = 100.0f;

	if (UserInput::IsKeyPressed(VK_RIGHT))
	{
		camera.Rotate(camera.transform.getRotation() + Math::cVector(0, rotationSpeed * Time::GetElapsedSecondCount_duringPreviousFrame(), 0));
	}
	if (UserInput::IsKeyPressed(VK_LEFT))
	{
		camera.Rotate(camera.transform.getRotation() - Math::cVector(0, rotationSpeed * Time::GetElapsedSecondCount_duringPreviousFrame(), 0));
	}
	if (UserInput::IsKeyPressed(VK_DOWN))
	{
		camera.Rotate(camera.transform.getRotation() + Math::cVector(rotationSpeed * Time::GetElapsedSecondCount_duringPreviousFrame(), 0, 0));
	}
	if (UserInput::IsKeyPressed(VK_UP))
	{
		camera.Rotate(camera.transform.getRotation() - Math::cVector(rotationSpeed * Time::GetElapsedSecondCount_duringPreviousFrame(), 0, 0));
	}
	Graphics::SetCamera(camera);
}