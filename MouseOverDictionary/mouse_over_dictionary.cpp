#include "mouse_over_dictionary.h"

MouseOverDictionary::MouseOverDictionary(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	// スクリーンのサイズ取得
	QScreen *screen = QGuiApplication::primaryScreen();
	QRect  screenGeometry = screen->geometry();
	screen_width = screenGeometry.width();
	screen_height = screenGeometry.height();

	// 設定ファイル読み込み
	ReadSettings();

	// 透明度設定
	this->setWindowOpacity(main_window_opacity);

	// 前回終了時のウィンドウ位置、サイズを復元
	this->move(main_window_pos_x, main_window_pos_y);
	this->resize(main_window_width, main_window_height);

	// 前回終了時の最前面設定を復元
	if (stay_top == true) {
		this->setWindowFlags(Qt::WindowStaysOnTopHint);
		ui.toolButton_2->setIcon(QIcon(":/MouseOverDictionary/Resources/push-pin-2-line.png"));
	}
	else {
		ui.toolButton_2->setIcon(QIcon(":/MouseOverDictionary/Resources/push-pin-line.png"));
	}


	// マウス追従ウィンドウ作成
	//   最小化や最前面設定を独立にしたいので、メインウィンドウの子（new MiniWindow(this);）にはしない
	//   closeEventで削除する
	mini_window = new MiniWindow();

	// マウス追従ウィンドウを最前面、枠なしに指定
	mini_window->setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

	// 透明度、サイズ、位置設定
	mini_window->setWindowOpacity(mini_window_opacity);
	mini_window->resize(mini_window_width, mini_window_height);
	mini_window->setRelativePos(mini_window_pos_x, mini_window_pos_y);

	// マウス追従ウィンドウをカーソルに追従
	auto timer = new QTimer(this);
	timer->setInterval(15);
	timer->setSingleShot(false);
	connect(timer, SIGNAL(timeout()), mini_window, SLOT(followCursor()));
	timer->start();

	// 前回終了時のマウス追従ウィンドウの表示状態を復元
	if (mini_show == true) {
		mini_window->show();
		ui.toolButton->setIcon(QIcon(":/MouseOverDictionary/Resources/chat-4-line.png"));
	}
	else {
		mini_window->hide();
		ui.toolButton->setIcon(QIcon(":/MouseOverDictionary/Resources/chat-off-line.png"));
	}


	// 画面停止ウィンドウ作成
	//   最小化や最前面設定を独立にしたいので、メインウィンドウの子（new PauseWindow(this);）にはしない
	//   closeEventで削除する
	pause_window = new PauseWindow();

	// 画面停止ウィンドウ設定
	pause_window->setWindowOpacity(pause_window_opacity);
	pause_window->setStyleSheet(QString::fromStdString("QScrollArea {border: " + std::to_string(pause_window_border_width) + "px solid " + pause_window_border_color + ";}"));


	// 検索履歴をダブルクリックで編集できないように設定
	ui.listView->setEditTriggers(QAbstractItemView::NoEditTriggers);

	// 編集履歴のサイズを小さく設定
	ui.splitter->setSizes(QList<int>() << 100 << 300);

	// 検索履歴モデルを設定
	historyModel = new QStringListModel(this);
	QStringList historyList;
	historyModel->setStringList(historyList);
	ui.listView->setModel(historyModel);

	// 前回終了時の検索履歴表示状態を復元
	if (history_show == true) {
		ui.toolButton_3->setIcon(QIcon(":/MouseOverDictionary/Resources/side-bar-line.png"));
		ui.listView->show();
	}
	else {
		ui.toolButton_3->setIcon(QIcon(":/MouseOverDictionary/Resources/side-bar-line-hide.png"));
		ui.listView->hide();
	}


	// 準備完了時にワード欄を有効化
	connect(&thread, SIGNAL(ready(bool)), ui.lineEdit, SLOT(setEnabled(bool)));
	connect(&thread, SIGNAL(ready(bool)), this, SLOT(setReady(bool)));

	// 文字認識の結果をワード欄にセット
	connect(&thread, SIGNAL(wordChanged(QString)), ui.lineEdit, SLOT(setText(QString)));

	// ワード欄が変化した場合（文字認識 or 直接入力 or 履歴クリック）に、辞書を検索する
	connect(ui.lineEdit, SIGNAL(textChanged(QString)), &thread, SLOT(search(QString)));

	// 辞書検索の結果をテキスト欄にセット
	connect(&thread, SIGNAL(mainTextChanged(QString)), ui.textBrowser, SLOT(setHtml(QString)));
	connect(&thread, SIGNAL(miniTextChanged(QString)), mini_window, SLOT(setHtml(QString)));

	// 検索履歴を記録
	connect(&thread, SIGNAL(wordFound(QString)), this, SLOT(updateHistory(QString)));

	// 検索履歴をクリックしたときに、その項目をワード欄にセット
	connect(ui.listView->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
		this, SLOT(searchByHistory(QItemSelection)));

	// クリップボードが変化したときに、その内容をワード欄にセット
	clipboard = QApplication::clipboard();
	connect(clipboard, SIGNAL(dataChanged()), this, SLOT(setFromClipboard()));


	// キーボードショートカット設定
	//   new UGlobalHotkeys(this)として紐づけると、なぜかメインウィンドウの左上部分がクリックできなくなった
	//   なので紐づけずにcloseEvent()で削除することに
	hm_show_hide_both = new UGlobalHotkeys();
	hm_show_hide_both->registerHotkey(hotkey_show_hide_both);
	connect(hm_show_hide_both, &UGlobalHotkeys::activated, [&](size_t id) {
		if (enable_shortcut) {
			showHide();
		}
	});

	hm_show_hide_mini = new UGlobalHotkeys();
	hm_show_hide_mini->registerHotkey(hotkey_show_hide_mini);
	connect(hm_show_hide_mini, &UGlobalHotkeys::activated, [&](size_t id) {
		if (enable_shortcut) {
			showHideMini();
		}
	});

	hm_show_hide_pause = new UGlobalHotkeys();
	hm_show_hide_pause->registerHotkey(hotkey_screen_pause);
	connect(hm_show_hide_pause, &UGlobalHotkeys::activated, [&](size_t id) {
		if (enable_shortcut) {
			showPauseWindow();
		}
	});

	// 文字認識スレッド開始
	thread.start();

}

