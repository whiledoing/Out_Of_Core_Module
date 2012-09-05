#include "readingbigimage.h"
#include <QtGui/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	ReadingBigImage w;
	w.show();
    if(argc > 0) 
        w.open_file_name(QString(argv[1]));

	return a.exec();
}
