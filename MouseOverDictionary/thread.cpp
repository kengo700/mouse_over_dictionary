#include "thread.h"

Thread::Thread(QObject *parent)
	: QThread(parent)
{
	stopped = false;
}

Thread::~Thread()
{
	mutex.lock();
	stopped = true;
	mutex.unlock();
	wait();
}

void Thread::run()
{
	// Tesseract初期化
	emit mainTextChanged(QString::fromLocal8Bit("文字認識ライブラリ初期化中..."));
	if (ocr.Init() == false) {
		emit mainTextChanged(QString::fromLocal8Bit("文字認識ライブラリ失敗"));
		stopped = true;
	}

	// 辞書の読み込み
	mutex.lock();
	emit mainTextChanged(QString::fromLocal8Bit("辞書データ読み込み中..."));
	if (dict.Load("dictionary") == false) {
		emit mainTextChanged(QString::fromLocal8Bit("辞書データ読み込み失敗"));
		stopped = true;
	};
	mutex.unlock();

	if (!stopped) {
		emit mainTextChanged(QString::fromLocal8Bit("準備完了！"));
		emit ready(true);
	}

	std::string old_recognized_word;
	POINT old_po;
	GetCursorPos(&old_po);

	while (!stopped)
	{
		POINT po;
		GetCursorPos(&po);

		// マウスがメインウィンドウ内にあるなら文字認識は実行しない
		if (window_x < po.x && po.x < window_x + window_w &&
			window_y < po.y && po.y < window_y + window_h) {
			continue;
		}

		// マウスが移動していなければ文字認識は実行しない
		if (po.x == old_po.x && po.y == old_po.y) {
			continue;
		}
		old_po.x = po.x;
		old_po.y = po.y;

		int roi_w = 200;
		int roi_h = 40;
		int roi_mouse_x = roi_w / 4; // 熟語を取得するため、マウスの右側の領域を見る
		int roi_mouse_y = roi_h / 2;

		// マウス付近の画像を文字認識
		if (ocr.Recognize(po.x - roi_mouse_x, po.y - roi_mouse_y, roi_w, roi_h) == false)
		{
			continue;
		}

		std::vector<ocr_result> ocr_results;
		ocr.GetResults(ocr_results);

		if (ocr_results.size() < 1) {
			continue;
		}

		// Y方向にはマウス位置に一番近く、X方向にはワードの幅内にあるワードを取得
		//   単純に一番近いワードを選ぶと、短いワードと長いワードが並んでいる時に別のものを選んでしまうので
		double dist_y_word = 999;
		double temp_y_dist = 0;
		int cx, cy;
		std::string recognized_word;
		int recognized_word_cx;
		int recognized_word_cy;

		for (ocr_result result : ocr_results)
		{
			cx = (result.x1 + result.x2) / 2;
			cy = (result.y1 + result.y2) / 2;
			temp_y_dist = sqrt((roi_mouse_y - cy)*(roi_mouse_y - cy));
			if (temp_y_dist < dist_y_word && (result.x1 < roi_mouse_x && roi_mouse_x < result.x2)) {
				dist_y_word = temp_y_dist;
				recognized_word = result.word;
				recognized_word_cx = cx;
				recognized_word_cy = cy;
			}
		}

		// 熟語に対応するため、選択したワードと同じ高さの右側のワードを追加する
		for (ocr_result result : ocr_results)
		{
			cx = (result.x1 + result.x2) / 2;
			cy = (result.y1 + result.y2) / 2;

			if (abs(recognized_word_cy - cy) < 3) {
				if (recognized_word_cx < cx) {
					recognized_word += " " + result.word;
				}
			}
		}

		// 検出したワードが前回と違う場合のみ更新
		if (recognized_word != old_recognized_word) {
			emit wordChanged(QString::fromStdString(recognized_word));
		}
		old_recognized_word = recognized_word;

	}
	stopped = false;
}

