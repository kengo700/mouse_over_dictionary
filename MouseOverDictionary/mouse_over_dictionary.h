#pragma once

#include <QtWidgets/QMainWindow>
#include <QTimer>
#include <QStringList>
#include <QStringListModel>
#include <QAbstractItemView>
#include <QClipboard>
#include <QSettings>
#include <QScreen>

#include "ui_mouse_over_dictionary.h"
#include "mini_window.h"
#include "pause_window.h"
#include "thread.h"
#include "UGlobalHotkey\uglobalhotkeys.h"

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
	PauseWindow *pause_window;
	Thread thread;
	QClipboard *clipboard;

	UGlobalHotkeys *hm_show_hide_both;
	UGlobalHotkeys *hm_show_hide_mini;
	UGlobalHotkeys *hm_show_hide_pause;

	bool thread_ready = false;

	QStringListModel *historyModel;
	bool history_search_enable = true;
	bool history_update_enable = true;

	bool history_show = false;
	bool mini_show = false;
	bool stay_top = true;

	bool enable_shortcut;
	QString hotkey_show_hide_both;
	QString hotkey_show_hide_mini;
	QString hotkey_screen_pause;

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
	int main_window_pos_x;
	int main_window_pos_y;
	int main_window_history_width;

	int mini_window_width;
	int mini_window_height;
	int mini_window_pos_x;
	int mini_window_pos_y;

	double main_window_opacity;
	double mini_window_opacity;

	double pause_window_opacity;
	int pause_window_border_width;
	std::string pause_window_border_color;

	int screen_width;
	int screen_height;

public slots:
	// ToolButtonのchekableのトグルボタンは見た目が微妙だったので、普通のボタンクリックで切り替え
	void on_toolButton_clicked();
	void on_toolButton_2_clicked();
	void on_toolButton_3_clicked();

	void setReady(bool ready);
	void updateHistory(QString word);
	void searchByHistory(const QItemSelection &selected);
	void setFromClipboard();
	void showHide();
	void showHideMini();
	void showPauseWindow();

protected:
	void resizeEvent(QResizeEvent *event);
	void moveEvent(QMoveEvent *event);
	void closeEvent(QCloseEvent *event);
	void changeEvent(QEvent *event);
	//void hideEvent(QHideEvent *event);
	//void showEvent(QShowEvent *event);
};
