#ifndef SHOWFOTODELEGATEPRIV_H
#define SHOWFOTODELEGATEPRIV_H

// Qt includes

#include <QRect>
#include <QCache>

// Local includes

#include "showfotothumbnailbar.h"
#include "itemviewshowfotodelegatepriv.h"
#include "showfotodelegate.h"

namespace ShowFoto
{

class ShowfotoDelegate::ShowfotoDelegatePrivate : public ItemViewShowfotoDelegatePrivate
{
public:

    ShowfotoDelegatePrivate()
    {
      //categoryDrawer      = 0;
        contentWidth        = 0;
        drawImageFormat     = false;
        drawMouseOverFrame  = true;
        drawFocusFrame      = true;
        ratingOverThumbnail = false;
        currentModel        = 0;
        currentView         = 0;

        actualPixmapRectCache.setMaxCost(250);
    }

    int                    contentWidth;

    QRect                  dateRect;
    QRect                  pixmapRect;
    QRect                  nameRect;
//  QRect                  titleRect;
//  QRect                  commentsRect;
    QRect                  resolutionRect;
    QRect                  sizeRect;
//  QRect                  downloadRect;
//  QRect                  lockRect;
    QRect                  tagRect;
    QRect                  imageInformationRect;
    QRect                  pickLabelRect;
    QRect                  groupRect;

    bool                   drawImageFormat;
    bool                   drawFocusFrame;
    bool                   drawMouseOverFrame;
    bool                   ratingOverThumbnail;

    QCache<int, QRect>     actualPixmapRectCache;
//  ShowfotoCategoryDrawer*  categoryDrawer;

    ShowfotoThumbnailBar* currentView;
    QAbstractItemModel*    currentModel;

public:

    virtual void clearRects();
};

// --- ShowfotoThumbnailDelegate ----------------------------------------------------

class ShowfotoThumbnailDelegatePrivate : public ShowfotoDelegate::ShowfotoDelegatePrivate
{
public:

    ShowfotoThumbnailDelegatePrivate()
    {
        flow                = QListView::LeftToRight;

        // switch off drawing of frames
        drawMouseOverFrame  = false;
        drawFocusFrame      = false;

        // switch off composing rating over background
        ratingOverThumbnail = true;
    }

    void init(ShowfotoThumbnailDelegate* const q);

public:

    QListView::Flow flow;
    QRect           viewSize;
};

// --- ShowfotoNormalDelegate ----------------------------------------------------

class ShowfotoNormalDelegatePrivate : public ShowfotoDelegate::ShowfotoDelegatePrivate
{
public:

    ShowfotoNormalDelegatePrivate()
    {
    }

    void init(ShowfotoNormalDelegate* const q/*, ShowfotoThumbnailBar* const parent*/);
};

} // namespace Digikam


#endif // SHOWFOTODELEGATEPRIV_H
