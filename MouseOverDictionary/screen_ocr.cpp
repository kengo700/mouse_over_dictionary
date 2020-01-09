#include "screen_ocr.h"

ScreenOCR::ScreenOCR()
{
	tesseract_ptr = std::unique_ptr<tesseract::TessBaseAPI>(new tesseract::TessBaseAPI());

}

bool ScreenOCR::Init()
{
	// Tesseract初期化
	if (tesseract_ptr->Init(NULL, "eng")) {
		//fprintf(stderr, "Could not initialize tesseract.\n");
		return false;
	}

	// ホワイトリスト設定
	//   「-」を指定しているのに検出されない？ 一旦設定せず
	//tesseract_ptr->SetVariable("tessedit_char_whitelist", "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz-+,.;:/<>()[]*'\"!? ");

	// 警告「Warning. Invalid resolution 0 dpi. Using 70 instead.」を消すため
	//tesseract_ptr->SetVariable("user_defined_dpi", "300");
	tesseract_ptr->SetVariable("user_defined_dpi", "96");

	// Page Segmentation ModeのSingle Text Lineを試してみる
	//tesseract_ptr->SetVariable("psm", "7");

	return true;
}


bool ScreenOCR::Recognize(int x, int y, int width, int height)
{

	HWND SomeWindowHandle = GetDesktopWindow();
	HDC DC = GetDC(SomeWindowHandle);

	if (DC == NULL) {
		return false;
	}

	int roi_w = 200;
	int roi_h = 40;
	int roi_mouse_x = roi_w / 2;
	int roi_mouse_y = roi_h / 2;

	// マウス付近の画像をキャプチャ
	try {
		Image Img = Image(DC, x, y, width, height);
		tesseract_ptr->SetImage(Img.GetPixels(), Img.GetWidth(), Img.GetHeight(), Img.GetBytesPerPixel(), Img.GetBytesPerScanLine()); //Fixed this line..
	}
	catch (...) {
		return false;
	}

	ReleaseDC(SomeWindowHandle, DC);

	// 文字認識
	tesseract_ptr->Recognize(0);
	tesseract::ResultIterator* ri = tesseract_ptr->GetIterator();
	tesseract::PageIteratorLevel level = tesseract::RIL_WORD;

	// 結果を保存
	ocr_results.clear();
	bool found = false;
	if (ri != 0) {
		do {
			ocr_result result;
			ri->BoundingBox(level, &result.x1, &result.y1, &result.x2, &result.y2);

			const char* word = ri->GetUTF8Text(level);
			if (word != NULL) {
				result.word = word;
				ocr_results.push_back(result);
				found = true;
			}
			delete[] word;
		} while (ri->Next(level));
	}

	return found;
}


void ScreenOCR::GetResults(std::vector<ocr_result>& results)
{
	results = this->ocr_results;
}
