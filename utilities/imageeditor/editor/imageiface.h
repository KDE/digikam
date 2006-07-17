/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         Gilles Caulier <caulier dot gilles at kdemail dot net> 
 * Date  : 2004-02-14
 * Description : image data interface for image plugins
 *
 * Copyright 2004-2005 by Renchi Raju, Gilles Caulier
 * Copyright 2006 by Gilles Caulier
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

// Qt includes.

#include <qglobal.h>
#include <qstring.h>

// KDE includes.

#include <klocale.h>

// Local includes.

#include "dimg.h"
#include "dcolor.h"
#include "photoinfocontainer.h"
#include "digikam_export.h"

#define MAX3(a, b, c) (QMAX(QMAX(a,b),b))
#define MIN3(a, b, c) (QMIN(QMIN(a,b),b))
#define ROUND(x) ((int) ((x) + 0.5))

class QPaintDevice;

namespace Digikam
{

class ImageIfacePriv;

class DIGIKAM_EXPORT ImageIface
{
public:

    ImageIface(int w=0, int h=0);
    ~ImageIface();

    /** Return image data for the current, scaled preview image.
        The preview...() methods provide the characteristics of the data
        (width, heigh, sixteen bit, alpha).
        Ownership of the returned buffer is passed to the caller.
    */
    uchar* getPreviewImage();

    /** Return image data for the current original image selection.
        The selectionWidth(), selectionHeight(), originalSixteenBit()
        and originalHasAlpha() methods provide the characteristics of the data.
        Ownership of the returned buffer is passed to the caller.
    */
    uchar* getImageSelection();

    /** Return image data for the original image.
        The preview...() methods provide the characteristics of the data.
        Ownership of the returned buffer is passed to the caller.
    */
    uchar* getOriginalImage();

    /** Return a pointer to the DImg object representing the original image.
        This object may not be modified or stored. Make copies if you need.
    */
    DImg*  getOriginalImg();

    /** Replace the image data of the original image with the given data.
        The characteristics of the data must match the characteristics of
        the original image as returned by the original...() methods,
        respectively the given width and height parameters.
        No ownership of the data pointer is assumed.
        If w == -1 and h == -1, the size is unchanged.
        Caller is an i18n'ed string that will be shown as the undo/redo action name.
    */
    void   putOriginalImage(const QString &caller, uchar* data, int w=-1, int h=-1);

    /** Embed the Color Profile we have used in ICC plugin when this option is 
        selected
    */
    void   setEmbeddedICCToOriginalImage(QString profilePath);

    /** Replace the data of the current original image selection with the given data.
        The characteristics of the data must match the characteristics of the current
        selection as returned by the selectionWidth(), selectionHeight(),
        originalSixteenBit() and originalHasAlpha() methods.
        No ownership of the data pointer is assumed.
        Caller is an i18n'ed string that will be shown as the undo/redo action name.
    */
    void   putImageSelection(const QString &caller, uchar* data);

    /** Replace the stored target preview data with the given data.
        The characteristics of the data must match the characteristics of the current
        as returned by the preview...() methods.
        The target preview data is used by the paint() and
        getColorInfoFromTargetPreviewImage() methods.
        The data returned by getPreviewImage() is unaffected.
        No ownership of the data pointer is assumed.
    */
    void   putPreviewImage(uchar* data);

    /** Get colors from original, (unchanged) preview
        or target preview (set by putPreviewImage) image.
    */

    DColor getColorInfoFromOriginalImage(QPoint point);
    DColor getColorInfoFromPreviewImage(QPoint point);
    DColor getColorInfoFromTargetPreviewImage(QPoint point);

    /** Original image information.*/
    int  originalWidth();
    int  originalHeight();
    bool originalSixteenBit();
    bool originalHasAlpha();

    /** Original image metadata.*/
    QByteArray getEmbeddedICCFromOriginalImage();
    QByteArray getExifFromOriginalImage();
    QByteArray getIptcFromOriginalImage();

    /** Get photograph informations from original image.*/ 
    PhotoInfoContainer getPhotographInformations() const;

    /** Standard methods to get/set preview informations.*/
    int  previewWidth();
    int  previewHeight();
    bool previewHasAlpha();
    bool previewSixteenBit();

    /** Sets preview size and returns new preview data as with getPreviewImage.
        The parameters are only hints, previewWidth() and previewHeight()
        may differ from w and h.
    */
    uchar* setPreviewImageSize(int w, int h);

    /** Standard methods to get image selection informations.*/
    int  selectedWidth();
    int  selectedHeight();

    /** Get selected (X, Y) position on the top/left corner of the original image.*/
    int  selectedXOrg();
    int  selectedYOrg();

    /** Set BCG correction for preview and original image */
    void setPreviewBCG(double brightness, double contrast, double gamma, bool overIndicator=false);
    void setOriginalBCG(double brightness, double contrast, double gamma);

    /** Convert depth of original image */
    void convertOriginalColorDepth(int depth);

    /** Paint the current target preview image (or the preview image,
        if putPreviewImage has not been called) on the given paint device.
        at x|y, with given maximum width and height.
    */
    void paint(QPaintDevice* device, int x, int y, int w, int h);

private:

    ImageIfacePriv* d;
};

}

#endif /* IMAGEIFACE_H */
