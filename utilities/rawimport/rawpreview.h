/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-08-04
 * Description : RAW preview widget.
 *
 * Copyright (C) 2008 Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef RAWPREVIEW_H
#define RAWPREVIEW_H

// Qt includes.

#include <qstring.h>
#include <qimage.h>

// Local includes

#include "dimg.h"
#include "previewwidget.h"
#include "digikam_export.h"

class QPixmap;

namespace Digikam
{

class LoadingDescription;
class ImageInfo;
class RawPreviewPriv;

class DIGIKAM_EXPORT RawPreview : public PreviewWidget
{

Q_OBJECT

public:

    RawPreview(QWidget *parent=0);
    ~RawPreview();

    void setImage(const DImg& image);
    DImg& getImage() const;

    void setImageInfo(ImageInfo *info);
    void setDecodingSettings(const KDcrawIface::RawDecodingSettings& settings);

signals:

    void signalPreviewed(const DImg&);

protected:

    void resizeEvent(QResizeEvent* e);

private slots:

    void slotImageLoaded(const LoadingDescription &loadingDescription, const DImg &image);
    void slotThemeChanged();
    void slotCornerButtonPressed();
    void slotPanIconSelectionMoved(const QRect&, bool);
    void slotPanIconHiden();

private:

    int  previewWidth();
    int  previewHeight();
    bool previewIsNull();
    void resetPreview();
    void zoomFactorChanged(double zoom);
    void updateZoomAndSize(bool alwaysFitToWindow);
    inline void paintPreview(QPixmap *pix, int sx, int sy, int sw, int sh);

private:

    RawPreviewPriv* d;
};

}  // NameSpace Digikam

#endif /* RAWPREVIEW_H */
