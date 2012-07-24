#ifndef READINGBIGIMAGE_H
#define READINGBIGIMAGE_H

#include <QtGui/QMainWindow>
#include <Qtgui/QWidget>
#include "ui_readingbigimage.h"

class QMouseEvent;
class QPaintEvent;
class QBigImageWidget;

class ReadingBigImage : public QMainWindow
{
	Q_OBJECT

public:
	ReadingBigImage(QWidget *parent = 0, Qt::WFlags flags = 0);
	~ReadingBigImage();

public slots:
	void on_open_file_name();

protected:
	virtual void	mouseMoveEvent ( QMouseEvent * event );
	virtual void	mousePressEvent ( QMouseEvent * event );
	virtual void	mouseReleaseEvent ( QMouseEvent * event );
	virtual void	paintEvent ( QPaintEvent * event );
	void resizeEvent ( QResizeEvent * event );
private:
	Ui::ReadingBigImageClass ui;
	QBigImageWidget *m_central_widget;
};

#endif // READINGBIGIMAGE_H
