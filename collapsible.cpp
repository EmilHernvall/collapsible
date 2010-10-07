#include "stdafx.h"
#include "collapsible.h"
#include "game.h"
#include "imageloader.h"

#define BUFFERSIZE 10

Collapsible::Collapsible() :
    m_hwnd(NULL),
    m_pDirect2dFactory(NULL),
    m_pRenderTarget(NULL),
	m_pDirectWriteFactory(NULL),
	m_pTextFormat(NULL),
	m_pDirectInput(NULL),
	m_pKeyboard(NULL),
	m_pBackgroundImage(NULL),
	m_pGame(NULL)
{
	m_pGame = new Game();
}

Collapsible::~Collapsible()
{
	if (m_pGame != NULL) {
		delete m_pGame;
	}

    SafeRelease(&m_pDirect2dFactory);
    SafeRelease(&m_pRenderTarget);

	SafeRelease(&m_pTextFormat);
	SafeRelease(&m_pDirectWriteFactory);

	m_pKeyboard->Unacquire();
	SafeRelease(&m_pKeyboard);
	SafeRelease(&m_pDirectInput);

	SafeRelease(&m_pBackgroundImage);
	SafeRelease(&m_pBrickImage);
	SafeRelease(&m_pLogoImage);
}

void Collapsible::RunMessageLoop()
{
	MSG msg;
	BYTE diKeys[256];

	while (TRUE) {
		if (m_bHasFocus && 
			m_pKeyboard->GetDeviceState(256, diKeys) == DI_OK) {
		
			BOOL bKeyPressed = FALSE;
			if (diKeys[DIK_LEFT] & 0x80) {
				m_pGame->moveLeft();
				bKeyPressed = TRUE;
			} 
			if (diKeys[DIK_RIGHT] & 0x80) {
				m_pGame->moveRight();
				bKeyPressed = TRUE;
			} 
			if (diKeys[DIK_UP] & 0x80) {
				m_pGame->rotate();
				bKeyPressed = TRUE;
			} 
			if (diKeys[DIK_DOWN] & 0x80) {
				m_pGame->drop();
				bKeyPressed = TRUE;
			}
			if (diKeys[DIK_P] & 0x80) {
				m_pGame->togglePause();
				bKeyPressed = TRUE;
			}

			if (bKeyPressed) {
				RECT rc;
				GetClientRect(m_hwnd, &rc);
				InvalidateRect(m_hwnd, &rc, TRUE);
			}
		}

		if (m_bHasFocus && m_pGame->tick()) {
			RECT rc;
			GetClientRect(m_hwnd, &rc);
			InvalidateRect(m_hwnd, &rc, TRUE);
		}

		while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)){ 
			if (msg.message == WM_QUIT) {
				return;
			}
			TranslateMessage(&msg); 
			DispatchMessage(&msg); 
		}
		
		// Yield most of our CPU time when we don't have focus
		if (!m_bHasFocus) {
			Sleep(100);
		}
	}
}

HRESULT Collapsible::Initialize()
{
    HRESULT hr;

    // Initialize device-indpendent resources, such
    // as the Direct2D factory.
    hr = CreateDeviceIndependentResources();
    if (FAILED(hr)) {
		return hr;
    }

    // Register the window class.
    WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = Collapsible::WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = sizeof(LONG_PTR);
    wcex.hInstance = HINST_THISCOMPONENT;
    wcex.hbrBackground = NULL;
    wcex.lpszMenuName = NULL;
    wcex.hCursor  = LoadCursor(NULL, IDI_APPLICATION);
    wcex.lpszClassName = L"CollapsibleClass";

    RegisterClassEx(&wcex);

    // Create the window.
    m_hwnd = CreateWindow(
        L"CollapsibleClass",
        L"Collapsible",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        640,
        480,
        NULL,
        NULL,
        HINST_THISCOMPONENT,
        this);

    hr = m_hwnd ? S_OK : E_FAIL;
    if (SUCCEEDED(hr)) {
		m_bHasFocus = TRUE;
        ShowWindow(m_hwnd, SW_SHOWNORMAL);
        UpdateWindow(m_hwnd);
	} else {
		return hr;
	}

	hr = InitializeInput();
	if (FAILED(hr)) {
		return hr;
	}

    return hr;
}

HRESULT Collapsible::InitializeInput()
{
	HRESULT hr;

	hr = DirectInput8Create(HINST_THISCOMPONENT, DIRECTINPUT_VERSION, IID_IDirectInput8, 
		reinterpret_cast<LPVOID*>(&m_pDirectInput), NULL);
	if (FAILED(hr)) {
		return hr;
	}

	hr = m_pDirectInput->CreateDevice(GUID_SysKeyboard, &m_pKeyboard, NULL);
	if (FAILED(hr)) {
		return hr;
	}

	/*DIPROPDWORD dipdw;
	dipdw.diph.dwSize = sizeof(DIPROPDWORD);
	dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	dipdw.diph.dwObj = 0;
	dipdw.diph.dwHow = DIPH_DEVICE;
	dipdw.dwData = BUFFERSIZE;
	hr = m_pKeyboard->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph);*/

	m_pKeyboard->SetCooperativeLevel(m_hwnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
	m_pKeyboard->SetDataFormat(&c_dfDIKeyboard); 
	//m_pKeyboard->SetEventNotification(m_hKeyboardEvent);
	m_pKeyboard->Acquire();

	return hr;
}

HRESULT Collapsible::CreateDeviceIndependentResources()
{
    HRESULT hr = S_OK;

    // Create a Direct2D factory.
    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pDirect2dFactory);
	if (FAILED(hr)) {
		return hr;
	}

	hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,
		__uuidof(IDWriteFactory),
		reinterpret_cast<IUnknown**>(&m_pDirectWriteFactory));
	if (FAILED(hr)) {
		return hr;
	}

	hr = m_pDirectWriteFactory->CreateTextFormat(
		L"Verdana",
		NULL,
		DWRITE_FONT_WEIGHT_REGULAR,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		16.0f,
		L"en-us",
		&m_pTextFormat);

	m_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);

    return hr;
}

