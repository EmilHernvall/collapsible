#pragma once

#include "resource.h"

template<class Interface>
inline void SafeRelease(Interface **ppInterfaceToRelease)
{
    if (*ppInterfaceToRelease != NULL) {
        (*ppInterfaceToRelease)->Release();
        (*ppInterfaceToRelease) = NULL;
    }
}

#ifndef Assert
#if defined( DEBUG ) || defined( _DEBUG )
#define Assert(b) do {if (!(b)) {OutputDebugStringA("Assert: " #b "\n");}} while(0)
#else
#define Assert(b)
#endif //DEBUG || _DEBUG
#endif

#ifndef HINST_THISCOMPONENT
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)
#endif

class Game;

class Collapsible
{
public:
    Collapsible();
    ~Collapsible();

    // Register the window class and call methods for instantiating drawing resources
    HRESULT Initialize();

    // Process and dispatch messages
    void RunMessageLoop();

private:
	// Window state
	HWND m_hwnd;
	BOOL m_bHasFocus;

	// Direct2D
	ID2D1Factory *m_pDirect2dFactory;
	ID2D1HwndRenderTarget *m_pRenderTarget;

	// DirectWrite
	IDWriteFactory *m_pDirectWriteFactory;
	IDWriteTextFormat *m_pTextFormat;

	// DirectInput
	IDirectInput8 *m_pDirectInput;
	IDirectInputDevice8 *m_pKeyboard;

	// Images
	ID2D1Bitmap *m_pBackgroundImage;
	ID2D1Bitmap *m_pBrickImage;
	ID2D1Bitmap *m_pLogoImage;

	// Game
	Game *m_pGame;

private:
    // Initialize input devices.
    HRESULT InitializeInput();

    // Initialize device-independent resources.
    HRESULT CreateDeviceIndependentResources();

    // Initialize device-dependent resources.
    HRESULT CreateDeviceResources();

    // Release device-dependent resource.
    void DiscardDeviceResources();

    // Draw content.
    HRESULT OnRender();

    // Resize the render target.
    void OnResize(
        UINT width,
        UINT height
        );

    // The windows procedure.
    static LRESULT CALLBACK WndProc(
        HWND hWnd,
        UINT message,
        WPARAM wParam,
        LPARAM lParam
        );
};
