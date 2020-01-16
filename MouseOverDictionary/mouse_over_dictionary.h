#pragma once

#include <QtWidgets/QMainWindow>
#include <QTimer>
#include <QStringList>
#include <QStringListModel>
#include <QAbstractItemView>
#include <QClipboard>
#include <QSettings>

#include "ui_mouse_over_dictionary.h"
#include "mini_window.h"
#include "thread.h"

class MouseOverDictionary : public QMainWindow
{
	Q_OBJECT

public:
	MouseOverDictionary(QWidget *parent = Q_NULLPTR);
	~MouseOverDictionary();

private:
	void ReadSettings();
	void WriteSettings();

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

	int ocr_scale;
	int ocr_area_left;
	int ocr_area_right;
	int ocr_area_top;
	int ocr_area_bottom;

	int main_window_word_font_size;
	int main_window_text_font_size;
	int main_window_mark_font_size;
	std::string main_window_word_font_color;
	std::string main_window_text_font_color;
	std::string main_window_mark_font_color;
	std::string main_window_background_color;

	int mini_window_word_font_size;
	int mini_window_text_font_size;
	std::string mini_window_word_font_color;
	std::string mini_window_text_font_color;
	std::string mini_window_background_color;

	int main_window_width;
	int main_window_height;
	int main_window_history_width;

	int mini_window_width;
	int mini_window_height;
	int mini_window_position_x;
	int mini_window_position_y;

	bool show_mini_window;
	bool show_history;
	bool always_on_top;

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