bool Thread::search(QString word)
{
	if (word.isEmpty() || word.isNull()) {
		return false;
	}

	// 前後のホワイトスペースを削除し、ワード間のホワイトスペースが複数個の場合は1つにする
	word = word.simplified();

	// スペースで分割
	QStringList original_words = word.split(" ");

	// 検索するワードの候補
	std::vector<std::string> lookup_words;

	// 前処理
	for (int i = 0; i < original_words.size(); ++i) {

		// 記号を削除
		std::string temp_word = original_words[i].toStdString();
		size_t c;
		while ((c = temp_word.find_first_of(" ,.;:()[]*\"!?")) != std::string::npos) {
			temp_word.erase(c, 1);
		}
		original_words[i] = QString::fromStdString(temp_word);
	}

	// 熟語に対応するため、マウスより右にあるワードを順次追加
	std::string temp_word = "";
	std::vector<std::string> temp_lookup_words;
	for (int i = 0; i < original_words.size(); ++i) {
		temp_word += original_words[i].toStdString();
		temp_lookup_words.push_back(temp_word);
		temp_word += " ";
	}
	// 熟語を先に表示するため、逆順にして結合
	std::reverse(begin(temp_lookup_words), end(temp_lookup_words));
	lookup_words.insert(lookup_words.end(), temp_lookup_words.begin(), temp_lookup_words.end());

	// 小文字にしたバージョンを候補に追加
	QStringList lower_words;
	int lower_words_count;
	for (int i = 0; i < original_words.size(); ++i) {

		// 小文字に変換
		//   #include <regex>でstd::transform()を使うとエラーが出たので、Qtの機能を使う
		//     std::string temp_word = original_words[i].toStdString();
		//     std::transform(lookup_word.begin(), lookup_word.end(), lookup_word.begin(), std::tolower);
		lower_words.push_back(original_words[i].toLower());
	}
	lower_words_count = lower_words.size();

	// 熟語に対応するため、マウスより右にあるワードを順次追加
	temp_word = "";
	temp_lookup_words.clear();
	for (int i = 0; i < lower_words_count; ++i) {
		temp_word += lower_words[i].toStdString();
		temp_lookup_words.push_back(temp_word);
		temp_word += " ";
	}
	// 熟語を先に表示するため、逆順にして結合
	std::reverse(begin(temp_lookup_words), end(temp_lookup_words));
	lookup_words.insert(lookup_words.end(), temp_lookup_words.begin(), temp_lookup_words.end());


	// 複数形を単数形にしたバージョンを候補に追加
	//  名詞以外も変換してしまうので要修正（「this→thi」など）
	//  辞書でヒットしなければ表示されないので、大きな問題はないはず
	QStringList singular_words;
	bool include_plural = false;
	for (int i = 0; i < lower_words_count; ++i) {

		// 単数形を取得
		if (inflect.processPlural(lower_words[i].toStdString(), temp_word)) {
			include_plural = true;
		};
		singular_words.push_back(QString::fromStdString(temp_word));
	}

	// 熟語に対応するため、マウスより右にあるワードを順次追加
	if (include_plural) {
		temp_word = "";
		temp_lookup_words.clear();
		for (int i = 0; i < singular_words.size(); ++i) {
			temp_word += singular_words[i].toStdString();
			temp_lookup_words.push_back(temp_word);
			//sub_lookup_words.push_back(temp_word);
			temp_word += " ";
		}
		// 熟語を先に表示するため、逆順にして結合
		std::reverse(begin(temp_lookup_words), end(temp_lookup_words));
		lookup_words.insert(lookup_words.end(), temp_lookup_words.begin(), temp_lookup_words.end());
	}

	// 過去形を現在形にしたバージョンを候補に追加
	for (int rule_set_num = 0; rule_set_num < 2; rule_set_num++) {
		QStringList presenttense_words;
		bool include_pasttense = false;
		for (int i = 0; i < lower_words_count; ++i) {

			// 単数を取得
			if (inflect.processPastTense(rule_set_num, lower_words[i].toStdString(), temp_word)) {
				include_pasttense = true;
			}
			presenttense_words.push_back(QString::fromStdString(temp_word));
		}

		// 熟語に対応するため、マウスより右にあるワードを順次追加
		if (include_pasttense) {
			temp_word = "";
			temp_lookup_words.clear();
			for (int i = 0; i < presenttense_words.size(); ++i) {
				temp_word += presenttense_words[i].toStdString();
				temp_lookup_words.push_back(temp_word);
				temp_word += " ";
			}
			// 熟語を先に表示するため、逆順にして結合
			std::reverse(begin(temp_lookup_words), end(temp_lookup_words));
			lookup_words.insert(lookup_words.end(), temp_lookup_words.begin(), temp_lookup_words.end());
		}
	}

	// 現在分詞を原型にしたバージョンを候補に追加
	for (int rule_set_num = 0; rule_set_num < 2; rule_set_num++) {
		QStringList infinitive_words;
		bool include_participle = false;
		for (int i = 0; i < lower_words_count; ++i) {

			// 単数を取得
			if (inflect.processParticiple(rule_set_num, lower_words[i].toStdString(), temp_word)) {
				include_participle = true;
			}
			infinitive_words.push_back(QString::fromStdString(temp_word));
		}

		// 熟語に対応するため、マウスより右にあるワードを順次追加
		if (include_participle) {
			temp_word = "";
			temp_lookup_words.clear();
			for (int i = 0; i < infinitive_words.size(); ++i) {
				temp_word += infinitive_words[i].toStdString();
				temp_lookup_words.push_back(temp_word);
				temp_word += " ";
			}
			// 熟語を先に表示するため、逆順にして結合
			std::reverse(begin(temp_lookup_words), end(temp_lookup_words));
			lookup_words.insert(lookup_words.end(), temp_lookup_words.begin(), temp_lookup_words.end());

		}
	}

	// 代名詞をoneやsomeoneにしたバージョンを候補に追加
	for (int rule_set_num = 0; rule_set_num < 3; rule_set_num++) {
		QStringList general_words;
		bool include_pronoun = false;
		for (int i = 0; i < lower_words_count; ++i) {

			// 単数を取得
			if (inflect.processPronoun(rule_set_num, lower_words[i].toStdString(), temp_word)) {
				include_pronoun = true;
			}
			general_words.push_back(QString::fromStdString(temp_word));
		}

		// 熟語に対応するため、マウスより右にあるワードを順次追加
		if (include_pronoun) {
			temp_word = "";
			temp_lookup_words.clear();
			for (int i = 0; i < general_words.size(); ++i) {
				temp_word += general_words[i].toStdString();
				temp_lookup_words.push_back(temp_word);
				temp_word += " ";
			}
			// 熟語を先に表示するため、逆順にして結合
			std::reverse(begin(temp_lookup_words), end(temp_lookup_words));
			lookup_words.insert(lookup_words.end(), temp_lookup_words.begin(), temp_lookup_words.end());

		}
	}


	// 検索するワードの候補を、ソートせずに重複削除
	// Reference: https://dixq.net/forum/viewtopic.php?t=5601
	std::set<std::string> set;
	std::vector<std::string> sorted;
	for (std::vector<std::string>::iterator it = lookup_words.begin(); it != lookup_words.end(); ++it)
	{
		if (set.insert(*it).second)
			sorted.push_back(*it);
	}
	lookup_words = sorted;


	// 辞書から検索
	std::vector<std::string> output_words;
	std::vector<std::string> output_texts;
	bool found = false;
	int found_word_count_max = 0;
	for (std::string lookup_word : lookup_words) {
		std::string text;
		if (dict.Find(lookup_word, text)) {

			output_words.push_back(lookup_word);
			output_texts.push_back(text);

			// 検索履歴に記録する文字列を作るために、検索にヒットしたワード数を記録
			int word_count = std::count(lookup_word.cbegin(), lookup_word.cend(), ' ') + 1;
			if (found_word_count_max < word_count) {
				found_word_count_max = word_count;
			}
			if (original_words.count() < found_word_count_max) {
				found_word_count_max = original_words.count();
			}

			found = true;
		}
	}

	// 検索履歴に記録する文字列を作成
	std::string history_word = "";
	if (found) {
		for (int i = 0; i < found_word_count_max; ++i) {
			history_word += original_words[i].toStdString();
			if (i < found_word_count_max - 1) {
				history_word += " ";
			}
		}
	}

	// HTML形式に変換
	std::string html_main = ""; // メインウィンドウ用
	std::string html_mini = ""; // マウス追従ウィンドウ用

	std::string html_head_l = "<span style = \"color:#000088;font-weight:bold;\">";
	std::string html_head_r = "</span>";
	std::string html_desc_l = "<span style = \"color:#101010;\">";
	std::string html_desc_r = "</span>";
	std::string html_hr = "<hr>";
	std::string html_br = "<br/>";

	for (int i = 0; i < output_words.size(); i++) {

		// 「<」「>」を先に変換
		output_texts[i] = std::regex_replace(output_texts[i], std::regex("\\<"), "&lt;");
		output_texts[i] = std::regex_replace(output_texts[i], std::regex("\\>"), "&gt;");

		// 各ワードの間（2つ目以降のワードの上）に横線を入れる
		if (i > 0) {
			html_main += html_hr;
			html_mini += html_br;
		}

		html_main += html_head_l + output_words[i] + html_head_r + html_br;
		html_main += html_desc_l + output_texts[i] + html_desc_r;

		html_mini += html_head_l + output_words[i] + html_head_r + "：";
		html_mini += html_desc_l + output_texts[i] + html_desc_r;
	}

	// テキストを修飾
	std::vector<std::vector<std::string>> re_rules = {
		{"(■.+?$|◆.+?$|・.+?$)","<span style=\"color:#008000;\">$1</span>"},
		//{"(\\{.+?\\}|\\[.+?\\]|\\(.+?\\))","<span style=\"color:#008000;\">$1</span>"},
		{"(\\{.+?\\}|\\(.+?\\))","<span style=\"color:#008000;\">$1</span>"},
		{"(【.+?】|《.+?》|〈.+?〉|〔.+?〕)","<span style=\"color:#008000;\">$1</span>"},
		{"\\r\\n|\\n|\\r", "<br/>"}
	};
	for (auto re_rule : re_rules) {
		html_main = std::regex_replace(html_main, std::regex(re_rule[0]), re_rule[1]);
	}

	std::vector<std::vector<std::string>> re_rules_popup = {
		{"(■.+?$|◆.+?$|・.+?$)",""},
		//{"(\\{.+?\\}|\\[.+?\\]|\\(.+?\\))",""},
		{"(\\{.+?\\}|\\(.+?\\))",""},
		{"(【.+?】|《.+?》|〈.+?〉|〔.+?〕)",""},
		{"\\r\\n|\\n|\\r", "；"}
	};
	for (auto re_rule : re_rules_popup) {
		html_mini = std::regex_replace(html_mini, std::regex(re_rule[0]), re_rule[1]);
	}

	// 結果をシグナル経由で伝達
	mutex.lock();
	if (found) {
		emit mainTextChanged(QString::fromLocal8Bit(html_main.c_str()));
		emit miniTextChanged(QString::fromLocal8Bit(html_mini.c_str()));
		emit wordFound(QString::fromStdString(history_word));
	}
	mutex.unlock();

	return true;
}

void Thread::stop()
{
	mutex.lock();
	stopped = true;
	mutex.unlock();
}

void Thread::setWindowPos(int x, int y)
{
	mutex.lock();
	window_x = x;
	window_y = y;
	mutex.unlock();
}

void Thread::setWindowSize(int w, int h)
{
	mutex.lock();
	window_w = w;
	window_h = h;
	mutex.unlock();
}
