#pragma once

#include <QWidget>
#include <windows.h>

#include "ui_mini_window.h"

class MiniWindow : public QWidget
{
	Q_OBJECT

public:
	MiniWindow(QWidget *parent = Q_NULLPTR);
	~MiniWindow();
	void setRelativePos(int x, int y);

public slots:
	void followCursor();
	void setHtml(QString text);
	void toggleShow(bool toggle);

private:
	int relative_pos_x;
	int relative_pos_y;
	Ui::MiniWindow ui;
};