MouseOverDictionary::~MouseOverDictionary()
{
	// 設定ファイル書き込み
	WriteSettings();
}

void MouseOverDictionary::ReadSettings()
{
	QSettings settings("settings.ini", QSettings::IniFormat);
	settings.setIniCodec(QTextCodec::codecForName("UTF-8"));

	// デザインの設定
	settings.beginGroup("Color");
	main_window_word_font_color  = settings.value("MainWindowWord", "#000088").toString().toStdString();
	main_window_text_font_color  = settings.value("MainWindowText", "#101010").toString().toStdString();
	main_window_mark_font_color  = settings.value("MainWindowMark", "#008000").toString().toStdString();
	main_window_background_color = settings.value("MainWindowBackground", "#ffffff").toString().toStdString();
	mini_window_word_font_color  = settings.value("MiniWindowWord", "#000088").toString().toStdString();
	mini_window_text_font_color  = settings.value("MiniWindowText", "#101010").toString().toStdString();
	mini_window_background_color = settings.value("MiniWindowBackground", "#ffffff").toString().toStdString();
	settings.endGroup();

	settings.beginGroup("FontSize");
	main_window_word_font_size = settings.value("MainWindowWord", 10).toInt();
	main_window_text_font_size = settings.value("MainWindowText", 10).toInt();
	main_window_mark_font_size = settings.value("MainWindowMark", 10).toInt();
	mini_window_word_font_size = settings.value("MiniWindowWord", 10).toInt();
	mini_window_text_font_size = settings.value("MiniWindowText", 10).toInt();
	settings.endGroup();


	// キーボードショートカットの設定
	settings.beginGroup("Shortcut");
	enable_shortcut = settings.value("Enable", true).toBool();
	hotkey_show_hide_both = settings.value("ShowHideBothWindow", "Ctrl+Alt+Z").toString();
	hotkey_show_hide_mini = settings.value("ShowHideMiniWindow", "Ctrl+Alt+X").toString();
	hotkey_screen_pause = settings.value("ScreenPause", "Ctrl+Alt+A").toString();
	settings.endGroup();


	// 各ウィンドウの設定
	settings.beginGroup("Window");
	main_window_opacity = settings.value("MainWindowOpacity", 0.9).toDouble();
	mini_window_opacity = settings.value("MiniWindowOpacity", 0.9).toDouble();
	mini_window_width   = settings.value("MiniWindowWidth", 250).toInt();
	mini_window_height  = settings.value("MiniWindowHeight", 70).toInt();
	mini_window_pos_x   = settings.value("MiniWindowPositionX",  0).toInt(); // マウス追従ウィンドウがカーソルに被るとクリックができなくなるので注意
	mini_window_pos_y   = settings.value("MiniWindowPositionY", 25).toInt();
	settings.endGroup();


	// 画面停止機能の設定
	settings.beginGroup("ScreenPause");
	pause_window_border_width = settings.value("BorderThickness", 1).toInt();
	pause_window_border_color = settings.value("BorderColor", "#ff4500").toString().toStdString();
	pause_window_opacity = settings.value("Opacity", 0.9).toDouble();
	settings.endGroup();


	// 文字認識の設定
	settings.beginGroup("OCR");
	// 画面キャプチャの範囲、カーソルから左右上下のピクセル数、キャプチャ幅 = AreaLeft + AreaRight、キャプチャ高さ = AreaTop + AreaBottom、範囲：1～500
	ocr_area_left   = settings.value("CaptureAreaLeft", 50).toInt();
	ocr_area_right  = settings.value("CaptureAreaRight", 150).toInt();
	ocr_area_top    = settings.value("CaptureAreaTop", 20).toInt();
	ocr_area_bottom = settings.value("CaptureAreaBottom", 20).toInt();
	// 文字認識の精度を上げるためにキャプチャ後の画像を拡大する倍率、100→等倍、150→1.5倍、範囲：100～200
	ocr_scale = settings.value("CaptureScale", 130).toInt();
	settings.endGroup();


	// 前回終了時のウィンドウの情報 // saveGeometry()、restoreGeometry()という機能もあるらしいけど、とりあえずこれで
	settings.beginGroup("PreviousState");
	main_window_width  = settings.value("MainWindowWidth", 350).toInt();
	main_window_height = settings.value("MainWindowHeight", 300).toInt();
	main_window_pos_x  = settings.value("MainWindowPositionX", (screen_width - main_window_width) / 2).toInt(); // デフォルトは画面の中央に
	main_window_pos_y  = settings.value("MainWindowPositionY", (screen_height - main_window_height) / 2).toInt();
	mini_show    = settings.value("ShowMiniWindow", false).toBool();
	history_show = settings.value("ShowHistory", false).toBool();
	stay_top     = settings.value("AlwaysOnTop", true).toBool();
	// MainWindowHistoryWidth


	// 不正な値が指定されてしまわないように、色コードをチェック（要検討）
	std::regex re_color("^#([\\da-fA-F]{6}|[\\da-fA-F]{3})$");
	if (std::regex_match(main_window_word_font_color , re_color) != 1) main_window_word_font_color  = "#000088";
	if (std::regex_match(main_window_text_font_color , re_color) != 1) main_window_text_font_color  = "#101010";
	if (std::regex_match(main_window_mark_font_color , re_color) != 1) main_window_mark_font_color  = "#008000";
	if (std::regex_match(main_window_background_color, re_color) != 1) main_window_background_color = "#ffffff";
	if (std::regex_match(mini_window_word_font_color , re_color) != 1) mini_window_word_font_color  = "#000088";
	if (std::regex_match(mini_window_text_font_color , re_color) != 1) mini_window_text_font_color  = "#101010";
	if (std::regex_match(mini_window_background_color, re_color) != 1) mini_window_background_color = "#ffffff";
	if (std::regex_match(pause_window_border_color   , re_color) != 1) pause_window_border_color    = "#ff4500";

	// 不正な値が指定されてしまわないように、各パラメータを範囲内に収める（要検討）
	main_window_word_font_size = std::clamp(main_window_word_font_size, 3, 32);
	main_window_text_font_size = std::clamp(main_window_text_font_size, 3, 32);
	main_window_mark_font_size = std::clamp(main_window_mark_font_size, 3, 32);
	mini_window_word_font_size = std::clamp(mini_window_word_font_size, 3, 32);
	mini_window_text_font_size = std::clamp(mini_window_text_font_size, 3, 32);
	main_window_opacity  = std::clamp(main_window_opacity,  0.0, 1.0);
	mini_window_opacity  = std::clamp(mini_window_opacity,  0.0, 1.0);
	pause_window_opacity = std::clamp(pause_window_opacity, 0.0, 1.0);
	ocr_scale       = std::clamp(ocr_scale,     100, 200);
	ocr_area_left   = std::clamp(ocr_area_left,   1, 500);
	ocr_area_right  = std::clamp(ocr_area_right,  1, 500);
	ocr_area_top    = std::clamp(ocr_area_top,    1, 500);
	ocr_area_bottom = std::clamp(ocr_area_bottom, 1, 500);
	if (main_window_width < 0) mini_window_width = 0;
	if (main_window_height < 0) mini_window_height = 0;
	if (mini_window_width < 0) mini_window_width = 0;
	if (mini_window_height < 0) mini_window_height = 0;
	if (pause_window_border_width < 0) pause_window_border_width = 0;

	// 各パラメータを文字認識スレッドにセット
	thread.setMainFontColor(main_window_word_font_color, main_window_text_font_color, main_window_mark_font_color, main_window_background_color);
	thread.setMiniFontColor(mini_window_word_font_color, mini_window_text_font_color, mini_window_background_color);
	thread.setMainFontSize(main_window_word_font_size, main_window_text_font_size, main_window_mark_font_size);
	thread.setMiniFontSize(mini_window_word_font_size, mini_window_text_font_size);
	thread.setOcrScale(ocr_scale);
	thread.setOcrRoi(ocr_area_left, ocr_area_right, ocr_area_top, ocr_area_bottom);

}

