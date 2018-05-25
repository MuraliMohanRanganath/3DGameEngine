// Header Files
//=============

#include "../UserInput.h"

#include "../../Windows/Includes.h"

// Helper Function Declarations
//=============================

namespace
{
	bool IsVirtualKeyPressed( const int i_virtualKeyCode );
	bool IsVirtualKeyPressedOnce(const int i_virtualKeyCode);
	bool isKeyPressedOnce = false;
	int keyThatIsPressedAndNotReleased = 0;
}


// Interface
//==========

bool eae6320::UserInput::IsKeyPressed( const int i_virtualKeyCode )
{
	return IsVirtualKeyPressed( i_virtualKeyCode );
}

bool eae6320::UserInput::IsKeyPressedOnce(const int i_virtualKeyCode)
{
	return IsVirtualKeyPressedOnce(i_virtualKeyCode);
}

bool eae6320::UserInput::IsMouseButtonPressed( const int i_virtualButtonCode )
{
	return IsVirtualKeyPressed( i_virtualButtonCode );
}

// Helper Function Definitions
//============================

namespace
{
	bool IsVirtualKeyPressed(const int i_virtualKeyCode)
	{
		if (GetActiveWindow()) {

			short keyState = GetAsyncKeyState(i_virtualKeyCode);
			const short isKeyDownMask = ~1;
			return (keyState & isKeyDownMask) != 0;
		}
		return false;
	}

	bool IsVirtualKeyPressedOnce(const int i_virtualKeyCode)
	{
		if (GetActiveWindow())
		{
			if ((GetAsyncKeyState(i_virtualKeyCode) & 0xfffe) && !isKeyPressedOnce)
			{
				keyThatIsPressedAndNotReleased = i_virtualKeyCode;
				isKeyPressedOnce = true;
				return true;
			}
			else if (!(GetAsyncKeyState(keyThatIsPressedAndNotReleased) & 0xfffe))
			{
				keyThatIsPressedAndNotReleased = 0;
				isKeyPressedOnce = false;
			}
		}
		return false;
	}
}