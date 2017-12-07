/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-07-08
 * Description : Qt item view for images - the delegate
 *
 * Copyright (C) 2012      by Islam Wazery <wazery at ubuntu dot com>
 * Copyright (C) 2012-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef IMPORTDELEGATEPRIV_H
#define IMPORTDELEGATEPRIV_H

// Qt includes

#include <QRect>
#include <QCache>

// Local includes

#include "importcategorizedview.h"
#include "itemviewimportdelegatepriv.h"

namespace Digikam
{

class ImportCategoryDrawer;

class ImportDelegate::ImportDelegatePrivate : public ItemViewImportDelegatePrivate
{
public:

    ImportDelegatePrivate()
    {
        categoryDrawer      = 0;
        contentWidth        = 0;
        drawImageFormat     = false;
        drawCoordinates     = false;
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
    QRect                  downloadRect;
    QRect                  lockRect;
    QRect                  coordinatesRect;
    QRect                  tagRect;
    QRect                  imageInformationRect;
    QRect                  pickLabelRect;
    QRect                  groupRect;

    bool                   drawImageFormat;
    bool                   drawCoordinates;
    bool                   drawFocusFrame;
    bool                   drawMouseOverFrame;
    bool                   ratingOverThumbnail;

    QCache<int, QRect>     actualPixmapRectCache;
    ImportCategoryDrawer*  categoryDrawer;

    ImportCategorizedView* currentView;
    QAbstractItemModel*    currentModel;

public:

    virtual void clearRects();
};

// --- ImportThumbnailDelegate ----------------------------------------------------

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
        ratingOverThumbnail = true;
    }

    void init(ImportThumbnailDelegate* const q);

public:

    QListView::Flow flow;
    QRect           viewSize;
};

// --- ImportNormalDelegate ----------------------------------------------------

class ImportNormalDelegatePrivate : public ImportDelegate::ImportDelegatePrivate
{
public:

    ImportNormalDelegatePrivate()
    {
    }
    virtual ~ImportNormalDelegatePrivate();

    void init(ImportNormalDelegate* const q, ImportCategorizedView* const parent);
};

} // namespace Digikam

#endif // IMPORTDELEGATEPRIV_H
