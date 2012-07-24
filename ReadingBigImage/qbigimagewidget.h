#ifndef QBIGIMAGEWIDGET_H
#define QBIGIMAGEWIDGET_H

#include "../src/DataType.h"
#include "../src/HierarchicalInterface.h"

/* qt module */
#include <QWidget>
class QPaintEvent;
class QResizeEvent;
class QMouseEvent;
class QWheelEvent;

/* library module */
#include <boost/shared_ptr.hpp>
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
	virtual void wheelEvent(QWheelEvent *event);

private:
	void get_show_size() {
		show_rows = height() - 2*margin;
		show_cols = width() - 2*margin;
	}

	void set_current_image_level(int level) {
		static int last_level = -1;

		level = (level > (int)big_image->get_max_image_level()) ? big_image->get_max_image_level() : level;
		level = (level < 0) ? 0 : level;

		/* if exist the image and set different level */
		if(big_image && last_level != level) {
			double start_row_ratio = start_row / img_current_rows;
			double start_col_ration = start_col / img_current_cols;

			/* set current level */
			img_current_level = level;
			big_image->set_current_level(img_current_level);
			img_current_rows = big_image->get_current_level_image_rows();
			img_current_cols = big_image->get_current_level_image_cols();

			start_row = img_current_rows*start_row_ratio;
			start_col = img_current_cols*start_col_ration;
			normalize_start_pos();

			/* keep the last level value */
			last_level = level;
		}
	}

	bool get_image_data() {
		static int last_start_row = -1;
		static int last_start_col = -1;
		static int last_img_rows = -1;
		static int last_img_cols = -1;
		static int last_current_level = -1;

		/* if any one of the data is not equal to the last data, then get the actual image data */
		if(last_start_row != start_row || last_start_col != start_col 
			|| last_img_rows != img_rows || last_img_cols != img_cols || last_current_level != img_current_level) {

			/* get the new image data */
			if(!big_image->get_pixels_by_level(img_current_level, start_row, start_col, img_rows, img_cols, img_data)) 
				return false;

			last_start_row = start_row; last_start_col = start_col;
			last_img_rows = img_rows; last_img_cols = img_cols;
			last_current_level = img_current_level;
		}

		/* here, everything is equal to the formal one, so the last image data is just the exact one */
		return true;
	}

	void normalize_start_pos() {
		start_row = (start_row < 0) ? 0 : start_row;
		start_col = (start_col < 0) ? 0 : start_col;

		if(start_row + img_rows > img_current_rows) start_row = img_current_rows - img_rows;
		if(start_col + img_cols > img_current_cols) start_col = img_current_cols - img_cols;

		start_row = (start_row < 0) ? 0 : start_row;
		start_col = (start_col < 0) ? 0 : start_col;
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
