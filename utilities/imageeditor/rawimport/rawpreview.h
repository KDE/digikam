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

// Qt includes

#include <QImage>
#include <QPixmap>
#include <QResizeEvent>

// KDE includes

#include <kurl.h>

// Local includes

#include "dimg.h"
#include "previewwidget.h"
#include "digikam_export.h"

class QPixmap;

namespace Digikam
{

class LoadingDescription;
class RawPreviewPriv;

class DIGIKAM_EXPORT RawPreview : public PreviewWidget
{

Q_OBJECT

public:

    RawPreview(const KUrl& url, QWidget *parent);
    ~RawPreview();

    DImg& demosaicedImage() const;
    DImg& postProcessedImage() const;

    void setDecodingSettings(const DRawDecoding& settings);
    void setPostProcessedImage(const DImg& image);

    void cancelLoading();

Q_SIGNALS:

    void signalLoadingStarted();
    void signalLoadingProgress(float);
    void signalLoadingFailed();
    void signalDemosaicedImage();
    void signalPostProcessedImage();

protected:

    void resizeEvent(QResizeEvent* e);

private Q_SLOTS:

    void slotLoadingProgress(const LoadingDescription& description, float progress);
    void slotImageLoaded(const LoadingDescription& description, const DImg& image);
    void slotThemeChanged();
    void slotCornerButtonPressed();
    void slotPanIconSelectionMoved(const QRect&, bool);
    void slotPanIconHiden();

private:

    void setdemosaicedImg(const DImg& image);
    void postProcessing(const DRawDecoding& settings);
    int  previewWidth();
    int  previewHeight();
    bool previewIsNull();
    void resetPreview();
    void zoomFactorChanged(double zoom);
    void updateZoomAndSize(bool alwaysFitToWindow);
    inline void paintPreview(QPixmap *pix, int sx, int sy, int sw, int sh);

private:

    RawPreviewPriv* const d;
};

}  // namespace Digikam

#endif /* RAWPREVIEW_H */
