#include "pause_window.h"

PauseWindow::PauseWindow(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	// スクロールバー非表示
	ui.scrollArea->horizontalScrollBar()->setStyleSheet("QScrollBar {height:0px;}");
	ui.scrollArea->verticalScrollBar()->setStyleSheet("QScrollBar {width:0px;}");

	// スクロール不可
	ui.scrollArea->verticalScrollBar()->setEnabled(false);
	ui.scrollArea->horizontalScrollBar()->setEnabled(false);

	// ドラッグでスクロールできるように
	//QScroller::grabGesture(ui.scrollArea, QScroller::LeftMouseButtonGesture);

	// 枠無しウィンドウ
	this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
	this->setWindowState(windowState() ^ Qt::WindowMaximized);

}

PauseWindow::~PauseWindow()
{
}

void PauseWindow::setScreenshot(QPixmap screenPixmap)
{
	ui.label->setPixmap(screenPixmap);
}

void PauseWindow::logActiveWindow(HWND hWnd)
{
	previous_active_window = hWnd;
}

void PauseWindow::mousePressEvent(QMouseEvent * event)
{

	SetForegroundWindow(previous_active_window);

	this->hide();
}