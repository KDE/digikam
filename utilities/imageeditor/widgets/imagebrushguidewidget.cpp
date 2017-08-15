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
        QPoint currentDst = QPoint(e->x(),e->y());
        //updateSpotPosition(src.x() + currentDst.x() - dst.x(), src.y() + currentDst.y() - dst.y());
        currentDst = translateImagePosition(currentDst, false);
        QPoint currentSrc = translateImagePosition(src, true);
        QPoint orgDst = translateImagePosition(dst, false);
        currentSrc = QPoint(currentSrc.x() + currentDst.x() - orgDst.x(), currentSrc.y() + currentDst.y() - orgDst.y());
        //QPoint spotSrc = translateImagePosition(currentSrc, true);
        setSpotPosition(currentSrc);
        //updateSpotPosition(src.x() + currentDst.x() - orgDst.x(), src.y() + currentDst.y() - orgDst.y());

        emit signalClone(currentSrc, currentDst);
    }
    if(!srcSet)
    {
        ImageGuideWidget::mouseMoveEvent(e);
    }
}

void ImageBrushGuideWidget::mouseReleaseEvent(QMouseEvent* e)
{
    if(srcSet)
    {
        QPoint p = translatePointPosition(src);
        setSpotPosition(p);
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
            //QPoint edit = translatePointPosition(dst);
        // signal the clone/heal
            QPoint currentSrc = translateImagePosition(src, true);
            QPoint currentDst = translateImagePosition(dst, false);
            emit signalClone(currentSrc, currentDst);
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
        src =  getSpotPosition();
    }
    released = true;
}
}
