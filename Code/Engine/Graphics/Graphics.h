/*
	This file contains the function declarations for graphics
*/

#ifndef EAE6320_GRAPHICS_H
#define EAE6320_GRAPHICS_H

// Header Files
//=============

#include "Configuration.h"
#include "Mesh.h"
#include "ConstantBufferData.h"
#include "../Math/cVector.h"
#include "../Math/cQuaternion.h"
#include "Camera.h"
#include "cMaterial.h"
#include "cSprite.h"
#include "cText.h"

#if defined( EAE6320_PLATFORM_WINDOWS )
	#include "../Windows/Includes.h"
#endif

// Interface
//==========

namespace eae6320
{
	namespace Graphics
	{
		// Render
		//-------

		void RenderFrame();

		// Initialization / Clean Up
		//--------------------------

		struct sInitializationParameters
		{
#if defined( EAE6320_PLATFORM_WINDOWS )
			HWND mainWindow;
	#if defined( EAE6320_PLATFORM_D3D )
			unsigned int resolutionWidth, resolutionHeight;
	#elif defined( EAE6320_PLATFORM_GL )
			HINSTANCE thisInstanceOfTheApplication;
	#endif
#endif
		};
		
		struct MeshObject
		{
			cMaterial *material;
			Mesh *mesh;
			Math::cVector position;
			Math::cQuaternion rotation;
		};
		
		struct UIObject
		{
			cMaterial material;
			cSprite* sprite;
		};

		struct UIText
		{
			cMaterial material;
			cText* text;
		};

		bool Initialize( const sInitializationParameters& i_initializationParameters );
		bool CleanUp();
		void SetMesh(MeshObject i_gameObject);
		void SetSprite(UIObject i_uiObject);
		void AddUIText(UIText i_uiText);
		void SetCamera(Camera i_camera);
	}
}
#endif	// EAE6320_GRAPHICS_H
