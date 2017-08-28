#ifndef IMAGEBRUSHGUIDEWIDGET_H
#define IMAGEBRUSHGUIDEWIDGET_H

#include "imageguidewidget.h"

namespace Digikam
{

class ImageBrushGuideWidget : public ImageGuideWidget
{
    Q_OBJECT
public:
    /**
     * Using the parent's constructor
     * Should be changed to get rid of the inheritance
     */
    using ImageGuideWidget::ImageGuideWidget;

    /**
     * @brief setSrcSet fixes/unfixes the source spot
     * @param s
     */
    void setSrcSet(bool s);

    /**
     * @brief isSrcSet
     * @return if the source spot is set
     */
    bool isSrcSet();
Q_SIGNALS:
    /**
     * @brief signalClone emitted when the src is set and the user initiated a brush click
     * and keeps emmitting with motion
     */
    void signalClone(QPoint&,QPoint&);

public Q_SLOTS:
    /**
     * @brief slotSrcSet toggles the fixing of the brush source center
     */
    void slotSrcSet();

private:
    void mouseMoveEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);
    void mousePressEvent(QMouseEvent*);
    bool srcSet=false; //toggle in srcSet slot
    bool released=true;
    QPoint src;
    QPoint dst;
};

}
#endif // IMAGEBRUSHGUIDEWIDGET_H
