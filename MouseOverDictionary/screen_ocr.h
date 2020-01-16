#pragma once

#include <vector>
#include <tesseract/baseapi.h>

#include "image.h"

struct ocr_result
{
	std::string word;
	float conf;
	int x1, x2, y1, y2;
};

class ScreenOCR
{
public:
	ScreenOCR();

private:
	std::unique_ptr<tesseract::TessBaseAPI> tesseract_ptr;
	std::vector<ocr_result> ocr_results;

public:
	bool Init();
	bool Recognize(int x, int y, int width, int height, int scale);
	void GetResults(std::vector<ocr_result>& results);
};
