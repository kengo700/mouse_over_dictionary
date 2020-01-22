#pragma once

#include <QWidget>
#include <QScroller>
#include <QScrollBar>
#include "ui_pause_window.h"

#include "windows.h"

class PauseWindow : public QWidget
{
	Q_OBJECT

public:
	PauseWindow(QWidget *parent = Q_NULLPTR);
	~PauseWindow();
	void setScreenshot(QPixmap screenPixmap);
	void logActiveWindow(HWND hWnd);

private:
	Ui::PauseWindow ui;

	HWND previous_active_window;
protected:
	void mousePressEvent(QMouseEvent  *event);
};
