#ifndef IMAGEBRUSHGUIDEWIDGET_H
#define IMAGEBRUSHGUIDEWIDGET_H

#include "imageguidewidget.h"

namespace Digikam
{

class ImageBrushGuideWidget : public ImageGuideWidget
{
    Q_OBJECT
public:
    using ImageGuideWidget::ImageGuideWidget;

    void setSrcSet(bool s);
    bool isSrcSet();
Q_SIGNALS:

    void signalClone(QPoint&,QPoint&);

public Q_SLOTS:
    void slotSrcSet();

private:
    void mouseMoveEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);
    void mousePressEvent(QMouseEvent*);
    bool srcSet=false; //toggle in srcSet slot
    bool released=true; //set to true in srcSet slot
    QPoint src;
    QPoint dst;
};

}
#endif // IMAGEBRUSHGUIDEWIDGET_H
