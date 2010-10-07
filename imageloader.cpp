#include "stdafx.h"
#include "collapsible.h"
#include "imageloader.h"

ImageLoader::ImageLoader()
: m_pImagingFactory(NULL)
{
	HRESULT hr;

	hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, 
		CLSCTX_INPROC_SERVER, IID_IWICImagingFactory,
		(LPVOID*)&m_pImagingFactory);
	if (FAILED(hr)) {
		return;
	}
}

ImageLoader::~ImageLoader()
{
	SafeRelease(&m_pImagingFactory);
}

HRESULT ImageLoader::loadImageResource(ID2D1RenderTarget *pRenderTarget, LPCWSTR szName, ID2D1Bitmap **lpOut)
{
	HRESULT hr;

	HRSRC imageResHandle = NULL;
	HGLOBAL imageResDataHandle = NULL;
	void *pImageFile = NULL;
	DWORD imageFileSize = 0;

	IWICBitmapDecoder *pDecoder = NULL; 
	IWICBitmapFrameDecode *pSource = NULL;
	IWICStream *pStream = NULL;
	IWICFormatConverter *pConverter;
	ID2D1Bitmap *pImage;

	imageResHandle = FindResource(HINST_THISCOMPONENT, szName, L"Image");
	hr = imageResHandle ? S_OK : E_FAIL;
	if (FAILED(hr)) {
		goto cleanup;
	}

	imageResDataHandle = LoadResource(HINST_THISCOMPONENT, imageResHandle);
	hr = imageResHandle ? S_OK : E_FAIL;
	if (FAILED(hr)) {
		goto cleanup;
	}

    // Lock it to get a system memory pointer.
    pImageFile = LockResource(imageResDataHandle);
    hr = pImageFile ? S_OK : E_FAIL;
	if (FAILED(hr)) {
		goto cleanup;
	}

	imageFileSize = SizeofResource(HINST_THISCOMPONENT, imageResHandle);
	hr = imageFileSize ? S_OK : E_FAIL;
	if (FAILED(hr)) {
		goto cleanup;
	}

	hr = m_pImagingFactory->CreateStream(&pStream);
	if (FAILED(hr)) {
		goto cleanup;
	}

    hr = pStream->InitializeFromMemory(
        reinterpret_cast<BYTE*>(pImageFile),
        imageFileSize
        );
	if (FAILED(hr)) {
		goto cleanup;
	}

	hr = m_pImagingFactory->CreateDecoderFromStream(pStream, NULL, 
		WICDecodeMetadataCacheOnDemand, &pDecoder);
	if (FAILED(hr)) {
		goto cleanup;
	}

	hr = pDecoder->GetFrame(0, &pSource);
	if (FAILED(hr)) {
		goto cleanup;
	}

	hr = m_pImagingFactory->CreateFormatConverter(&pConverter);
	if (FAILED(hr)) {
		goto cleanup;
	}

    hr = pConverter->Initialize(
        pSource,
        GUID_WICPixelFormat32bppPBGRA,
        WICBitmapDitherTypeNone,
        NULL,
        0.f,
        WICBitmapPaletteTypeMedianCut
        );
	if (FAILED(hr)) {
		goto cleanup;
	}

	hr = pRenderTarget->CreateBitmapFromWicBitmap(pConverter, NULL, &pImage);
	if (FAILED(hr)) {
		goto cleanup;
	}

	*lpOut = pImage;

cleanup:

	SafeRelease(&pConverter);
	SafeRelease(&pStream);
	SafeRelease(&pDecoder);
	SafeRelease(&pSource);

	return hr;
}
