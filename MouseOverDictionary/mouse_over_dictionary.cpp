#include "mouse_over_dictionary.h"

MouseOverDictionary::MouseOverDictionary(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

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
