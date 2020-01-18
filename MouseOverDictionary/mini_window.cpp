#include "mini_window.h"

MiniWindow::MiniWindow(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
}

MiniWindow::~MiniWindow()
{
}

void MiniWindow::setRelativePos(int x, int y)
{
	relative_pos_x = x;
	relative_pos_y = y;
}

void MiniWindow::followCursor()
{
	POINT po;
	GetCursorPos(&po);
	this->move(po.x + relative_pos_x, po.y + relative_pos_y);
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