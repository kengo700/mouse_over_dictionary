#include "image.h"

void Image::Flip(void* In, void* Out, int width, int height, unsigned int Bpp)
{
	unsigned long Chunk = (Bpp > 24 ? width * 4 : width * 3 + width % 4);
	unsigned char* Destination = static_cast<unsigned char*>(Out);
	unsigned char* Source = static_cast<unsigned char*>(In) + Chunk * (height - 1);

	while (Source != In)
	{
		std::memcpy(Destination, Source, Chunk);
		Destination += Chunk;
		Source -= Chunk;
	}
}

Image::Image(HDC DC, int X, int Y, int Width, int Height) : Pixels(), width(Width), height(Height), BitsPerPixel(32)
{
	BITMAP Bmp = { 0 };
	HBITMAP hBmp = reinterpret_cast<HBITMAP>(GetCurrentObject(DC, OBJ_BITMAP));

	if (GetObject(hBmp, sizeof(BITMAP), &Bmp) == 0)
		throw std::runtime_error("BITMAP DC NOT FOUND.");

	RECT area = { X, Y, X + Width, Y + Height };
	HWND Window = WindowFromDC(DC);
	GetClientRect(Window, &area);

	HDC MemDC = GetDC(nullptr);
	HDC SDC = CreateCompatibleDC(MemDC);
	HBITMAP hSBmp = CreateCompatibleBitmap(MemDC, width, height);
	DeleteObject(SelectObject(SDC, hSBmp));

	BitBlt(SDC, 0, 0, width, height, DC, X, Y, SRCCOPY);
	StretchBlt(SDC, 0, 0, width, height, DC, X, Y, Width, Height, SRCCOPY);

	unsigned int data_size = ((width * BitsPerPixel + 31) / 32) * 4 * height;
	std::vector<std::uint8_t> Data(data_size);
	this->Pixels.resize(data_size);

	// キャプチャした画像を画面左上に表示（デバッグ用）
	//BitBlt(DC, 0, 0, width, height, SDC, 0, 0, SRCCOPY);

	BITMAPINFO Info = { sizeof(BITMAPINFOHEADER), static_cast<long>(width), static_cast<long>(height), 1, BitsPerPixel, BI_RGB, data_size, 0, 0, 0, 0 };
	GetDIBits(SDC, hSBmp, 0, height, &Data[0], &Info, DIB_RGB_COLORS);
	this->Flip(&Data[0], &Pixels[0], width, height, BitsPerPixel);

	DeleteDC(SDC);
	DeleteObject(hSBmp);
	ReleaseDC(nullptr, MemDC);
}

// 文字認識精度を上げるため、拡大してキャプチャする
Image::Image(HDC DC, int X, int Y, int Width, int Height, int scale) : Pixels(), width(Width), height(Height), BitsPerPixel(32)
{
	width = (int)((width  * scale) / 100);
	height = (int)((height * scale) / 100);

	BITMAP Bmp = { 0 };
	HBITMAP hBmp = reinterpret_cast<HBITMAP>(GetCurrentObject(DC, OBJ_BITMAP));

	if (GetObject(hBmp, sizeof(BITMAP), &Bmp) == 0)
		throw std::runtime_error("BITMAP DC NOT FOUND.");

	RECT area = { X, Y, X + Width, Y + Height };
	HWND Window = WindowFromDC(DC);
	GetClientRect(Window, &area);

	HDC MemDC = GetDC(nullptr);
	HDC SDC = CreateCompatibleDC(MemDC);
	HBITMAP hSBmp = CreateCompatibleBitmap(MemDC, width, height);
	DeleteObject(SelectObject(SDC, hSBmp));

	//SetStretchBltMode(SDC, BLACKONWHITE); // 伸縮モードBLACKONWHITE：低画質だが高速、デフォルト設定
	//SetStretchBltMode(SDC, COLORONCOLOR); // 伸縮モードCOLORONCOLOR：中画質で高速、デフォルト設定
	SetStretchBltMode(SDC, HALFTONE); // 伸縮モードHALFTONE：高画質だが低速
	SetBrushOrgEx(SDC, 0, 0, NULL);
	StretchBlt(SDC, 0, 0, width, height, DC, X, Y, Width, Height, SRCCOPY);

	unsigned int data_size = ((width * BitsPerPixel + 31) / 32) * 4 * height;
	std::vector<std::uint8_t> Data(data_size);
	this->Pixels.resize(data_size);

	// キャプチャした画像を画面左上に表示（デバッグ用）
	//BitBlt(DC, 0, 0, width, height, SDC, 0, 0, SRCCOPY);

	BITMAPINFO Info = { sizeof(BITMAPINFOHEADER), static_cast<long>(width), static_cast<long>(height), 1, BitsPerPixel, BI_RGB, data_size, 0, 0, 0, 0 };
	GetDIBits(SDC, hSBmp, 0, height, &Data[0], &Info, DIB_RGB_COLORS);
	this->Flip(&Data[0], &Pixels[0], width, height, BitsPerPixel);

	DeleteDC(SDC);
	DeleteObject(hSBmp);
	ReleaseDC(nullptr, MemDC);
}