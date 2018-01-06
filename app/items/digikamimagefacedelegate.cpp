/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-04-19
 * Description : Qt item view for images - the delegate
 *
 * Copyright (C) 2002-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2002-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2010 by Andi Clemens <andi dot clemens at gmail dot com>
 * Copyright (C) 2006-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "digikamimagefacedelegate.h"
#include "imagedelegatepriv.h"

// Local includes

#include "digikam_debug.h"
#include "applicationsettings.h"
#include "facetagsiface.h"
#include "imagemodel.h"
#include "tagregion.h"
#include "digikamimagedelegatepriv.h"
#include "faceutils.h"

namespace Digikam
{

DigikamImageFaceDelegate::DigikamImageFaceDelegate(ImageCategorizedView* const parent)
    : DigikamImageDelegate(*new DigikamImageFaceDelegatePrivate, parent)
{
}

DigikamImageFaceDelegate::~DigikamImageFaceDelegate()
{
}

void DigikamImageFaceDelegate::prepareThumbnails(ImageThumbnailModel* thumbModel, const QList<QModelIndex>& indexes)
{
    //TODO
    DigikamImageDelegate::prepareThumbnails(thumbModel, indexes);
}

QPixmap DigikamImageFaceDelegate::thumbnailPixmap(const QModelIndex& index) const
{
    QRect rect = largerFaceRect(index);

    if (rect.isNull())
    {
        return DigikamImageDelegate::thumbnailPixmap(index);
    }

    // set requested thumbnail detail
    if (rect.isValid())
    {
        const_cast<QAbstractItemModel*>(index.model())->setData(index, rect, ImageModel::ThumbnailRole);
    }

    // parent implementation already resets the thumb size and rect set on model
    return DigikamImageDelegate::thumbnailPixmap(index);
}

QRect DigikamImageFaceDelegate::faceRect(const QModelIndex& index) const
{
    return face(index).region().toRect();
}

QRect DigikamImageFaceDelegate::largerFaceRect(const QModelIndex& index) const
{
    QRect rect = faceRect(index);

    if (rect.isNull())
    {
        return rect;
    }

    const int margin = FaceUtils::faceRectDisplayMargin();

    return rect.adjusted(-margin, -margin, margin, margin);
}

FaceTagsIface DigikamImageFaceDelegate::face(const QModelIndex& index) const
{
    QVariant extraData = index.data(ImageModel::ExtraDataRole);

    if (extraData.isNull())
    {
        return FaceTagsIface();
    }

    FaceTagsIface face = FaceTagsIface::fromVariant(extraData);
    return face;
}

void DigikamImageFaceDelegate::updateRects()
{
    Q_D(DigikamImageFaceDelegate);
    DigikamImageDelegate::updateRects();
    d->groupRect = QRect();
}

void DigikamImageFaceDelegate::clearModelDataCaches()
{
    DigikamImageDelegate::clearModelDataCaches();
}

} // namespace Digikam
