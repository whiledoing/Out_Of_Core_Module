#include "qbigimagewidget.h"

/* qt module */
#include <Qtgui/QPaintEvent>
#include <Qtgui/QPixmap>
#include <QtGui/QImage>
#include <Qtgui/QPainter>
#include <Qtgui/QToolTip>
#include <Qtgui/QMouseEvent>
#include <Qtgui/QWheelEvent>
#include <Qtgui/QKeyEvent>

/* out of core module */
#include <boost/format.hpp>
#include "OutOfCore/DiskBigImage.hpp"

QBigImageWidget::QBigImageWidget(QWidget *parent)
	: m_parent(parent), QWidget(parent)
{
	margin = 10;
	init_para();
	setFocusPolicy(Qt::StrongFocus);
}

bool QBigImageWidget::load_image(QString file_name)
{
	try {
		big_image = load_disk_image<Vec3b>(file_name.toStdString());

		/* if big_image is null, so just load_image failure */
		if(!big_image) return false;

		/* set the cache size is 32 */
		big_image->set_file_cache_number(32);
	} catch (const std::bad_alloc &err){
		init_para();
		throw err;
	}

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

	if(img_data.size() <= 0) return;

	/* tells the img_rows and img_cols has been changed */
	img_rows = show_rows;
	img_cols = show_cols;

    if(!get_image_data()) return;
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
	if(img_data.size() <= 0) return;

	b_mouse_pressed = false;
	setCursor(QCursor(Qt::OpenHandCursor));
}

void QBigImageWidget::mouseMoveEvent(QMouseEvent *event)
{
	/* for debugging the mouse position */
	/*
	setMouseTracking(true);
	QToolTip::showText(event->globalPos(), QString("(%1,%2)").arg(event->pos().y()).arg(event->pos().x()), this);
	*/

	if(img_data.size() <= 0) return;
	if(b_mouse_pressed) {
		int deltaRows = last_point.y() - event->pos().y();
		int deltaCols = last_point.x() - event->pos().x();

		if(std::abs(deltaRows) < 20 && std::abs(deltaCols) < 20) return;

		/* change the start position according to the mouse movement */
		start_row += deltaRows;
		start_col += deltaCols;

		if(!get_image_data()) {
			b_mouse_pressed = false;
            setCursor(QCursor(Qt::ArrowCursor));
			return;
		}

		this->repaint();

		/* save current position */
		last_point = event->pos();
	}
}

void QBigImageWidget::wheelEvent(QWheelEvent *event)
{
	/* if no image data, just ignore the event for the parent to handle this */
	if(img_data.size() <= 0) {
		event->ignore();
		return;
	}

	/* tells we take care of this event */
	event->accept();

	/* set the level with opposite direction of the wheel direction */
	/* if wheel down, thus scale small the image, then the level should increases */
	int num_step = event->delta() / 120;
	set_current_image_level(img_current_level - num_step);

	/* now everything is ready, just get the new data */
	if(!get_image_data())	return; 

	/* redraw the image*/
	this->repaint();
}

void QBigImageWidget::keyPressEvent(QKeyEvent *event)
{
	/* test code for the set_pixel_by_level function */
	/*
	if(img_data.size() <= 0) return;
	switch (event->key()) {
	case Qt::Key_W :
		if(event->modifiers() & Qt::ControlModifier) {
			int rows = 324;
			int cols = 213;
			int start_row = 101;
			int start_col = 145;
			std::vector<Vec3b> zero_data(rows*cols);
			big_image->set_pixel_by_level(img_current_level, start_row, start_col, rows, cols, zero_data);
			return;
		}
	default :
		QWidget::keyPressEvent(event);
	}
	*/

	return QWidget::keyPressEvent(event);
}