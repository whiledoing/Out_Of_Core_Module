#ifndef DOCKWIDGET_h
#define DOCKWIDGET_h

#include <QtGui/QLabel>
#include <QtGui/QPaintEvent>

class DockLabel : public QLabel
{
    Q_OBJECT

public:
    DockLabel(QWidget *parent = NULL);

public slots:
    void set_draw_rect_ratio(double start_row_ratio, double start_col_ratio,
        double rows_ratio, double cols_ratio);

protected:
    void paintEvent(QPaintEvent *event);

private:
    double m_start_row_ratio;
    double m_start_col_ratio;
    double m_rows_ratio;
    double m_cols_ratio;
};

#endif // !DOCKWIDGET_h
