#include "qbigimagewidget.h"

/* qt module */
#include <Qtgui/QPaintEvent>
#include <Qtgui/QPixmap>
#include <QtGui/QImage>
#include <Qtgui/QPainter>
#include <Qtgui/QToolTip>
#include <Qtgui/QMouseEvent>
#include <Qtgui/QWheelEvent>

/* out of core module */
#include "../src/HierarchicalImage.hpp"

QBigImageWidget::QBigImageWidget(QWidget *parent)
	: m_parent(parent), QWidget(parent)
{
	margin = 10;
	img_rows = img_cols = start_row = start_col = 0;
	img_current_level = 0;
	img_current_rows = img_current_cols = 1;
	get_show_size();
}

bool QBigImageWidget::load_big_image(QString file_name)
{
	try {
		big_image = HierarchicalImage<Vec3b, 32>::load_image(file_name.toStdString().c_str());
	} catch (const std::bad_alloc &err){
		cerr << err.what() << endl;
		return false;
	}

	img_rows = show_rows; 
	img_cols = show_cols;

	/* default is the maximum level (thus the smallest size) */
	set_current_image_level(big_image->get_max_image_level());

	if(!get_image_data()) return false;

	this->repaint();

	setCursor(QCursor(Qt::OpenHandCursor));
	return true;
}

void QBigImageWidget::paintEvent(QPaintEvent * event)
{
	if(img_data.size() <= 0)	return;

	QPainter painter(this);
	painter.drawImage(margin, margin, QImage((uchar*)(&img_data[0]), img_cols, img_rows, QImage::Format_RGB888));
}

void QBigImageWidget::resizeEvent(QResizeEvent *event)
{
	get_show_size();

	static int last_show_rows = show_rows;
	static int last_show_cols = show_cols;

	if(last_show_rows != show_rows || last_show_cols != show_cols) {
		last_show_rows = show_rows;
		last_show_cols = show_cols;

		img_rows = show_rows;
		img_cols = show_cols;

		if(!get_image_data()) return;
	}
}

void QBigImageWidget::mousePressEvent(QMouseEvent *event)
{
	if(img_data.size() <= 0) return;

	setCursor(QCursor(Qt::ClosedHandCursor));
	b_mouse_pressed = true;
	last_point = event->pos();
}

void QBigImageWidget::mouseReleaseEvent(QMouseEvent *event)
{
	b_mouse_pressed = false;
	setCursor(QCursor(Qt::ArrowCursor));
}

void QBigImageWidget::mouseMoveEvent(QMouseEvent *event)
{
	/* for debugging the mouse position */
	/*
	setMouseTracking(true);
	QToolTip::showText(event->globalPos(), QString("(%1,%2)").arg(event->pos().y()).arg(event->pos().x()), this);
	*/

	if(b_mouse_pressed) {
		int deltaRows = last_point.y() - event->pos().y();
		int deltaCols = last_point.x() - event->pos().x();

		start_row += deltaRows;
		start_col += deltaCols;

		normalize_start_pos();

		/* save current position */
		last_point = event->pos();

		if(!get_image_data()) return;
		this->repaint();
	}
}

void QBigImageWidget::wheelEvent(QWheelEvent *event)
{
	/* if no image data, just ignore the event for the parent to handle this */
	if(img_data.size() <= 0) {
		event->ignore();
		return;
	}

	int num_step = event->delta() / 120;
	cout << num_step << endl;
	cout << "current level " << img_current_level << endl;
	set_current_image_level(img_current_level - num_step);
 	cout << "current level " << img_current_level << endl;
	if(!get_image_data())	return; 
	this->repaint();

	event->accept();
}


