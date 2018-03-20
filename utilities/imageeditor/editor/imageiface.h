/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-02-14
 * Description : image data interface for image tools
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2004-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef IMAGEIFACE_H
#define IMAGEIFACE_H

// Qt includes

#include <QString>
#include <QPixmap>

// Local includes

#include "dimg.h"
#include "dcolor.h"
#include "filteraction.h"
#include "infocontainer.h"
#include "digikam_export.h"
#include "digikam_globals.h"

class QPaintDevice;

namespace Digikam
{

class DIGIKAM_EXPORT ImageIface
{

public:

    enum PreviewType
    {
        FullImage,      /// Preview will be rendered using full image.
        ImageSelection  /// Preview will be rendered using current selection from editor canvas.
    };

public:

    /** Standard constructor. Size is the constrain dimension of preview. This can be null size.
     */
    explicit ImageIface(const QSize& size = QSize(0, 0));
    ~ImageIface();

    /** If useSelection is true, preview will be rendered using current selection in editor instead the full
     *  image. Default preview is FullImage.
     */
    void setPreviewType(PreviewType type = FullImage);

    /** Sets preview size and returns new preview as with getPreview.
     *  The parameters are only hints, previewSize() may differ from size.
     */
    DImg  setPreviewSize(const QSize& size) const;

    /** Methods to get/set preview image information.
     */
    QSize       previewSize()               const;
    bool        previewHasAlpha()           const;
    bool        previewSixteenBit()         const;
    PreviewType previewType()               const;

    /** Return a pointer to the DImg object representing the preview image.
     *  This function is a backdoor for preview editing.
     */
    DImg * previewReference();

    /** Return a DImg object representing the preview image.
     */
    DImg  preview()                         const;

    /** Return current image selection position and size into original image coordinates.
     */
    QRect selectionRect()                   const;

    /** Return a DImg object representing the current original image selection.
     */
    DImg  selection()                       const;

    /** Get colors from original, (unchanged) preview
     *  or target preview (set by setPreviewImage) image.
     */
    DColor colorInfoFromOriginal(const QPoint& point)      const;
    DColor colorInfoFromPreview(const QPoint& point)       const;
    DColor colorInfoFromTargetPreview(const QPoint& point) const;

    /** Methods to get/set original image information.
     */
    QSize originalSize()       const;
    bool  originalHasAlpha()   const;
    bool  originalSixteenBit() const;

    /** Original image meta-data management methods.
     */
    IccProfile         originalIccProfile() const;
    PhotoInfoContainer originalPhotoInfo()  const;
    MetaEngineData     originalMetadata()   const;
    void               setOriginalMetadata(const MetaEngineData& meta);

    /** Return a pointer to the DImg object representing the original image.
     *  This object may not be modified or stored. Make copies if you need.
     */
    DImg* original() const;

    /** Convert depth of original image.
     */
    void convertOriginalColorDepth(int depth);

    /** Convert a DImg image to a pixmap for screen using color
     *  managed view if necessary.
     */
    QPixmap convertToPixmap(DImg& img) const;

    /** Paint the current target preview image (or the preview image,
     *  if setPreview has not been called) on the given paint device.
     *  at x|y, with given maximum width and height taken from rectangle rect.
     */
    void paint(QPaintDevice* const device, const QRect& rect, QPainter* const painter = 0);

    /** Set the color profile of the original image.
     */
    void setOriginalIccProfile(const IccProfile& profile);

    /** Set the color profile of the preview image.
     */
    void setPreviewIccProfile(const IccProfile& profile);

    /** Replace the data of the current original image selection with the given data.
     *  The characteristics of the data must match the characteristics of the current
     *  selection as returned by the selectionWidth(), selectionHeight(),
     *  originalSixteenBit() and originalHasAlpha() methods.
     *  Caller is an i18n'ed string that will be shown as the undo/redo action name.
     */
    void setSelection(const QString& caller, const FilterAction& action, const DImg& img);

    /** Replace the stored target preview with the given image.
     *  The characteristics of the data must match the characteristics of the current
     *  as returned by the preview...() methods.
     *  The target preview image is used by the paint() and
     *  colorInfoFromTargetPreview() methods.
     *  The image returned by getPreview() is unaffected.
     */
    void setPreview(const DImg& img);

    /** Replace the data of the original with the given image.
     *  The characteristics of the data must match the characteristics of
     *  the original image as returned by the original...() methods,
     *  The size of image can be changed.
     *  Caller is an i18n'ed string that will be shown as the undo/redo action name.
     */
    void setOriginal(const QString& caller, const FilterAction& action, const DImg& img);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif /* IMAGEIFACE_H */