HRESULT Collapsible::CreateDeviceResources()
{
	HRESULT hr = S_OK;
	RECT rc;

	if (m_pRenderTarget) {
		return S_OK;
	}

	GetClientRect(m_hwnd, &rc);

    D2D1_SIZE_U size = D2D1::SizeU(
        rc.right - rc.left,
        rc.bottom - rc.top);

    // Create a Direct2D render target.
    hr = m_pDirect2dFactory->CreateHwndRenderTarget(
        D2D1::RenderTargetProperties(),
        D2D1::HwndRenderTargetProperties(m_hwnd, size),
        &m_pRenderTarget);

	if (FAILED(hr)) {
		return hr;
	}

	if (!m_pBackgroundImage) {
		ImageLoader *pLoader = new ImageLoader();
		hr = pLoader->loadImageResource(m_pRenderTarget, L"BackgroundImage", &m_pBackgroundImage);
		hr = pLoader->loadImageResource(m_pRenderTarget, L"BrickImage", &m_pBrickImage);
		hr = pLoader->loadImageResource(m_pRenderTarget, L"LogoImage", &m_pLogoImage);
		delete pLoader;
	}

	return hr;
}

void Collapsible::DiscardDeviceResources()
{
    SafeRelease(&m_pRenderTarget);
}

LRESULT CALLBACK Collapsible::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;

    if (message == WM_CREATE) {
        LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
        Collapsible *pApp = (Collapsible *)pcs->lpCreateParams;

        ::SetWindowLongPtrW(
            hwnd,
            GWLP_USERDATA,
            PtrToUlong(pApp)
            );

        result = 1;
    }
    else {
        Collapsible *pApp = reinterpret_cast<Collapsible*>(static_cast<LONG_PTR>(
            ::GetWindowLongPtrW(hwnd, GWLP_USERDATA)));

        bool wasHandled = false;

        if (pApp) {
            switch (message)
            {
			case WM_SETFOCUS:
				pApp->m_bHasFocus = TRUE;
				if (pApp->m_pKeyboard) {
					pApp->m_pKeyboard->Acquire();
				}
				break;
			case WM_KILLFOCUS:
				pApp->m_bHasFocus = FALSE;
				if (pApp->m_pKeyboard) {
					pApp->m_pKeyboard->Unacquire();
				}
				break;
            case WM_SIZE:
                {
                    UINT width = LOWORD(lParam);
                    UINT height = HIWORD(lParam);
                    pApp->OnResize(width, height);
                }
                result = 0;
                wasHandled = true;
                break;

            case WM_DISPLAYCHANGE:
                {
                    InvalidateRect(hwnd, NULL, FALSE);
                }
                result = 0;
                wasHandled = true;
                break;

            case WM_PAINT:
                {
                    pApp->OnRender();
                    ValidateRect(hwnd, NULL);
                }
                result = 0;
                wasHandled = true;
                break;

            case WM_DESTROY:
                {
                    PostQuitMessage(0);
                }
                result = 1;
                wasHandled = true;
                break;
            }
        }

        if (!wasHandled) {
            result = DefWindowProc(hwnd, message, wParam, lParam);
        }
    }

    return result;
}

HRESULT Collapsible::OnRender()
{
    HRESULT hr = S_OK;

    hr = CreateDeviceResources();
    if (SUCCEEDED(hr)) {
		m_pRenderTarget->BeginDraw();
		m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
		m_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));

		D2D1_SIZE_F rtSize = m_pRenderTarget->GetSize();

		// Draw background
		D2D1_RECT_F globalRect = D2D1::RectF(
			static_cast<float>(0), 
			static_cast<float>(0),
			static_cast<float>(rtSize.width), 
			static_cast<float>(rtSize.height));

		m_pRenderTarget->DrawBitmap(m_pBackgroundImage, &globalRect , 1.0f);

		// Draw logo
		int logoWidth = rtSize.width / 2 - rtSize.height / 4;
		int logoHeight = (logoWidth * m_pLogoImage->GetSize().height) / m_pLogoImage->GetSize().width;
		D2D1_RECT_F logoRect = D2D1::RectF(
			static_cast<float>(0), 
			static_cast<float>(0),
			static_cast<float>(logoWidth), 
			static_cast<float>(logoHeight));

		m_pRenderTarget->DrawBitmap(m_pLogoImage, &logoRect , 1.0f);

		m_pGame->draw(m_pRenderTarget, logoHeight, m_pBrickImage, m_pTextFormat);

		hr = m_pRenderTarget->EndDraw();
    }

    if (hr == D2DERR_RECREATE_TARGET) {
        hr = S_OK;
        DiscardDeviceResources();
    }

    return hr;
}

void Collapsible::OnResize(UINT width, UINT height)
{
    if (m_pRenderTarget)
    {
        // Note: This method can fail, but it's okay to ignore the
        // error here, because the error will be returned again
        // the next time EndDraw is called.
        m_pRenderTarget->Resize(D2D1::SizeU(width, height));
    }
}

int APIENTRY _tWinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance, LPTSTR lpCmdLine,
	int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

    if (!SUCCEEDED(CoInitialize(NULL))) {
		return 0;
    }

    Collapsible app;
    if (SUCCEEDED(app.Initialize())) {
        app.RunMessageLoop();
    }
    
    CoUninitialize();

	return 0;
}
