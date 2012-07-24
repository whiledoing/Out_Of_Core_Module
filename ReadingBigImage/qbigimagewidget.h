#ifndef QBIGIMAGEWIDGET_H
#define QBIGIMAGEWIDGET_H

#include <QWidget>

#include <boost/shared_ptr.hpp>
#include "../src/DataType.h"
#include "../src/HierarchicalInterface.h"
#include <vector>

class QBigImageWidget : public QWidget
{
	Q_OBJECT

public:
	QBigImageWidget(QWidget *parent);
	bool load_big_image(QString file_name);

protected:
	virtual void	paintEvent(QPaintEvent *event);
	virtual void resizeEvent(QResizeEvent *event);
	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseReleaseEvent(QMouseEvent *event);
	virtual void mouseMoveEvent(QMouseEvent *event);

private:
	void get_show_size() {
		show_rows = height() - 2*margin;
		show_cols = width() - 2*margin;
	}

	void set_current_image_level(int level) {
		if(big_image) {
			img_current_level = level;
			big_image->set_current_level(img_current_level);
			img_current_rows = big_image->get_current_level_image_rows();
			img_current_cols = big_image->get_current_level_image_cols();
		}
	}

	bool get_image_data() {
		static int last_start_row = -1;
		static int last_start_col = -1;
		static int last_img_rows = -1;
		static int last_img_cols = -1;

		/* if any one of the data is not equal to the last data, then get the actual image data */
		if(last_start_row != start_row || last_start_col != start_col 
			|| last_img_rows != img_rows || last_img_cols != img_cols) {

			last_start_row = start_row; last_start_col = start_col;
			last_img_rows = img_rows; last_img_cols = img_cols;

			/* get the new image data */
			return big_image->get_pixels_by_level(img_current_level, start_row, start_col, img_rows, img_cols, img_data);
		}

		/* here, everything is equal to the formal one, so the last image data is just the exact one */
		return true;
	}

private:
	/* save the big image object read from the image file in disk */
	boost::shared_ptr<HierarchicalInterface<Vec3b> > big_image;

	/* saves the actual image data for painting */
	std::vector<Vec3b> img_data;

	/* the actual image size saving in the img_data */
	int img_rows;
	int img_cols;

	/* the image current level size */
	int img_current_rows;
	int img_current_cols;

	/* the start position of the total image when painting in the widget */
	int start_row;
	int start_col; 

	/* the showing area size in the widget */
	int show_rows;
	int show_cols;

	/* the showing area margin, so the total size of the showing area is : (show_rows + 2*margin, show_cols + 2*margin)
	 * the image is showing in the central of the showing area
	 */
	int margin;

	/* index the current image level */
	int img_current_level;

	bool b_mouse_pressed;
	QPoint last_point;

	QWidget *m_parent;
};

#endif // QBIGIMAGEWIDGET_H
