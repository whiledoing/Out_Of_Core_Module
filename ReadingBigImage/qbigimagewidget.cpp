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
	init_para();
}

bool QBigImageWidget::load_big_image(QString file_name)
{
	try {
		big_image = HierarchicalImage<Vec3b, 256>::load_image(file_name.toStdString().c_str());

		/* if big_image is null, so just load_image failure */
		if(!big_image) return false;

		/* set the cache size is 32 */
		reinterpret_cast<HierarchicalImage<Vec3b, 256>*>(big_image.get())->set_file_cache_number(64);
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

	if(b_mouse_pressed) 
		painter.drawImage(margin, margin, QImage((uchar*)(&img_mousemove_data[0]), img_cols, img_rows, QImage::Format_RGB888));
	else
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

	if(b_mouse_pressed) {
		start_row += last_point.y() - event->pos().y();
		start_col += last_point.x() - event->pos().x();

		if(!get_image_data()) {
			b_mouse_pressed = false;
			setCursor(QCursor(Qt::ArrowCursor));
			return;
		}
		b_mouse_pressed = false;
		setCursor(QCursor(Qt::OpenHandCursor));
		this->repaint();
	}
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
		
		if(std::abs(deltaRows) < 10 && std::abs(deltaCols) < 10)	return;
		
		/* clear the data */
		for(size_t i = 0; i < img_mousemove_data.size(); ++i) {
			img_mousemove_data[i] = Vec3b();
		}

		int s_row, d_row, s_col, d_col;
		if(deltaRows < 0) {
			d_row = -deltaRows;
			s_row = 0;
		} else {
			d_row = 0;
			s_row = deltaRows;
		}
		
		if(deltaCols < 0) {
			d_col = -deltaCols;
			s_col = 0;
		} else {
			d_col = 0;
			s_col = deltaCols;
		}

		int t_rows = img_rows - std::abs(deltaRows);
		int t_cols = img_cols - std::abs(deltaCols);

		/* copy the data that comes from img_data which has been read from the disk */
		for(int i = 0; i < t_rows; ++i) {
			for(int j = 0; j < t_cols; ++j) {
				img_mousemove_data[(d_row+i)*img_cols + (d_col+j)] = img_data[(s_row+i)*img_cols + (s_col+j)];
			}
		}

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


