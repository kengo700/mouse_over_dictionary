#include "mouse_over_dictionary.h"

MouseOverDictionary::MouseOverDictionary(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	// 設定ファイル読み込み
	ReadSettings();


	// 最初から最前面に表示
	this->setWindowFlags(Qt::WindowStaysOnTopHint);

	// 最初から最前面ボタンをオンに
	ui.toolButton_2->setChecked(true);

	// 最初は検索履歴を非表示に
	ui.listView->hide();

	// 検索履歴をダブルクリックで編集できないように設定
	ui.listView->setEditTriggers(QAbstractItemView::NoEditTriggers);

	// 編集履歴のサイズを小さく設定
	ui.splitter->setSizes(QList<int>() << 100 << 300);

	// マウス追従ウィンドウ作成
	//   最小化や最前面設定を独立にしたいので、メインウィンドウの子（new MiniWindow(this);）にはしない
	//   closeEventで削除する
	mini_window = new MiniWindow();

	// マウス追従ウィンドウを最前面、枠なしに指定
	mini_window->setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

	// マウス追従ウィンドウをカーソルに追従
	auto timer = new QTimer(this);
	timer->setInterval(15);
	timer->setSingleShot(false);
	connect(timer, SIGNAL(timeout()), mini_window, SLOT(followCursor()));
	timer->start();

	// 検索履歴モデルを設定
	historyModel = new QStringListModel(this);
	QStringList historyList;
	historyModel->setStringList(historyList);
	ui.listView->setModel(historyModel);

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

	// ウィンドウの設定
	settings.beginGroup("Window");
	// MainWindowWidth
	// MainWindowHeight
	// MainWindowHistoryWidth
	// MiniWindowWidth
	// MiniWindowHeight
	// MiniWindowPositionX
	// MiniWindowPositionY

	// ShowMiniWindow
	// ShowHistory
	// AlwaysOnTop
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


	// 不正な値が指定されてしまわないように、色コードをチェック（要検討）
	std::regex re_color("^#([\\da-fA-F]{6}|[\\da-fA-F]{3})$");
	if (std::regex_match(main_window_word_font_color , re_color) != 1) main_window_word_font_color  = "#000088";
	if (std::regex_match(main_window_text_font_color , re_color) != 1) main_window_text_font_color  = "#101010";
	if (std::regex_match(main_window_mark_font_color , re_color) != 1) main_window_mark_font_color  = "#008000";
	if (std::regex_match(main_window_background_color, re_color) != 1) main_window_background_color = "#ffffff";
	if (std::regex_match(mini_window_word_font_color , re_color) != 1) mini_window_word_font_color  = "#000088";
	if (std::regex_match(mini_window_text_font_color , re_color) != 1) mini_window_text_font_color  = "#101010";
	if (std::regex_match(mini_window_background_color, re_color) != 1) mini_window_background_color = "#ffffff";

	// 不正な値が指定されてしまわないように、各パラメータを範囲内に収める（要検討）
	main_window_word_font_size = std::clamp(main_window_word_font_size, 3, 32);
	main_window_text_font_size = std::clamp(main_window_text_font_size, 3, 32);
	main_window_mark_font_size = std::clamp(main_window_mark_font_size, 3, 32);
	mini_window_word_font_size = std::clamp(mini_window_word_font_size, 3, 32);
	mini_window_text_font_size = std::clamp(mini_window_text_font_size, 3, 32);
	ocr_scale       = std::clamp(ocr_scale,     100, 200);
	ocr_area_left   = std::clamp(ocr_area_left,   1, 500);
	ocr_area_right  = std::clamp(ocr_area_right,  1, 500);
	ocr_area_top    = std::clamp(ocr_area_top,    1, 500);
	ocr_area_bottom = std::clamp(ocr_area_bottom, 1, 500);

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

	settings.beginGroup("Window");
	// MainWindowWidth
	// MainWindowHeight
	// MainWindowHistoryWidth
	// MiniWindowWidth
	// MiniWindowHeight
	// MiniWindowPositionX
	// MiniWindowPositionY

	// ShowMiniWindow
	// ShowHistory
	// AlwaysOnTop
	settings.endGroup();

	settings.beginGroup("OCR");
	settings.setValue("CaptureAreaLeft", ocr_area_left);
	settings.setValue("CaptureAreaRight", ocr_area_right);
	settings.setValue("CaptureAreaTop", ocr_area_top);
	settings.setValue("CaptureAreaBottom", ocr_area_bottom);
	settings.setValue("CaptureScale", ocr_scale);
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
	if (thread_ready) {
		// https://doc.qt.io/archives/4.3/geometry.html
		thread.setWindowSize(this->frameGeometry().width(), this->frameGeometry().height());
	}
	QWidget::resizeEvent(event);
}

void MouseOverDictionary::moveEvent(QMoveEvent *event)
{
	if (thread_ready) {
		thread.setWindowPos(this->pos().x(), this->pos().y());
	}
	QWidget::moveEvent(event);
}

void MouseOverDictionary::closeEvent(QCloseEvent *event)
{
	delete mini_window;
	QWidget::closeEvent(event);
}

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
