#pragma once

#include <QThread>
#include <QMutex>
#include <QDebug>
#include <QRegularExpression>
#include <QTextBoundaryFinder>
#include <string>
#include <cctype>
#include <regex>

#include "screen_ocr.h"
#include "dictionary.h"
#include "inflector.h"

class Thread :
	public QThread
{
	Q_OBJECT

public:
	Thread(QObject *parent = nullptr);
	~Thread();

	void stop();

	void enableOcr();
	void disableOcr();

	void setWindowPos(int x, int y);
	void setWindowSize(int w, int h);
	void setOcrScale(int ocr_scale);
	void setOcrRoi(int left, int right, int top, int bottom);
	void setMainFontColor(std::string word, std::string text, std::string mark, std::string background);
	void setMiniFontColor(std::string word, std::string text, std::string background);
	void setMainFontSize(int word, int text, int mark);
	void setMiniFontSize(int word, int text);

signals:
	void wordChanged(QString word);
	void mainTextChanged(QString text);
	void miniTextChanged(QString text);
	void ready(bool ready);
	void wordFound(QString word);

public slots:
	bool search(QString word);

protected:
	void run() override;

private:
	QMutex mutex;
	bool stopped;

	bool enable_ocr;

	ScreenOCR ocr;
	Dictionary dict;
	Inflector inflect;

	int window_x, window_y;
	int window_w, window_h;

	int ocr_scale;
	int roi_w;
	int roi_h;
	int roi_mouse_x;
	int roi_mouse_y;

	std::string main_window_word_font_color;
	std::string main_window_text_font_color;
	std::string main_window_mark_font_color;
	std::string main_window_background_color;
	std::string mini_window_word_font_color;
	std::string mini_window_text_font_color;
	std::string mini_window_background_color;

	int main_window_word_font_size;
	int main_window_text_font_size;
	int main_window_mark_font_size;
	int mini_window_word_font_size;
	int mini_window_text_font_size;

};