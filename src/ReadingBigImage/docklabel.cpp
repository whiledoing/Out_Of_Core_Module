#include "docklabel.h"
#include <QtGui/QPainter>

DockLabel::DockLabel(QWidget *parent)
    : QLabel(parent) 
{
    m_cols_ratio= m_rows_ratio = m_start_col_ratio= m_start_row_ratio = 0;
}

void DockLabel::paintEvent(QPaintEvent *event)
{
    if(pixmap() == NULL) return;

    int pix_width = pixmap()->width();
    int pix_height = pixmap()->height();
    int pix_margin = this->margin();

    QPainter painter(this);

    painter.drawPixmap(pix_margin, pix_margin, pix_width, pix_height, *pixmap());
    painter.setPen(QPen(Qt::magenta, 3, Qt::DashDotDotLine, Qt::RoundCap, Qt::RoundJoin));
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