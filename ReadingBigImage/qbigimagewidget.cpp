#include "qbigimagewidget.h"

#include <Qtgui/QPaintEvent>
#include <Qtgui/QPixmap>
#include <QtGui/QImage>
#include <Qtgui/QPainter>
#include <Qtgui/QToolTip>
#include <Qtgui/QMouseEvent>

/* out of core module */
#include "../src/HierarchicalImage.hpp"

QBigImageWidget::QBigImageWidget(QWidget *parent)
	: m_parent(parent), QWidget(parent)
{
	margin = 10;
	img_rows = img_cols = start_row = start_col = 0;
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

		start_row = (start_row < 0) ? 0 : start_row;
		start_col = (start_col < 0) ? 0 : start_col;

		if(start_row + img_rows > img_current_rows) start_row = img_current_rows - img_rows;
		if(start_col + img_cols > img_current_cols) start_col = img_current_cols - img_cols;

		/* save current position */
		last_point = event->pos();

		if(!get_image_data()) return;
		this->repaint();
	}
}

