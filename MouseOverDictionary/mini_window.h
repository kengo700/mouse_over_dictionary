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

public slots:
	void followCursor();
	void setHtml(QString text);
	void toggleShow(bool toggle);

private:
	Ui::MiniWindow ui;
};
