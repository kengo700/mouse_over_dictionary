#include "mini_window.h"

MiniWindow::MiniWindow(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
}

MiniWindow::~MiniWindow()
{
}

void MiniWindow::followCursor()
{
	POINT po;
	GetCursorPos(&po);
	this->move(po.x, po.y + 25);
}

void MiniWindow::setHtml(QString text)
{
	ui.textBrowser->setHtml(text);
}

void MiniWindow::toggleShow(bool toggle)
{
	if (toggle) {
		this->show();
	}
	else {
		this->hide();
	}
}