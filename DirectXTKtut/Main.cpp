//
// Main.cpp
//

#include "EnginePCH.h"
#include "DR_ModelRendering.h"


// Entry point
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	DR_ModelRendering game;

	return game.Run(hInstance);
}
