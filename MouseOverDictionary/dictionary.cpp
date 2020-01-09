#include "dictionary.h"

Dictionary::Dictionary()
{

}

bool Dictionary::Load(std::string foldername)
{
	// 指定フォルダ内のテキストファイル名を取得
	QString path = QApplication::applicationDirPath() + "/" + QString::fromStdString(foldername) + "/";

	QStringList nameFilters;
	nameFilters.append("*.txt");
	nameFilters.append("*.TXT");

	QDir dir(path);
	QStringList files = dir.entryList(nameFilters, QDir::Files);

	// 各ファイルのフォーマットを確認
	bool read_success = false;
	for (QString file : files) {
		std::string filename = dir.filePath(file).toStdString();

		// データ読み込み
		switch (getFormat(filename))
		{
		case EIJIRO:
			if (LoadEIJIRO(filename)) {
				read_success = true;
			}
			break;
		case EJDIC:
			if (LoadEJDIC(filename)) {
				read_success = true;
			}
			break;
		case PDIC1:
			if (LoadPDIC1(filename)) {
				read_success = true;
			}
			break;
		default:
			break;
		}
	}

	return read_success;
}

// ejdic-hand形式の辞書データを読み込む
//   ejdic-hand形式について：https://github.com/kujirahand/EJDict
bool Dictionary::LoadEJDIC(std::string filename)
{
	QTextCodec *codec = QTextCodec::codecForName("Shift-JIS");

	// ファイルを開く
	//   UTF-8のファイルはin.setCodec()で指定しないと文字化けする
	QFile file(filename.c_str());
	if (!file.open(QIODevice::ReadOnly)) {
		return false;
	}
	QTextStream in(&file);
	in.setCodec("UTF-8");

	QString line;
	while (!in.atEnd()) {
		line = in.readLine();

		QStringList items = line.split(codec->toUnicode("\t"));

		if (items.size() < 2) {
			continue;
		}

		// 意味が同じで綴りが少し異なるだけの単語はカンマで区切って列挙されているので、それぞれ登録する
		QStringList words = items[0].split(codec->toUnicode(","));

		std::string text = codec->fromUnicode(items[1]).toStdString();

		for (int i = 0; i < words.count(); i++) {
			std::string word = codec->fromUnicode(words[i]).toStdString();
			// 読み込み済みのワードは追記する
			if (data.find(word) != data.end()) {
				data[word] = data[word] + "\n" + text;
			}
			else {
				data[word] = text;
			}
		}
	}

	file.close();

	return true;
}

// 辞郎形式の辞書データを読み込む
//   辞郎形式について：http://www.eijiro.jp/spec.htm
bool Dictionary::LoadEIJIRO(std::string filename)
{

	QTextCodec *codec = QTextCodec::codecForName("Shift-JIS");

	// ファイルを開く
	//   in.setCodec()で指定しなくても、Shift-JISとUnicodeのファイルは正しく読めるっぽい
	QFile file(filename.c_str());
	if (!file.open(QIODevice::ReadOnly)) {
		return false;
	}
	QTextStream in(&file);

	// 一行ずつ処理
	//   in.readAll()で一括で読み込んでsplit("\n")で分割してforeachで処理する方法も試してみたところ、倍以上遅かった
	QString line;
	while (!in.atEnd()) {
		line = in.readLine();

		// 用語と訳語の区切り「 : 」
		QStringList items = line.split(codec->toUnicode(" : "));

		if (items.size() < 2) {
			continue;
		}

		// 先頭の「■」を削除
		items[0].remove(codec->toUnicode("■"));

		// 品詞ラベルなどを削除
		QStringList items2 = items[0].split(codec->toUnicode("{"));
		items[0] = items2[0];

		// 前後の半角スペースを削除
		items[0] = items[0].trimmed();

		// 品詞ラベルなどは訳語の先頭に付けておく
		if (items2.size() >= 2) {
			items[1] = codec->toUnicode("{") + items2[1] + items[1];
		}

		std::string word = codec->fromUnicode(items[0]).toStdString();
		std::string text = codec->fromUnicode(items[1]).toStdString();

		// 読み込み済みのワードは追記する
		if (data.find(word) != data.end()) {
			data[word] = data[word] + "\n" + text;
		}
		else {
			data[word] = text;
		}
	}

	file.close();

	return true;
}

// PDIC1行テキスト形式の辞書データを読み込む
//   PDIC1行テキスト形式について：http://pdic.la.coocan.jp/unicode/help/OneLineFormat.html
bool Dictionary::LoadPDIC1(std::string filename)
{
	QTextCodec *codec = QTextCodec::codecForName("Shift-JIS");

	// ファイルを開く
	//   in.setCodec()で指定しなくても、Shift-JISとUnicodeのファイルは正しく読めるっぽい
	QFile file(filename.c_str());
	if (!file.open(QIODevice::ReadOnly)) {
		return false;
	}
	QTextStream in(&file);

	QString line;
	while (!in.atEnd()) {
		line = in.readLine();

		QStringList items = line.split(codec->toUnicode(" /// "));

		if (items.size() < 2) {
			continue;
		}

		// 改行指定「 \ 」を改行に置換
		items[1] = items[1].replace(codec->toUnicode(" \\ "), codec->toUnicode("\n"));

		// 訳語と用例の区切りは、辞郎フォーマットに合わせて「■・」に置換しておく（要検討）
		items[1] = items[1].replace(codec->toUnicode(" / "), codec->toUnicode("■・"));

		std::string word = codec->fromUnicode(items[0]).toStdString();
		std::string text = codec->fromUnicode(items[1]).toStdString();

		// 読み込み済みのワードは追記する
		if (data.find(word) != data.end()) {
			data[word] = data[word] + "\n" + text;
		}
		else {
			data[word] = text;
		}
	}

	file.close();

	return true;
}

bool Dictionary::Find(std::string word, std::string& text)
{
	if (word != "" && data.find(word) != data.end()) {
		text = data[word];
		return true;
	}
	return false;
}

enum DictionaryFormat Dictionary::getFormat(std::string filename)
{

	QTextCodec *codec = QTextCodec::codecForName("Shift-JIS");

	// ファイルを開く
	//   C++だとUnicodeのファイル読み込みが難しいので、Qtで読み込み
	QFile file(filename.c_str());
	if (!file.open(QIODevice::ReadOnly)) {
		return OTHER;
	}
	QTextStream in(&file);

	// 一行目を読み込み
	DictionaryFormat format;
	QString first_line = in.readLine();

	// 1行目に「■」を含む場合は辞郎フォーマットと判断する（要検討）
	if (first_line.contains(codec->toUnicode("■"))) {
		format = EIJIRO;
	}
	// 1行目にタブを含む場合はejdicフォーマットと判断する（要検討）
	else if (first_line.contains(codec->toUnicode("\t"))) {
		format = EJDIC;
	}
	// 1行目に「///」を含む場合はPDIC1行テキストフォーマットと判断する（要検討）
	else if (first_line.contains(codec->toUnicode("///"))) {
		format = PDIC1;
	}
	else {
		format = OTHER;
	}

	file.close();
	return format;
}