void MouseOverDictionary::WriteSettings()
{
	QSettings settings("settings.ini", QSettings::IniFormat);
	settings.setIniCodec(QTextCodec::codecForName("UTF-8"));

	settings.beginGroup("Color");
	settings.setValue("MainWindowWord", QString::fromStdString(main_window_word_font_color));
	settings.setValue("MainWindowText", QString::fromStdString(main_window_text_font_color));
	settings.setValue("MainWindowMark", QString::fromStdString(main_window_mark_font_color));
	settings.setValue("MainWindowBackground", QString::fromStdString(main_window_background_color));
	settings.setValue("MiniWindowWord", QString::fromStdString(mini_window_word_font_color));
	settings.setValue("MiniWindowText", QString::fromStdString(mini_window_text_font_color));
	settings.setValue("MiniWindowBackground", QString::fromStdString(mini_window_background_color));
	settings.endGroup();

	settings.beginGroup("FontSize");
	settings.setValue("MainWindowWord", main_window_word_font_size);
	settings.setValue("MainWindowText", main_window_text_font_size);
	settings.setValue("MainWindowMark", main_window_mark_font_size);
	settings.setValue("MiniWindowWord", mini_window_word_font_size);
	settings.setValue("MiniWindowText", mini_window_text_font_size);
	settings.endGroup();

	settings.beginGroup("Shortcut");
	settings.setValue("Enable", enable_shortcut);
	settings.setValue("ShowHideBothWindow", hotkey_show_hide_both);
	settings.setValue("ShowHideMiniWindow", hotkey_show_hide_mini);
	settings.setValue("ScreenPause", hotkey_screen_pause);
	settings.endGroup();

	settings.beginGroup("Window");
	settings.setValue("MainWindowOpacity", main_window_opacity);
	settings.setValue("MiniWindowOpacity", mini_window_opacity);
	settings.setValue("MiniWindowWidth", mini_window_width);
	settings.setValue("MiniWindowHeight", mini_window_height);
	settings.setValue("MiniWindowPositionX", mini_window_pos_x);
	settings.setValue("MiniWindowPositionY", mini_window_pos_y);
	settings.endGroup();

	settings.beginGroup("ScreenPause");
	settings.setValue("BorderThickness", pause_window_border_width);
	settings.setValue("BorderColor", QString::fromStdString(pause_window_border_color));
	settings.setValue("Opacity", pause_window_opacity);
	settings.endGroup();

	settings.beginGroup("OCR");
	settings.setValue("CaptureAreaLeft", ocr_area_left);
	settings.setValue("CaptureAreaRight", ocr_area_right);
	settings.setValue("CaptureAreaTop", ocr_area_top);
	settings.setValue("CaptureAreaBottom", ocr_area_bottom);
	settings.setValue("CaptureScale", ocr_scale);
	settings.endGroup();

	// 前回終了時のウィンドウの情報
	settings.beginGroup("PreviousState");
	settings.setValue("MainWindowWidth", main_window_width);
	settings.setValue("MainWindowHeight", main_window_height);
	settings.setValue("MainWindowPositionX", main_window_pos_x);
	settings.setValue("MainWindowPositionY", main_window_pos_y);
	settings.setValue("ShowMiniWindow", mini_show);
	settings.setValue("ShowHistory", history_show);
	settings.setValue("AlwaysOnTop", stay_top);
	// MainWindowHistoryWidth
	settings.endGroup();

}

