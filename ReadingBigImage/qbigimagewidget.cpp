#include "qbigimagewidget.h"

#include <Qtgui/QPaintEvent>
#include <Qtgui/QPixmap>
#include <QtGui/QImage>
#include <Qtgui/QPainter>

/* out of core module */
#include "../src/HierarchicalImage.hpp"

QBigImageWidget::QBigImageWidget(QWidget *parent)
	: m_parent(parent), QWidget(parent)
{
	img_rows = img_cols = 0;
	margin = 10;
}

QBigImageWidget::~QBigImageWidget()
{

}

bool QBigImageWidget::load_big_image(QString file_name)
{
	try {
		big_image = HierarchicalImage<Vec3b, 32>::load_image(file_name.toStdString().c_str());
	} catch (const std::bad_alloc &err){
		cerr << err.what() << endl;
		return false;
	}

	HierarchicalImage<Vec3b, 32> *p_big_image = reinterpret_cast<HierarchicalImage<Vec3b, 32>* >(big_image.get());

	/* get the highest level image data */
	p_big_image->set_current_level(p_big_image->get_max_image_level());

	int start_row = 0, start_col = 0; 
	img_rows = p_big_image->get_current_level_image_rows(); 
	img_cols = p_big_image->get_current_level_image_cols();

	if(!p_big_image->get_pixels_by_level(p_big_image->get_max_image_level(), start_row, start_col, img_rows, img_cols, img_data))
		return false;

	repaint();

	return true;
}

void QBigImageWidget::paintEvent(QPaintEvent * event)
{
	if(img_data.size() <= 0)	return;
	QImage img((uchar*)(&img_data[0]), img_cols, img_rows, QImage::Format_RGB888);

	m_parent->resize(img_rows + 2*margin, img_cols + 2*margin);

	QPainter painter(this);
	painter.drawImage(margin, margin, img);
}

