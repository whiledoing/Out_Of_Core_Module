#ifndef DOCKWIDGET_h
#define DOCKWIDGET_h

#include <QtGui/QLabel>
#include <QtGui/QPaintEvent>
#include <QtGui/QMouseEvent>

class DockLabel : public QLabel
{
    Q_OBJECT

public:
    DockLabel(QWidget *parent = NULL);

public slots:
    void set_draw_rect_ratio(double start_row_ratio, double start_col_ratio,
        double rows_ratio, double cols_ratio);

signals:
    void signal_change_rect_ratio(double start_row_ratio, double start_col_ratio);

protected:
    void paintEvent(QPaintEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);

private:
    bool pos_in_pixmap(const QPoint &point)
    {
        return QRect(margin(), margin(), pixmap()->width(), 
            pixmap()->height()).contains(point);
    }

private:
    double m_start_row_ratio;
    double m_start_col_ratio;
    double m_rows_ratio;
    double m_cols_ratio;
};

#endif // !DOCKWIDGET_h