void MouseOverDictionary::on_toolButton_clicked()
{
	if (mini_show == false) {
		mini_window->show();
		ui.toolButton->setIcon(QIcon(":/MouseOverDictionary/Resources/chat-4-line.png"));
	}
	else {
		mini_window->hide();
		ui.toolButton->setIcon(QIcon(":/MouseOverDictionary/Resources/chat-off-line.png"));
	}

	mini_show = !mini_show;
}

void MouseOverDictionary::on_toolButton_2_clicked()
{
	// https://stackoverflow.com/questions/2855968/how-do-i-toggle-always-on-top-for-a-qmainwindow-in-qt-without-causing-a-flicke
#ifdef Q_OS_WIN
	if (stay_top == false)
	{
		SetWindowPos((HWND)this->winId(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	}
	else
	{
		SetWindowPos((HWND)this->winId(), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	}
#else
	Qt::WindowFlags flags = this->windowFlags();
	if (stay_top == false)
	{
		this->setWindowFlags(flags | Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint);
		this->show();
	}
	else
	{
		this->setWindowFlags(flags ^ (Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint));
		this->show();
	}
#endif

	if (stay_top == false) {
		ui.toolButton_2->setIcon(QIcon(":/MouseOverDictionary/Resources/push-pin-2-line.png"));
	}
	else {
		ui.toolButton_2->setIcon(QIcon(":/MouseOverDictionary/Resources/push-pin-line.png"));
	}

	stay_top = !stay_top;
}

void MouseOverDictionary::on_toolButton_3_clicked()
{
	if (history_show == false) {
		ui.listView->show();
		ui.toolButton_3->setIcon(QIcon(":/MouseOverDictionary/Resources/side-bar-line.png"));
	}
	else {
		ui.listView->hide();
		ui.toolButton_3->setIcon(QIcon(":/MouseOverDictionary/Resources/side-bar-line-hide.png"));

	}

	history_show = !history_show;
}

void MouseOverDictionary::setReady(bool ready)
{
	thread_ready = ready;
	thread.setWindowSize(this->frameGeometry().width(), this->frameGeometry().height());
	thread.setWindowPos(this->pos().x(), this->pos().y());
}

void MouseOverDictionary::resizeEvent(QResizeEvent *event)
{
	// ウィンドウ内のサイズ
	main_window_width = this->geometry().width();
	main_window_height = this->geometry().height();

	// メインウィンドウ内では文字認識を走らせないようにするために、現在サイズを指定
	if (thread_ready) {
		// ウィンドウの枠なども含めたサイズを指定 https://doc.qt.io/archives/4.3/geometry.html
		thread.setWindowSize(this->frameGeometry().width(), this->frameGeometry().height());
	}
	QWidget::resizeEvent(event);
}

void MouseOverDictionary::moveEvent(QMoveEvent *event)
{
	main_window_pos_x = this->pos().x();
	main_window_pos_y = this->pos().y();

	// メインウィンドウ内では文字認識を走らせないようにするために、現在位置を指定
	if (thread_ready) {
		thread.setWindowPos(main_window_pos_x, main_window_pos_y);
	}
	QWidget::moveEvent(event);
}

void MouseOverDictionary::closeEvent(QCloseEvent *event)
{
	// delete *** よりも ***->deleteLater() の方が安全らしい  https://stackoverflow.com/questions/39407564/difference-between-hide-close-and-show-in-qt
	mini_window->deleteLater();
	pause_window->deleteLater();
	hm_show_hide_both->deleteLater();
	hm_show_hide_mini->deleteLater();
	hm_show_hide_pause->deleteLater();
	QWidget::closeEvent(event);
}

void MouseOverDictionary::changeEvent(QEvent * event)
{
	QMainWindow::changeEvent(event);
	if (event->type() == QEvent::WindowStateChange)
	{

		// メインウィンドウ最小化時に文字読み取りをオフにする
		if (windowState() == Qt::WindowMinimized)
		{
			thread.disableOcr();
			if (mini_show == true) {
				mini_window->hide();
			}

		}
		else if (windowState() == Qt::WindowNoState)
		{
			thread.enableOcr();
			if (mini_show == true) {
				mini_window->show();
			}
		}
	}
}

//void MouseOverDictionary::hideEvent(QHideEvent * event)
//{
//	thread.disableOcr();
//	if (mini_show == true) {
//		mini_window->hide();
//	}
//
//	QWidget::hideEvent(event);
//}
//
//void MouseOverDictionary::showEvent(QShowEvent * event)
//{
//	thread.enableOcr();
//	if (mini_show == true) {
//		mini_window->show();
//	}
//
//	QWidget::showEvent(event);
//}

void MouseOverDictionary::updateHistory(QString word)
{
	// 履歴選択 → ワード欄更新 → 辞書検索 → 履歴更新により選択変化 → ... の無限ループを防ぐためのフラグ（要検討）
	history_search_enable = false;

	if (history_update_enable) {

		//新しい検索語が履歴に含まれている場合は、その項目を削除
		//  →逆に使いにくいかもしれない。要検討
		QModelIndexList indexMatchList = historyModel->match(historyModel->index(0, 0), Qt::DisplayRole, QVariant::fromValue(word), 1);
		if (indexMatchList.size() > 0) {
			historyModel->removeRows(indexMatchList.first().row(), 1); ;
		}

		// リストの一番上に追加
		int row = 0;
		historyModel->insertRows(row, 1);
		QModelIndex index = historyModel->index(row);
		historyModel->setData(index, word);


		// 一番上の項目を選択状態に
		QModelIndex indexOfTheCellIWant = historyModel->index(0, 0);
		ui.listView->setCurrentIndex(indexOfTheCellIWant);


		// 最大数を超える履歴を下から削除
		int history_max = 100;
		if (historyModel->rowCount() > history_max) {
			int row = historyModel->rowCount() - 1;
			historyModel->removeRows(row, 1);
		}

	}
	else {
		history_update_enable = true;
	}

	history_search_enable = true;
}


void MouseOverDictionary::searchByHistory(const QItemSelection &selected)
{

	//履歴選択 → ワード欄更新 → 辞書検索 → 履歴更新により選択変化 → ... の無限ループを防ぐためのフラグ（要検討）
	if (history_search_enable) {
		if (selected.indexes().isEmpty() == false) {

			// 履歴選択による検索の場合は、履歴をアップデートしないように
			history_update_enable = false;

			QString word = historyModel->data(selected.indexes().first()).toString();
			ui.lineEdit->setText(word);
		}
	}

}

void MouseOverDictionary::setFromClipboard()
{
	if (thread_ready) {
		QString clippboard_text = clipboard->text();
		if (clippboard_text.isEmpty() == false) {
			ui.lineEdit->setText(clippboard_text);
		}
	}
}

void MouseOverDictionary::showHide()
{
	if (windowState() == Qt::WindowMinimized)
	{
		this->setWindowState(Qt::WindowNoState);
	}
	else if (windowState() == Qt::WindowNoState)
	{
		this->setWindowState(Qt::WindowMinimized);
	}

	// メインウィンドウを最小化する動きが少しうっとおしいので、show()、hide()にする
	//   ただしこの方法だと、ショートカット以外に再表示させる方法がないので、誤操作が心配かも（最小化ならタスクバーから戻せる）
	//   → タスクマネージャーからも見えなくなる挙動が怖いので、やっぱり最小化に戻す。タスクトレイ常駐を実装したら戻すかも
	//if (this->isVisible()) {
	//	this->hide();
	//}
	//else {
	//	this->show();
	//}
}

void MouseOverDictionary::showHideMini()
{
	// そのうち「アクション」などで整理
	if (mini_show == false) {
		mini_window->show();
		ui.toolButton->setIcon(QIcon(":/MouseOverDictionary/Resources/chat-4-line.png"));
	}
	else {
		mini_window->hide();
		ui.toolButton->setIcon(QIcon(":/MouseOverDictionary/Resources/chat-off-line.png"));
	}

	mini_show = !mini_show;
}

void MouseOverDictionary::showPauseWindow()
{
	if (pause_window->isVisible()) {
		pause_window->hide();
	}
	else {

		// 画面停止ウィンドウ終了後に元のウィンドウをアクティブにするために、現在アクティブなウィンドウを記録
		//   GetActiveWindow()だと別スレッドは取れないので、GetForegroundWindow()を使う
		HWND hWnd;
		hWnd = GetForegroundWindow();
		pause_window->logActiveWindow(hWnd);

		// スクショに映り込まないように一時的に非表示に
		if (windowState() != Qt::WindowMinimized) {
			if (mini_show) {
				mini_window->hide();
			}
			this->hide();
		}

		// カーソルがあるスクリーンを取得
		QScreen * screen = QGuiApplication::screenAt(QCursor::pos());
		if (screen != 0) {

			// 画面停止ウィンドウのScrollAreaの枠の太さだけ内側をスクショ
			QPixmap screenPixmap = QPixmap();
			screenPixmap = screen->grabWindow(0, pause_window_border_width, pause_window_border_width);
			pause_window->setScreenshot(screenPixmap);

			QRect rect = screen->geometry();

			pause_window->show();

			// マウスがある方のスクリーンに移動させる
			//   なぜかshow()の後じゃないと移動できない　https://stackoverflow.com/questions/3203095/display-window-full-screen-on-secondary-monitor-using-qt
			pause_window->move(QPoint(rect.x(), rect.y()));
		}

		// 再表示
		if (windowState() != Qt::WindowMinimized) {
			this->show();
			if (mini_show) {
				mini_window->show();
			}
		}
	}

}