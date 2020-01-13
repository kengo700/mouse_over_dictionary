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

	void setWindowPos(int x, int y);
	void setWindowSize(int w, int h);

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

	ScreenOCR ocr;
	Dictionary dict;
	Inflector inflect;

	int window_x, window_y;
	int window_w, window_h;

};

