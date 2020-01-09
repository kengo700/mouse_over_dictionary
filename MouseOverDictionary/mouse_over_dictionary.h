#pragma once

#include <QtWidgets/QMainWindow>
#include <QTimer>
#include <QStringList>
#include <QStringListModel>
#include <QAbstractItemView>
#include <QClipboard>

#include "ui_mouse_over_dictionary.h"
#include "mini_window.h"
#include "thread.h"

class MouseOverDictionary : public QMainWindow
{
	Q_OBJECT

public:
	MouseOverDictionary(QWidget *parent = Q_NULLPTR);

private:
	Ui::MouseOverDictionaryClass ui;
	MiniWindow *mini_window;
	Thread thread;
	QClipboard *clipboard;

	bool thread_ready = false;

	QStringListModel *historyModel;
	bool history_search_enable = true;
	bool history_update_enable = true;

	bool history_show = false;
	bool mini_show = false;
	bool stay_top = true;

public slots:
	// ToolButtonのchekableのトグルボタンは見た目が微妙だったので、普通のボタンクリックで切り替え
	void on_toolButton_clicked();
	void on_toolButton_2_clicked();
	void on_toolButton_3_clicked();

	void setReady(bool ready);
	void updateHistory(QString word);
	void searchByHistory(const QItemSelection &selected);
	void setFromClipboard();

protected:
	void resizeEvent(QResizeEvent *event);
	void moveEvent(QMoveEvent *event);
	void closeEvent(QCloseEvent *event);

};
