/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-02-14
 * Description : image data interface for image plugins
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2004-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <QtCore/QString>
#include <QtGui/QPixmap>

// KDE includes

#include <klocale.h>

// Local includes

#include "dimg.h"
#include "dcolor.h"
#include "filteraction.h"
#include "photoinfocontainer.h"
#include "digikam_export.h"
#include "globals.h"

class QPaintDevice;

namespace Digikam
{

class DIGIKAM_EXPORT ImageIface
{
public:

    explicit ImageIface(int w=0, int h=0);
    ~ImageIface();

    /** Use this method to use the current selection in editor instead the full
     *  image to render the preview.
     */
    void setPreviewType(bool useSelect=false);

    /** Return 'true' if the preview is rendered using the current selection in editor.
     *  Return 'false' if the preview is rendered using the full image in editor.
     */
    bool previewType();

    /** Return image data for the current, scaled preview image.
     *  The preview...() methods provide the characteristics of the data
     *  (width, height, sixteen bit, alpha).
     *  Ownership of the returned buffer is passed to the caller.
     */
    uchar* getPreviewImage() const;

    /** Return a DImg object representing the preview image.
     */
    DImg getPreviewImg();

    /** Return image data for the current original image selection.
     *  The selectionWidth(), selectionHeight(), originalSixteenBit()
     *  and originalHasAlpha() methods provide the characteristics of the data.
     *  Ownership of the returned buffer is passed to the caller.
     */
    uchar* getImageSelection() const;

    /** Return image data for the original image.
     *  The preview...() methods provide the characteristics of the data.
     *  Ownership of the returned buffer is passed to the caller.
     */
    uchar* getOriginalImage() const;

    /** Return a pointer to the DImg object representing the original image.
     *  This object may not be modified or stored. Make copies if you need.
     */
    DImg*  getOriginalImg() const;

    /** Replace the image data of the original image with the given data.
     *  The characteristics of the data must match the characteristics of
     *  the original image as returned by the original...() methods,
     *  respectively the given width and height parameters.
     *  No ownership of the data pointer is assumed.
     *  If w == -1 and h == -1, the size is unchanged.
     *  Caller is an i18n'ed string that will be shown as the undo/redo action name.
     */
    void   putOriginalImage(const QString& caller, const FilterAction& action, uchar* data, int w=-1, int h=-1);

    /** Set the color profile of the original image.
     */
    void   putOriginalIccProfile(const IccProfile& profile);

    /** Replace the data of the current original image selection with the given data.
     *  The characteristics of the data must match the characteristics of the current
     *  selection as returned by the selectionWidth(), selectionHeight(),
     *  originalSixteenBit() and originalHasAlpha() methods.
     *  No ownership of the data pointer is assumed.
     *  Caller is an i18n'ed string that will be shown as the undo/redo action name.
     */
    void   putImageSelection(const QString& caller, const FilterAction& action, uchar* data);

    /** Replace the stored target preview data with the given data.
     *  The characteristics of the data must match the characteristics of the current
     *  as returned by the preview...() methods.
     *  The target preview data is used by the paint() and
     *  getColorInfoFromTargetPreviewImage() methods.
     *  The data returned by getPreviewImage() is unaffected.
     *  No ownership of the data pointer is assumed.
     */
    void   putPreviewImage(uchar* data);

    /** Set the color profile of the preview image.
     */
    void   putPreviewIccProfile(const IccProfile& profile);

    /** Get colors from original, (unchanged) preview
     *   or target preview (set by putPreviewImage) image.
     */
    DColor getColorInfoFromOriginalImage(const QPoint& point);
    DColor getColorInfoFromPreviewImage(const QPoint& point);
    DColor getColorInfoFromTargetPreviewImage(const QPoint& point);

    /** Original image information.
     */
    int  originalWidth();
    int  originalHeight();
    bool originalSixteenBit();
    bool originalHasAlpha();

    /** Original image metadata.
     */
    IccProfile getOriginalIccProfile();
    KExiv2Data getOriginalMetadata();
    void       setOriginalMetadata(const KExiv2Data& meta);

    /** Get photograph information from original image.
     */
    PhotoInfoContainer getPhotographInformation() const;

    /** Standard methods to get/set preview information.
     */
    int  previewWidth();
    int  previewHeight();
    bool previewHasAlpha();
    bool previewSixteenBit();

    /** Sets preview size and returns new preview data as with getPreviewImage.
     *  The parameters are only hints, previewWidth() and previewHeight()
     *  may differ from w and h.
     */
    uchar* setPreviewImageSize(int w, int h) const;

    /** Standard methods to get image selection information.
     */
    int  selectedWidth();
    int  selectedHeight();

    /** Get selected (X, Y) position on the top/left corner of the original image.
     */
    int  selectedXOrg();
    int  selectedYOrg();

    /** Convert depth of original image.
     */
    void convertOriginalColorDepth(int depth);

    /** Convert a DImg image to a pixmap for screen using color
     *  managed view if necessary.
     */
    QPixmap convertToPixmap(DImg& img) const;

    /** Paint the current target preview image (or the preview image,
     *  if putPreviewImage has not been called) on the given paint device.
     *   at x|y, with given maximum width and height.
     */
    void paint(QPaintDevice* device, int x, int y, int w, int h, QPainter* painter=0);

private:

    class ImageIfacePriv;
    ImageIfacePriv* const d;
};

} // namespace Digikam

#endif /* IMAGEIFACE_H */
