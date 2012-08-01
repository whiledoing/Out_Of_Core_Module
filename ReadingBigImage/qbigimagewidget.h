#ifndef QBIGIMAGEWIDGET_H
#define QBIGIMAGEWIDGET_H

#include "../src/BasicType.h"
#include "../src/HierarchicalInterface.h"

/* qt module */
#include <QWidget>
#include <QMessageBox>
#include <QString>
class QPaintEvent;
class QResizeEvent;
class QMouseEvent;
class QWheelEvent;
class QKeyEvent;

/* library module */
#include <boost/timer.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>
#include <iostream>

class QBigImageWidget : public QWidget
{
	Q_OBJECT

public:
	QBigImageWidget(QWidget *parent);
	bool load_big_image(QString file_name) throw (const std::bad_alloc&);

protected:
	virtual void	paintEvent(QPaintEvent *event);
	virtual void resizeEvent(QResizeEvent *event);
	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseReleaseEvent(QMouseEvent *event);
	virtual void mouseMoveEvent(QMouseEvent *event);
	virtual void wheelEvent(QWheelEvent *event);
	virtual void keyPressEvent(QKeyEvent * event);

private:

	/*
	 *	@para : get the new show area size according to the widget size
	 */
	void get_show_size() 
	{
		show_rows = height() - 2*margin;
		show_cols = width() - 2*margin;
	}

	/*
	 *	@para : set the current image level (when using mouse wheel to change the level)
	 */
	void set_current_image_level(int level) 
	{
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

			/* keep the last level value */
			last_level = level;
		}
	}

	/*
	 *	@para : get the actual image data, save the data into img_data member
	 */
	bool get_image_data() 
	{
		/* if any one of the data is not equal to the last data, then get the actual image data */
		if(last_start_row != start_row || last_start_col != start_col 
			|| last_img_rows != img_rows || last_img_cols != img_cols || last_current_level != img_current_level) {

			/* first get the new validating para */
			normalize_para();

			try {

				/* get the new image data */
				if(!big_image->get_pixels_by_level(img_current_level, start_row, start_col, img_rows, img_cols, img_data)) {
					init_para();
					if(QMessageBox::Abort == QMessageBox::critical(this, "ReadingBigImage", 
						QString::fromLocal8Bit("¶ÁÈ¡Í¼ÏñÊý¾ÝÊ§°Ü"), QMessageBox::Ok | QMessageBox::Abort, QMessageBox::Ok)) {
						m_parent->close();
					}

					return false;
				}

			} catch (std::exception  &err) {
                 QMessageBox::critical(this, "ReadingBigImage", QString::fromLocal8Bit("ÄÚ´æ·ÖÅäÊ§°Ü"), QMessageBox::Ok);
				 m_parent->close();
			} catch (...) {
				std::cerr << "get pixels error" << std::endl;
				m_parent->close();
			}

			/* keep the para info */
			last_start_row = start_row; last_start_col = start_col;
			last_img_rows = img_rows; last_img_cols = img_cols;
			last_current_level = img_current_level;
		}

		/* here, everything is equal to the formal one, so the last image data is just the exact one */
		return true;
	}

	/*
	 *	@para : normalize the parameter before calling get_pixels_by_level function
	 */
	void normalize_para() 
	{
		/* first get the image rows and cols as much as the show area size */
		img_rows = show_rows; 
		img_cols = show_cols;

		/* check the start position validation */
		start_row = (start_row < 0) ? 0 : start_row;
		start_col = (start_col < 0) ? 0 : start_col;

		/* make the start_row as less as possible to get the more area to be shown */
		if(start_row + img_rows > img_current_rows) start_row = img_current_rows - img_rows;
		if(start_col + img_cols > img_current_cols) start_col = img_current_cols - img_cols;

		/* if still start_ros is less than zero, that means the img_current_rows is less than img_rows
		 * so just let the img_rows to be the maximum value, thus current img size */
		if(start_row < 0) {
			start_row = 0;
			img_rows = img_current_rows;
		}
		if(start_col < 0) {
			start_col = 0;
			img_cols = img_current_cols;
		}
	}

	/*
	 *	@para : initialize the parameter
	 */
	void init_para() 
	{
		img_rows = img_cols = start_row = start_col = 0;
		img_current_level = 0;
		img_current_rows = img_current_cols = 1;
		img_data.clear();
		big_image.reset();

		last_start_row = -100;
		last_start_col = -100;
		last_img_rows = -100;
		last_img_cols = -100;
		last_current_level = -100;
		last_level = -100;

		b_mouse_pressed = false;
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

	/* save the last time value */
	int last_start_row;
	int last_start_col;
	int last_img_rows;
	int last_img_cols;
	int last_current_level;
    int last_level;

	/* the showing area margin, so the total size of the showing area is : (show_rows + 2*margin, show_cols + 2*margin)
	 * the image is showing in the central of the showing area
	 */
	int margin;

	/* index the current image level */
	int img_current_level;

	/* used to track the mouse event */
	bool b_mouse_pressed;
	QPoint last_point;

	/* save the main window widget */
	QWidget *m_parent;
};

#endif // QBIGIMAGEWIDGET_H
