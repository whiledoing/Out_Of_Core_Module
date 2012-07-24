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
	~QBigImageWidget();
	bool load_big_image(QString file_name);

protected:
	virtual void	paintEvent ( QPaintEvent * event );

private:
	boost::shared_ptr<HierarchicalInterface<Vec3b> > big_image;
	std::vector<Vec3b> img_data;
	int img_rows;
	int img_cols;
	int margin;
	QWidget *m_parent;
};

#endif // QBIGIMAGEWIDGET_H
