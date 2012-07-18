#ifndef IMPORTDELEGATEPRIV_H
#define IMPORTDELEGATEPRIV_H

// Qt includes

#include <QRect>
#include <QCache>

// Local includes

#include "importdelegate.h"
#include "itemviewimportdelegatepriv.h"

namespace Digikam
{

class ImportDelegate::ImportDelegatePrivate : public ItemViewImportDelegatePrivate
{
public:

    ImportDelegatePrivate()
    {
        categoryDrawer      = 0;
        contentWidth        = 0;
        drawImageFormat     = false;
        drawMouseOverFrame  = true;
        drawFocusFrame      = true;
        //ratingOverThumbnail = false;
        currentModel        = 0;
        currentView         = 0;

        actualPixmapRectCache.setMaxCost(250);
    }

    int                    contentWidth;

    QRect                  dateRect;
    QRect                  modDateRect;
    QRect                  pixmapRect;
    QRect                  nameRect;
    //QRect                  titleRect;
    //QRect                  commentsRect;
    QRect                  resolutionRect;
    QRect                  sizeRect;
    //QRect                  tagRect;
    QRect                  imageInformationRect;
    //QRect                  pickLabelRect;
    QRect                  groupRect;

    bool                   drawImageFormat;
    bool                   drawFocusFrame;
    bool                   drawMouseOverFrame;
    //bool                   ratingOverThumbnail;

    QCache<int, QRect>     actualPixmapRectCache;
    ImportCategoryDrawer*  categoryDrawer;

    ImportCategorizedView* currentView;
    QAbstractItemModel*    currentModel;

public:

    virtual void clearRects();
};

class ImportThumbnailDelegatePrivate : public ImportDelegate::ImportDelegatePrivate
{
public:

    ImportThumbnailDelegatePrivate()
    {
        flow                = QListView::LeftToRight;

        // switch off drawing of frames
        drawMouseOverFrame  = false;
        drawFocusFrame      = false;

        // switch off composing rating over background
        //TODO: ratingOverThumbnail = true;
    }

    QListView::Flow flow;
    QRect           viewSize;

public:

    void init(ImportThumbnailDelegate* q);
};

class ImportNormalDelegatePrivate : public ImportDelegate::ImportDelegatePrivate
{
public:

    ImportNormalDelegatePrivate()
    {
    }

    void init(ImportNormalDelegate* q, ImportCategorizedView* parent);
};

} // namespace Digikam

#endif // IMPORTDELEGATEPRIV_H
