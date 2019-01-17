/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-09-03
 * Description : Integrated, multithread face detection / recognition
 *
 * Copyright (C) 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef DIGIKAM_FACE_ITEM_RETRIEVER_H
#define DIGIKAM_FACE_ITEM_RETRIEVER_H

// Local includes

#include "facepipeline_p.h"

namespace Digikam
{

class Q_DECL_HIDDEN FaceItemRetriever
{
public:

    explicit FaceItemRetriever(FacePipeline::Private* const d);
    void cancel();

    ThumbnailImageCatcher* thumbnailCatcher()                                               const;
    QList<QImage> getDetails(const DImg& src, const QList<QRectF>& rects)                   const;
    QList<QImage> getDetails(const DImg& src, const QList<FaceTagsIface>& faces)            const;
    QList<QImage> getThumbnails(const QString& filePath, const QList<FaceTagsIface>& faces) const;

protected:

    ThumbnailImageCatcher* catcher;

private:

    FaceItemRetriever(const FaceItemRetriever&); // Disable
};

} // namespace Digikam

#endif // DIGIKAM_FACE_ITEM_RETRIEVER_H
