#pragma once

// Reference: https://stackoverflow.com/questions/22924209
// Author: Brandon https://stackoverflow.com/users/1462718/brandon
// License: Creative Commons Attribution-Share Alike

#include <vector>
#include <stdexcept>
#include <fstream>
#include <memory>
#include <cstring>
#include <windows.h>
#include <conio.h>

class Image
{
private:
	std::vector<std::uint8_t> Pixels;
	std::uint32_t width, height;
	std::uint16_t BitsPerPixel;

	void Flip(void* In, void* Out, int width, int height, unsigned int Bpp);

public:
	explicit Image(HDC DC, int X, int Y, int Width, int Height);
	explicit Image(HDC DC, int X, int Y, int Width, int Height, int scale);

	inline std::uint16_t GetBitsPerPixel() { return this->BitsPerPixel; }
	inline std::uint16_t GetBytesPerPixel() { return this->BitsPerPixel / 8; }
	inline std::uint16_t GetBytesPerScanLine() { return (this->BitsPerPixel / 8) * this->width; }
	inline int GetWidth() const { return this->width; }
	inline int GetHeight() const { return this->height; }
	inline const std::uint8_t* GetPixels() { return this->Pixels.data(); }
};