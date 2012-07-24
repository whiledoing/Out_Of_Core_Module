#include "readingbigimage.h"
#include <QtGui/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	ReadingBigImage w;
	w.show();
	return a.exec();
}
