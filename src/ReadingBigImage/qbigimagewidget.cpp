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

#include <boost/timer.hpp>
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

        get_show_image_size();

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
    if(img_data.size() <= 0) return;
    get_show_image_size();

    /* get the new image data according to the new imag size */
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
        int delta_rows = last_point.y() - event->pos().y();
        int delta_cols = last_point.x() - event->pos().x();

        /* if the movement is too small, do nothing */
        if(std::abs(delta_rows) < 10 && std::abs(delta_cols) < 10) return;

        boost::timer t;
        delta_copy_image_data(delta_rows, delta_cols);
        std::cout << "elapsed time is : " << t.elapsed() << "s" << std::endl;

        /* change the start position according to the mouse movement */
        //      start_row += delta_rows;
        //      start_col += delta_cols;

        //      if(!get_image_data()) {
        //          b_mouse_pressed = false;
        //          setCursor(QCursor(Qt::ArrowCursor));
        //          return;
        //      }

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
    get_show_image_size();

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

void QBigImageWidget::delta_copy_image_data(int delta_rows, int delta_cols) 
{
    last_start_row = start_row;
    last_start_col = start_col;

    start_row += delta_rows;
    start_col += delta_cols;

    normalize_para();

    delta_rows = start_row - last_start_row;
    delta_cols = start_col - last_start_col;

    if(delta_rows == 0 && delta_cols == 0)   return;

    /* first copy the image data from the original image data area into the new accordingly image data area */
    int ori_row, ori_col;
    int dst_row, dst_col;

    int distance_rows = std::abs(delta_rows);
    int distance_cols = std::abs(delta_cols);

    if(delta_cols < 0) {
        dst_col = distance_cols;
        ori_col = 0;
    } else {
        dst_col = 0;
        ori_col = distance_cols;
    }

    if(delta_rows < 0) {
        dst_row = distance_rows;
        ori_row = 0;
    } else {
        dst_row = 0;
        ori_row = distance_rows;
    }

    /* now copy the ori area data into dst area data */
    std::vector<Vec3b> ori_data = img_data;

    Vec3b *ori_ptr = &ori_data[ori_row*img_cols + ori_col];
    Vec3b *dst_ptr = &img_data[dst_row*img_cols + dst_col];

    for(unsigned row = 0; row < img_rows - distance_rows; ++row) {
        for(unsigned col = 0; col < img_cols - distance_cols; ++col) {
            *dst_ptr = *ori_ptr;
            ++dst_ptr;
            ++ori_ptr;
        }

        dst_ptr += distance_cols;
        ori_ptr += distance_cols;
    }

    /* now get the two rectangle image area into dst image */
    {
        int area_start_row, area_start_col;
        int area_rows, area_cols;

        area_rows = img_rows;
        area_cols = distance_cols;

        /* now get the delta image data */
        if(delta_cols < 0) {
            area_start_row = 0;
            area_start_col = 0;
            copy_area_image_data(area_start_row, area_start_col, area_rows, area_cols);

            area_rows = distance_rows;
            area_cols = img_cols - distance_cols;
            if(delta_rows < 0) {
                area_start_row = 0;
                area_start_col = distance_cols;
                copy_area_image_data(area_start_row, area_start_col, area_rows, area_cols);
            } else {
                area_start_row = img_rows - distance_rows;
                area_start_col = distance_cols;
                copy_area_image_data(area_start_row, area_start_col, area_rows, area_cols);
            }

        } else {
            area_start_row = 0;
            area_start_col = img_cols - distance_cols;
            copy_area_image_data(area_start_row, area_start_col, area_rows, area_cols);

            area_rows = distance_rows;
            area_cols = img_cols - distance_cols;
            if(delta_rows < 0) {
                area_start_row = 0;
                area_start_col = 0;
                copy_area_image_data(area_start_row, area_start_col, area_rows, area_cols);
            } else {
                area_start_row = img_rows - distance_rows;
                area_start_col = 0;
                copy_area_image_data(area_start_row, area_start_col, area_rows, area_cols);
            }
        }
    }
}

bool QBigImageWidget::copy_area_image_data(int area_start_row, int area_start_col, 
                                           int area_rows, int area_cols) 

{
    if(area_rows == 0 || area_cols == 0) return true;

    std::vector<Vec3b> area_data;

    if(!big_image->get_pixels_by_level(img_current_level, start_row+area_start_row, 
        start_col+area_start_col, area_rows, area_cols, area_data)) {
            init_para();
            if(QMessageBox::Abort == QMessageBox::critical(this, 
                "ReadingBigImage", 
                QString::fromLocal8Bit("¶ÁÈ¡Í¼ÏñÊý¾ÝÊ§°Ü"), 
                QMessageBox::Ok | QMessageBox::Abort, QMessageBox::Ok)) {
                    m_parent->close();
            }
            return false;
    }

    Vec3b *ori_ptr = &area_data[0];
    Vec3b *dst_ptr = &img_data[area_start_row*img_cols + area_start_col];

    for(unsigned row = 0; row < area_rows; ++row) {
        for(unsigned col = 0; col < area_cols; ++col) {
            *dst_ptr = *ori_ptr;
            ++dst_ptr;
            ++ori_ptr;
        }

        dst_ptr += img_cols - area_cols;
    }

    return true;
}
