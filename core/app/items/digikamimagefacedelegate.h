/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-04-19
 * Description : Qt item view for images - the delegate
 *
 * Copyright (C) 2009-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef DIGIKAMIMAGEFACEDELEGATE_H
#define DIGIKAMIMAGEFACEDELEGATE_H

// Local includes

#include "digikamimagedelegate.h"

namespace Digikam
{

class ImageCategoryDrawer;
class FaceTagsIface;
class DigikamImageFaceDelegatePrivate;

class DigikamImageFaceDelegate : public DigikamImageDelegate
{
    Q_OBJECT

public:

    explicit DigikamImageFaceDelegate(ImageCategorizedView* const parent);
    ~DigikamImageFaceDelegate();

    virtual void prepareThumbnails(ImageThumbnailModel* thumbModel, const QList<QModelIndex>& indexes);
    QRect faceRect(const QModelIndex& index) const;
    QRect largerFaceRect(const QModelIndex& index) const;

    FaceTagsIface face(const QModelIndex& index) const;

protected:

    virtual QPixmap thumbnailPixmap(const QModelIndex& index) const;
    virtual void updateRects();
    virtual void clearModelDataCaches();

private:

    Q_DECLARE_PRIVATE(DigikamImageFaceDelegate)
};

} // namespace Digikam

#endif /* DIGIKAMIMAGEFACEDELEGATE_H */
