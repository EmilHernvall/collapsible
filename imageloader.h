#pragma once

class ImageLoader
{
public:
	ImageLoader();
	~ImageLoader();

	HRESULT loadImageResource(ID2D1RenderTarget *pRenderTarget, LPCWSTR szName, ID2D1Bitmap **lpOut);

private:
	IWICImagingFactory *m_pImagingFactory;
};
