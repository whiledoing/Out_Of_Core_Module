#include "readingbigimage.h"
#include "qbigimagewidget.h"

#include <Qtgui/QMessageBox>
#include <Qtgui/QFileDialog>
#include <Qtgui/QMouseEvent>
#include <Qtgui/QPaintEvent>
#include <Qtgui/QPixmap>
#include <Qtgui/QPainter>

#include <iostream>
using namespace std;

ReadingBigImage::ReadingBigImage(QWidget *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags)
{
	ui.setupUi(this);

	m_central_widget = new QBigImageWidget(this);
	setCentralWidget(m_central_widget);
	setStatusBar(NULL);

	connect(ui.m_action_open_file, SIGNAL(triggered()), this, SLOT(on_open_file_name()));
}

ReadingBigImage::~ReadingBigImage()
{

}

void ReadingBigImage::on_open_file_name()
{
	QString file_name = QFileDialog::getOpenFileName(this, "Open Bigimage File", ".", "*.bigimage");

	/* if get the reasonable file name */
	if(!file_name.isEmpty()){
		if(!m_central_widget->load_big_image(file_name)) {
			QMessageBox::warning(this, "ReadingBigImage", QString(file_name) + " is not a support big image type"); 
			return;
		}
	}
}

void ReadingBigImage::mouseMoveEvent(QMouseEvent * event)
{

}

void ReadingBigImage::mousePressEvent(QMouseEvent * event)
{

}

void ReadingBigImage::mouseReleaseEvent(QMouseEvent * event)
{

}

void ReadingBigImage::paintEvent(QPaintEvent * event)
{

}

void ReadingBigImage::resizeEvent(QResizeEvent * event)
{

}
