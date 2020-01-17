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
	stopped = false;

	std::string message_style = "<head><style type=\"text/css\">";
	message_style += "body {background-color:" + main_window_background_color + "}";
	message_style += ".maintext {color:" + main_window_text_font_color + ";font-size:" + std::to_string(main_window_text_font_size) + "pt;}";
	message_style += "</style></head>";

	std::string message_l = "<body><span class=\"maintext\">";
	std::string message_r = "</span></body>";

	std::string message_tesseract_loading  = message_style + message_l + u8"文字認識ライブラリ初期化中..." + message_r;
	std::string message_tesseract_ng       = message_style + message_l + u8"文字認識ライブラリ初期化失敗" + message_r;
	std::string message_tesseract_ok       = message_style + message_l + u8"文字認識ライブラリ初期化成功" + message_r;

	std::string message_dictionary_loading = message_style + message_l + u8"辞書データ読み込み中..." + message_r;
	std::string message_dictionary_ng      = message_style + message_l + u8"辞書データ読み込み失敗" + message_r;
	std::string message_dictionary_ok      = message_style + message_l + u8"辞書データ読み込み成功" + message_r;

	std::string message_ready              = message_style + message_l + u8"準備完了！" + message_r;

	// Tesseract初期化
	emit mainTextChanged(QString::fromUtf8(message_tesseract_loading.c_str()));
	if (ocr.Init() == false) {
		emit mainTextChanged(QString::fromUtf8(message_tesseract_ng.c_str()));
		stopped = true;
	}
	else {
		emit mainTextChanged(QString::fromUtf8(message_tesseract_ok.c_str()));
	}

	// 辞書の読み込み
	if(!stopped){
		mutex.lock();
		emit mainTextChanged(QString::fromUtf8(message_dictionary_loading.c_str()));
		if (dict.Load("dictionary") == false) {
			emit mainTextChanged(QString::fromUtf8(message_dictionary_ng.c_str()));
			stopped = true;
		}
		else {
			emit mainTextChanged(QString::fromUtf8(message_dictionary_ok.c_str()));
		}
		mutex.unlock();
	}

	if (!stopped) {
		emit mainTextChanged(QString::fromUtf8(message_ready.c_str()));
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
			Sleep(1); // コンテキストスイッチを発生させてCPU使用率を下げる(Issue#2)
			continue;
		}

		// マウスが移動していなければ文字認識は実行しない
		if (po.x == old_po.x && po.y == old_po.y) {
			Sleep(1); // コンテキストスイッチを発生させてCPU使用率を下げる(Issue#2)
			continue;
		}
		old_po.x = po.x;
		old_po.y = po.y;

		// マウス付近の画像を文字認識
		if (ocr.Recognize(po.x - roi_mouse_x, po.y - roi_mouse_y, roi_w, roi_h, ocr_scale) == false)
		{
			Sleep(1); // コンテキストスイッチを発生させてCPU使用率を下げる(Issue#2)
			continue;
		}

		std::vector<ocr_result> ocr_results;
		ocr.GetResults(ocr_results);

		if (ocr_results.size() < 1) {
			Sleep(1); // コンテキストスイッチを発生させてCPU使用率を下げる(Issue#2)
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

	// 検索するワードの候補
	std::vector<std::string> lookup_words;

	// 前後のホワイトスペースを削除し、ワード間のホワイトスペースが複数個の場合は1つにする
	word = word.simplified();


	// スペースで分割しただけのものを候補に追加
	QStringList spaced_words = word.split(" ");

	// 熟語に対応するため、マウスより右にあるワードを順次追加
	std::string temp_word = u8"";
	std::vector<std::string> temp_lookup_words;
	for (int i = 0; i < spaced_words.size(); ++i) {
		temp_word += spaced_words[i].toStdString();
		temp_lookup_words.push_back(temp_word);
		temp_word += u8" ";
	}
	// 熟語を先に表示するため、逆順にして結合
	std::reverse(begin(temp_lookup_words), end(temp_lookup_words));
	lookup_words.insert(lookup_words.end(), temp_lookup_words.begin(), temp_lookup_words.end());


	// QTextBoundaryFinderを使用して、ワードのみを抽出したものを候補に追加
	QStringList original_words;
	auto finder = new QTextBoundaryFinder(QTextBoundaryFinder::Word, word);
	int prevPos = 0;
	while (finder->toNextBoundary() != -1) {
		// toNextBoundary()後に確認しているため、ワードの終わりであるか（EndOfItem）を見ている
		auto reason = finder->boundaryReasons();
		if (reason.testFlag(QTextBoundaryFinder::BreakOpportunity) && reason.testFlag(QTextBoundaryFinder::EndOfItem)) {
			original_words.append(word.mid(prevPos, finder->position() - prevPos));
		}
		prevPos = finder->position();
	}

	// 熟語に対応するため、マウスより右にあるワードを順次追加
	for (int i = 0; i < original_words.size(); ++i) {
		temp_word += original_words[i].toStdString();
		temp_lookup_words.push_back(temp_word);
		temp_word += u8" ";
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
	temp_word = u8"";
	temp_lookup_words.clear();
	for (int i = 0; i < lower_words_count; ++i) {
		temp_word += lower_words[i].toStdString();
		temp_lookup_words.push_back(temp_word);
		temp_word += u8" ";
	}
	// 熟語を先に表示するため、逆順にして結合
	std::reverse(begin(temp_lookup_words), end(temp_lookup_words));
	lookup_words.insert(lookup_words.end(), temp_lookup_words.begin(), temp_lookup_words.end());


	// 複数形を単数形にしたバージョンを候補に追加
	//  名詞以外も変換してしまうので要修正（「this→thi」など）
	//  辞書でヒットしなければ表示されないので、大きな問題はないはず
	for (int rule_set_num = 0; rule_set_num < 3; rule_set_num++) {
		QStringList singular_words;
		bool include_plural = false;
		for (int i = 0; i < lower_words_count; ++i) {

			// 単数形を取得
			if (inflect.processPlural(rule_set_num, lower_words[i].toStdString(), temp_word)) {
				include_plural = true;
			};
			singular_words.push_back(QString::fromStdString(temp_word));
		}

		// 熟語に対応するため、マウスより右にあるワードを順次追加
		if (include_plural) {
			temp_word = u8"";
			temp_lookup_words.clear();
			for (int i = 0; i < singular_words.size(); ++i) {
				temp_word += singular_words[i].toStdString();
				temp_lookup_words.push_back(temp_word);
				temp_word += u8" ";
			}
			// 熟語を先に表示するため、逆順にして結合
			std::reverse(begin(temp_lookup_words), end(temp_lookup_words));
			lookup_words.insert(lookup_words.end(), temp_lookup_words.begin(), temp_lookup_words.end());
		}
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
			temp_word = u8"";
			temp_lookup_words.clear();
			for (int i = 0; i < presenttense_words.size(); ++i) {
				temp_word += presenttense_words[i].toStdString();
				temp_lookup_words.push_back(temp_word);
				temp_word += u8" ";
			}
			// 熟語を先に表示するため、逆順にして結合
			std::reverse(begin(temp_lookup_words), end(temp_lookup_words));
			lookup_words.insert(lookup_words.end(), temp_lookup_words.begin(), temp_lookup_words.end());
		}
	}


	// 現在分詞を原型にしたバージョンを候補に追加
	for (int rule_set_num = 0; rule_set_num < 3; rule_set_num++) {
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
			temp_word = u8"";
			temp_lookup_words.clear();
			for (int i = 0; i < infinitive_words.size(); ++i) {
				temp_word += infinitive_words[i].toStdString();
				temp_lookup_words.push_back(temp_word);
				temp_word += u8" ";
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
			temp_word = u8"";
			temp_lookup_words.clear();
			for (int i = 0; i < general_words.size(); ++i) {
				temp_word += general_words[i].toStdString();
				temp_lookup_words.push_back(temp_word);
				temp_word += u8" ";
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
			int word_count = std::count(lookup_word.cbegin(), lookup_word.cend(), u8' ') + 1;
			if (found_word_count_max < word_count) {
				found_word_count_max = word_count;
			}
			if (original_words.count() < found_word_count_max) {
				found_word_count_max = original_words.count();
			}

			// 英辞郎のハイパーリンク機能書式にヒットする用語を追加で検索
			std::smatch link_words;
			if (std::regex_search(text, link_words, std::regex(u8"\\<→.+?\\>")))
			{
				std::string link_word = link_words[0].str();
				link_word.erase(link_word.begin(), link_word.begin() + 4); // 「<→」削除
				link_word.pop_back(); // 「>」削除

				if (dict.Find(link_word, text)) {
					output_words.push_back(link_word);
					output_texts.push_back(text);
				}
			}
			// 矢印だけの場合もあるっぽい
			else if (std::regex_search(text, link_words, std::regex(u8"→.+?$")))
			{
				std::string link_word = link_words[0].str();
				link_word.erase(link_word.begin(), link_word.begin() + 3); // 「→」削除

				if (dict.Find(link_word, text)) {
					output_words.push_back(link_word);
					output_texts.push_back(text);
				}
			}

			// ejdic形式では「=」で表現されている
			//  他の辞書形式の文中の「=」も拾ってしまうが、検索にヒットしなければ表示されないのでよしとする
			if (std::regex_search(text, link_words, std::regex(u8"=.+?$")))
			{
				std::string link_word = link_words[0].str();
				link_word.erase(link_word.begin(), link_word.begin() + 1); // 「=」削除

				// 前後のホワイトスペースを削除し、ワード間のホワイトスペースが複数個の場合は1つにする
				QString temp = QString::fromStdString(link_word);
				temp = temp.simplified();
				link_word = temp.toStdString();

				if (dict.Find(link_word, text)) {
					output_words.push_back(link_word);
					output_texts.push_back(text);
				}
			}


			found = true;
		}
	}

	if (found == false) {
		return false;
	}

	// 検索履歴に記録する文字列を作成
	std::string history_word = u8"";
	if (found) {
		for (int i = 0; i < found_word_count_max; ++i) {
			history_word += original_words[i].toStdString();
			if (i < found_word_count_max - 1) {
				history_word += u8" ";
			}
		}
	}

	// HTML形式に変換
	std::string html_main_style = "<head><style type=\"text/css\">";
	html_main_style += "body {background-color:" + main_window_background_color + "}";
	html_main_style += ".mainword {color:" + main_window_word_font_color + ";font-size:" + std::to_string(main_window_word_font_size) + "pt;font-weight:bold;}";
	html_main_style += ".maintext {color:" + main_window_text_font_color + ";font-size:" + std::to_string(main_window_text_font_size) + "pt;}";
	html_main_style += ".mainmark {color:" + main_window_mark_font_color + ";font-size:" + std::to_string(main_window_mark_font_size) + "pt;}";
	html_main_style += "</style></head><body>";

	std::string html_mini_style = "<head><style type=\"text/css\">";
	html_mini_style += "body {background-color:" + mini_window_background_color + "}";
	html_mini_style += ".miniword {color:" + mini_window_word_font_color + ";font-size:" + std::to_string(mini_window_word_font_size) + "pt;font-weight:bold;}";
	html_mini_style += ".minitext {color:" + mini_window_text_font_color + ";font-size:" + std::to_string(mini_window_text_font_size) + "pt;}";
	html_mini_style += "</style></head><body>";

	std::string html_main_head = "<span class=\"mainword\">";
	std::string html_main_desc = "<span class=\"maintext\">";
	std::string html_mini_head = "<span class=\"miniword\">";
	std::string html_mini_desc = "<span class=\"minitext\">";

	std::string html_main = ""; // メインウィンドウ用
	std::string html_mini = ""; // マウス追従ウィンドウ用

	for (int i = 0; i < output_words.size(); i++) {

		// 「<」「>」を先に変換
		output_texts[i] = std::regex_replace(output_texts[i], std::regex(u8"\\<"), u8"&lt;");
		output_texts[i] = std::regex_replace(output_texts[i], std::regex(u8"\\>"), u8"&gt;");

		// 各ワードの間（2つ目以降のワードの上）に横線を入れる
		if (i > 0) {
			html_main += "<hr>";
			html_mini += "<br/>";
		}

		html_main += html_main_head + output_words[i] + "</span>" + "<br/>";
		html_main += html_main_desc + output_texts[i] + "</span>";

		html_mini += html_mini_head + output_words[i] + "</span>" + u8"：";
		html_mini += html_mini_desc + output_texts[i] + "</span>";
	}

	// テキストを修飾
	std::vector<std::vector<std::string>> re_main_rules = {
		{u8"(■.+?$|◆.+?$)","<span class=\"mainmark\">$1</span>"},
		{u8"(\\{.+?\\}|\\(.+?\\))","<span class=\"mainmark\">$1</span>"},
		{u8"(【.+?】|《.+?》|〈.+?〉|〔.+?〕)","<span class=\"mainmark\">$1</span>"},
		{u8"\\r\\n|\\n|\\r", "<br/>"}
	};
	for (auto re_rule : re_main_rules) {
		html_main = std::regex_replace(html_main, std::regex(re_rule[0]), re_rule[1]);
	}

	std::vector<std::vector<std::string>> re_mini_rules = {
		{u8"(■.+?$|◆.+?$)",""},
		{u8"(\\{.+?\\}|\\(.+?\\))",""},
		{u8"(【.+?】|《.+?》|〈.+?〉|〔.+?〕)",""},
		{u8"\\r\\n|\\n|\\r", u8"；"}
	};
	for (auto re_rule : re_mini_rules) {
		html_mini = std::regex_replace(html_mini, std::regex(re_rule[0]), re_rule[1]);
	}

	// スタイル部分を付与（先につけておくと「{」「}」が置換されてしまうので最後に）
	html_main = html_main_style + html_main + "</body>";
	html_mini = html_mini_style + html_mini + "</body>";


	// 結果をシグナル経由で伝達
	mutex.lock();
	emit mainTextChanged(QString::fromUtf8(html_main.c_str()));
	emit miniTextChanged(QString::fromUtf8(html_mini.c_str()));
	emit wordFound(QString::fromStdString(history_word));
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

void Thread::setOcrScale(int ocr_scale)
{
	this->ocr_scale = ocr_scale;
}

void Thread::setOcrRoi(int left, int right, int top, int bottom)
{
	roi_w = left + right;
	roi_h = top + bottom;
	roi_mouse_x = left;
	roi_mouse_y = top;
}

void Thread::setMainFontColor(std::string word, std::string text, std::string mark, std::string background)
{
	main_window_word_font_color = word;
	main_window_text_font_color = text;
	main_window_mark_font_color = mark;
	main_window_background_color = background;
}

void Thread::setMiniFontColor(std::string word, std::string text, std::string background)
{
	mini_window_word_font_color = word;
	mini_window_text_font_color = text;
	mini_window_background_color = background;
}

void Thread::setMainFontSize(int word, int text, int mark)
{
	main_window_word_font_size = word;
	main_window_text_font_size = text;
	main_window_mark_font_size = mark;
}

void Thread::setMiniFontSize(int word, int text)
{
	mini_window_word_font_size = word;
	mini_window_text_font_size = text;
}
