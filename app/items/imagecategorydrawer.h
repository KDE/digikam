/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-04-20
 * Description : Qt item view for images - category drawer
 *
 * Copyright (C) 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef IMAGECATEGORYDRAWER_H
#define IMAGECATEGORYDRAWER_H

// Local includes

#include "dcategorydrawer.h"

class QStyleOptionViewItem;

namespace Digikam
{

class ImageCategorizedView;
class Album;
class PAlbum;
class TAlbum;
class SAlbum;
class DAlbum;

class ImageCategoryDrawer : public DCategoryDrawer
{
public:

    explicit ImageCategoryDrawer(ImageCategorizedView* const parent);
    ~ImageCategoryDrawer();

    virtual int categoryHeight(const QModelIndex& index, const QStyleOption& option) const;
    virtual void drawCategory(const QModelIndex& index, int sortRole, const QStyleOption& option, QPainter* painter) const;
    virtual int maximumHeight() const;

    void setLowerSpacing(int spacing);
    void setDefaultViewOptions(const QStyleOptionViewItem& option);
    void invalidatePaintingCache();

private:

    void updateRectsAndPixmaps(int width);
    void viewHeaderText(const QModelIndex& index, QString* header, QString* subLine) const;
    void textForAlbum(const QModelIndex& index, QString* header, QString* subLine) const;
    void textForPAlbum(PAlbum* a, bool recursive, int count, QString* header, QString* subLine) const;
    void textForTAlbum(TAlbum* a, bool recursive, int count, QString* header, QString* subLine) const;
    void textForSAlbum(SAlbum* a, int count, QString* header, QString* subLine) const;
    void textForDAlbum(DAlbum* a, int count, QString* header, QString* subLine) const;
    void textForFormat(const QModelIndex& index, QString* header, QString* subLine) const;

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif /* IMAGECATEGORYDRAWER_H */
