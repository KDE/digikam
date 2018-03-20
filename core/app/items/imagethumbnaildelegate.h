/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-06
 * Description : thumbnail bar for images - the delegate
 *
 * Copyright (C) 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef IMAGETHUMBNAILDELEGATE_H
#define IMAGETHUMBNAILDELEGATE_H

// Qt includes

#include <QListView>

// Local includes

#include "imagedelegate.h"

namespace Digikam
{

class ImageCategoryDrawer;
class ImageThumbnailDelegatePrivate;

class ImageThumbnailDelegate : public ImageDelegate
{
    Q_OBJECT

public:

    explicit ImageThumbnailDelegate(ImageCategorizedView* const parent);
    ~ImageThumbnailDelegate();

    void setFlow(QListView::Flow flow);

    /** Returns the minimum or maximum viewport size in the limiting dimension,
     *  width or height, depending on current flow. */
    int maximumSize() const;
    int minimumSize() const;

    virtual void setDefaultViewOptions(const QStyleOptionViewItem& option);
    virtual bool acceptsActivation(const QPoint& pos, const QRect& visualRect, const QModelIndex& index,
                                   QRect* activationRect) const;

protected:

    virtual void updateContentWidth();
    virtual void updateRects();

private:

    Q_DECLARE_PRIVATE(ImageThumbnailDelegate)
};

} // namespace Digikam

#endif /* IMAGETHUMBNAILDELEGATE_H */
