#include "mouse_over_dictionary.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	// 高DPI用の設定
	QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

	QApplication a(argc, argv);
	MouseOverDictionary w;

	w.show();
	return a.exec();
}