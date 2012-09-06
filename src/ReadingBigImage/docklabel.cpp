#include "docklabel.h"
#include <QtGui/QPainter>
#include <QtCore/QRect>

#include <iostream>
#define PRINT(x) std::cout << #x << " : " << x << std::endl;
DockLabel::DockLabel(QWidget *parent)
    : QLabel(parent) 
{
    m_cols_ratio= m_rows_ratio = m_start_col_ratio= m_start_row_ratio = 0;
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
}

void DockLabel::paintEvent(QPaintEvent *event)
{
    if(pixmap() == NULL) return;

    int pix_width = pixmap()->width();
    int pix_height = pixmap()->height();
    int pix_margin = this->margin();

    QPainter painter(this);

    painter.drawPixmap(pix_margin, pix_margin, pix_width, pix_height, *pixmap());
    painter.setPen(QPen(Qt::yellow, 3, Qt::DashDotDotLine));
    painter.drawRect(pix_margin+m_start_col_ratio*pix_width, pix_margin+m_start_row_ratio*pix_height,
        m_cols_ratio*pix_width, m_rows_ratio*pix_height);
}

void DockLabel::set_draw_rect_ratio(double start_row_ratio, double start_col_ratio,
        double rows_ratio, double cols_ratio)
{
    m_start_row_ratio = start_row_ratio;
    m_start_col_ratio = start_col_ratio;
    m_rows_ratio = rows_ratio;
    m_cols_ratio = cols_ratio;

    this->repaint();
}

void DockLabel::mouseMoveEvent(QMouseEvent *event)
{
    if(pos_in_pixmap(event->pos()))
        setCursor(Qt::PointingHandCursor);
    else
        setCursor(Qt::ArrowCursor);

    return QLabel::mouseMoveEvent(event);
}

void DockLabel::mousePressEvent(QMouseEvent *event)
{
    if(!pos_in_pixmap(event->pos()))    return QLabel::mousePressEvent(event);

    int pix_width = pixmap()->width();
    int pix_height = pixmap()->height();
    int pix_margin = this->margin();

    double press_col_ratio = (double)(event->pos().x() - pix_margin) / pix_width;
    double press_row_ratio = (double)(event->pos().y() - pix_margin) / pix_height;
    double delta = 0.0001;

    m_start_col_ratio = press_col_ratio - (m_cols_ratio/2);
    if(m_start_col_ratio < delta)  
        m_start_col_ratio = 0;
    else if(m_start_col_ratio + m_cols_ratio > 1.0)
        m_start_col_ratio = 1.0 - m_cols_ratio;
    
    m_start_row_ratio = press_row_ratio - (m_rows_ratio/2);
    if(m_start_row_ratio < delta)  
        m_start_row_ratio = 0;
    else if(m_start_row_ratio + m_rows_ratio > 1.0)
        m_start_row_ratio = 1.0 - m_rows_ratio;
    
    emit signal_change_rect_ratio(m_start_row_ratio, m_start_col_ratio);

    this->repaint();
}
