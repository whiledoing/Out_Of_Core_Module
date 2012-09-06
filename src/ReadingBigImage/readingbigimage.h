#ifndef READINGBIGIMAGE_H
#define READINGBIGIMAGE_H

#include <QtGui/QMainWindow>
#include <Qtgui/QWidget>
#include "ui_readingbigimage.h"

class QMouseEvent;
class QPaintEvent;
class QBigImageWidget;
class QDockWidget;
class DockLabel;

class ReadingBigImage : public QMainWindow
{
	Q_OBJECT

public:
	ReadingBigImage(QWidget *parent = 0, Qt::WFlags flags = 0);
	~ReadingBigImage();
    void open_file_name(QString file_name);

public slots:
	void on_open_file_name();

protected:
	virtual void mouseMoveEvent ( QMouseEvent * event );
	virtual void mousePressEvent ( QMouseEvent * event );
	virtual void mouseReleaseEvent ( QMouseEvent * event );
	virtual void paintEvent ( QPaintEvent * event );
	virtual void resizeEvent ( QResizeEvent * event );

private:
	Ui::ReadingBigImageClass ui;
	QBigImageWidget *m_central_widget;
    QString m_windowname;

private:
    DockLabel *m_dock_label;
    QDockWidget *m_dock_widget;
};

#endif // READINGBIGIMAGE_H
