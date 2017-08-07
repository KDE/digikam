// Local includes

#include "imagebrushguidewidget.h"


namespace Digikam
{

void ImageBrushGuideWidget::mouseMoveEvent(QMouseEvent* e)
{
    if ((e->buttons() & Qt::LeftButton & srcSet) && (!released))
    {
        released = false;
        qDebug() << "MOOOOOVE The location is: " << e->x() << ", "<< e->y();
        setSpotPosition(src.x() + e->x() - dst.x(), src.y() + e->y() - dst.y());
    }
}

void ImageBrushGuideWidget::mouseReleaseEvent(QMouseEvent* e)
{
    if(srcSet)
    {
        setSpotPosition(src.x(),src.y());
    }else{
        ImageGuideWidget::mouseReleaseEvent(e);
    }
    released = true;
}

void ImageBrushGuideWidget::mousePressEvent(QMouseEvent* e)
{
    if(!srcSet)
    {
        ImageGuideWidget::mousePressEvent(e);
    }else{
        if (e->button() == Qt::LeftButton)
        {
            dst = QPoint(e->x(),e->y());
        // signal the clone/heal
            emit signalClone(src, dst);
            released = false;
        }
    }
}

void ImageBrushGuideWidget::setSrcSet(bool s)
{
    srcSet = s;
}

bool ImageBrushGuideWidget::isSrcSet()
{
    return srcSet;
}

void ImageBrushGuideWidget::slotSrcSet()
{
    srcSet = !srcSet;
    if(srcSet)
    {
        src = getSpotPosition();
    }
    released = true;
}
}
