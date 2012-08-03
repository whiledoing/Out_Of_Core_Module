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

		try {

			/* if load_image failure */
            if(!m_central_widget->load_image(file_name)) {
                QString err_info = QString::fromLocal8Bit("读取图像文件 ") + file_name + QString::fromLocal8Bit(" 失败\n")
                    + QString::fromLocal8Bit("可能读入了错误的图像文件，或者图像数据丢失有误，请您从新指定");
                QMessageBox::warning(this, "ReadingBigImage", err_info);
                return;
            }

			/* exists the bad_alloc exception */
		} catch (std::bad_alloc &err) {
			QString err_info = QString::fromLocal8Bit("内存分配失败\n可能内存不足导致"); 
			if(QMessageBox::Abort == QMessageBox::critical(this, "ReadingBigImage", err_info, 
				QMessageBox::Ok | QMessageBox::Abort, QMessageBox::Ok)) {
				this->close();
			}
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
