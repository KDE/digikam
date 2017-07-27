// Qt includes
/*
#include <QRegion>
#include <QPainter>
#include <QPen>
#include <QTimer>
#include <QRect>
#include <QBrush>
#include <QFont>
#include <QFontMetrics>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dimg.h"
#include "digikam_debug.h"
#include "previewtoolbar.h"
#include "exposurecontainer.h"
#include "iccsettingscontainer.h"*/
#include "imagebrushguidewidget.h"
//#include <QDebug>

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
    }
    if(!srcSet)
    {
        ImageGuideWidget::mouseReleaseEvent(e);
    }
    released = true;
}

void ImageBrushGuideWidget::mousePressEvent(QMouseEvent* e)
{
    if(released && srcSet)
    {
        dst = QPoint(e->x(),e->y());
    }
    if(!srcSet)
    {
        ImageGuideWidget::mousePressEvent(e);
    }
    if (e->button() == Qt::LeftButton)
    {
        // call the clone/heal
        released = false;
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
