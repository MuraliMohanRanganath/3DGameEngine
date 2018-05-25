/*
	This class is your specific game
*/

#ifndef EAE6320_CMYGAME_H
#define EAE6320_CMYGAME_H

// Header Files
//=============

#include "../../Engine/Application/cbApplication.h"
#include "../../Engine/Graphics/Mesh.h"
#include "../../Engine/Graphics/Camera.h"
#include "../../Engine/Graphics/Graphics.h"
#include "../../Engine/Graphics/GameObject.h"
#include "../../Engine/Graphics/DebugObject.h"
#include "../../Engine/Graphics/CheckBox.h"
#include "FlyCam.h"
#include <ctime>
#include "cPlayer.h"

#if defined( EAE6320_PLATFORM_WINDOWS )
	#include "Resource Files/Resource.h"
#endif

// Class Declaration
//==================

namespace eae6320
{
	class cMyGame : public Application::cbApplication
	{
		// Interface
		//==========

	public:

		// Initialization / Clean Up
		//--------------------------

		virtual ~cMyGame();

		// Inherited Implementation
		//=========================

	private:

		Graphics::Camera flyCamera;
		FlyCamera flyCam;

		// Configuration
		Graphics::GameObject ceilingGameObject;
		Graphics::GameObject floorGameObject;
		Graphics::GameObject metalGameObject;
		Graphics::GameObject propsGameObject;
		Graphics::GameObject railingGameObject;
		Graphics::GameObject wallsGameObject;
		std::vector<Graphics::GameObject> worldObjects;
		Game::cPlayer player;

		//Debug Shapes array
		Graphics::DebugObject debugSphere;
		Graphics::DebugObject debugSphere2;
		
		//Debug Menu
		Graphics::UIText fpsText;
		Graphics::UIText arrowText;
		Graphics::UIText sliderText;
		Graphics::UIText sliderIndexText;
		Graphics::UIText resetText;
		Graphics::Checkbox * checkBox;
		Graphics::UIText musicVolumeSliderText;
		Graphics::UIText musicVolumeSliderIndexText;

		Graphics::UIText effectVolumeSliderText;
		Graphics::UIText effectVolumeSliderIndexText;

		Graphics::UIText myScoreText;
		Graphics::UIText opponentScoreText;
		Graphics::UIText stamina;

		float sphereRadius = 100;
		int sliderIndex = 0;
		int sliderMax = 5;

		int musicSliderIndex = 1;
		int effectSliderIndex = 5;
		
		enum DebugOptions { CHECKBOX, SLIDER, RESET};
		DebugOptions optionsIndex = CHECKBOX;

		enum VolumeOptions { MUSIC, EFFECT };
		VolumeOptions vOpsIndex = MUSIC;

		bool isDebug = false;
		bool enableFlyCam = true;

		float offsetX;
		float offsetY;
		float offsetZ;
		float cameraAngle;
		//--------------

		// You can configure your game by overriding the following functions
		
		virtual const char* GetPathToLogTo() const { return "eae6320.log"; }
#if defined( EAE6320_PLATFORM_WINDOWS )
		// The main window's name will be displayed as its caption (the text that is displayed in the title bar).
		// You can make it anything that you want, but please keep the platform name and debug configuration in it somewhere
		// so that it's easy to tell at a glance what kind of build is running.
		virtual const char* GetMainWindowName() const
		{
			return "Ranganath_Murali's EAE6320 Game"
				" -- "
#if defined( EAE6320_PLATFORM_D3D )
				"Direct3D"
#elif defined( EAE6320_PLATFORM_GL )
				"OpenGL"
#endif
#ifdef _DEBUG
				" -- Debug"
#endif
				;
		}
		// Window classes are almost always identified by name;
		// there is a unique "ATOM" associated with them,
		// but in practice Windows expects to use the class name as an identifier.
		// If you don't change the name below
		// your program could conceivably have problems if it were run at the same time on the same computer
		// as one of your classmate's.
		// You don't need to worry about this for our class,
		// but if you ever ship a real project using this code as a base you should probably set this to something unique :)
		virtual const char* GetMainWindowClassName() const { return "Ranganath_Murali's EAE6320 Main Window Class"; }
		// If you want you can change the icons used for your game here. The following three are provided:
		//	* IDI_EAEGAMEPAD
		//	* IDI_EAEALIEN
		//	* IDI_VSDEFAULT_LARGE / IDI_VSDEFAULT_SMALL
		// If you want to create your a convenient website that will help is: http://icoconvert.com/
		virtual const WORD* GetLargeIconId() const { static const WORD iconId_large = IDI_EAEGAMEPAD; return &iconId_large; }
		virtual const WORD* GetSmallIconId() const { static const WORD iconId_small = IDI_EAEGAMEPAD; return &iconId_small; }
#endif

		// Initialization / Clean Up
		//--------------------------

		virtual bool Initialize();
		virtual bool CleanUp();
		virtual void SetUpFrame();
		virtual void SetUpCamera();

		void  createDebugShapes();
		void setDebugShapes();
		void cleanUpDebugShapes();
	};
}

#endif	// EAE6320_CMYGAME_H
