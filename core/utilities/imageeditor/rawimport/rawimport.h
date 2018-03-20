/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-08-20
 * Description : Raw import tool
 *
 * Copyright (C) 2008-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef RAW_IMPORT_DLG_H
#define RAW_IMPORT_DLG_H

// Qt includes

#include <QUrl>

// Local includes

#include "editortool.h"
#include "dimg.h"
#include "digikam_export.h"

namespace Digikam
{

class DRawDecoderSettings;

class DIGIKAM_EXPORT RawImport : public EditorToolThreaded
{
    Q_OBJECT

public:

    explicit RawImport(const QUrl& url, QObject* const parent);
    ~RawImport();

    DRawDecoding rawDecodingSettings()      const;
    DImg         postProcessedImage()       const;
    bool         hasPostProcessedImage()    const;
    bool         demosaicingSettingsDirty() const;

private:

    void setBusy(bool busy);
    void preparePreview();
    void setPreviewImage();
    void setBackgroundColor(const QColor& bg);
    void ICCSettingsChanged();
    void exposureSettingsChanged();

private Q_SLOTS:

    void slotInit();

    void slotLoadingStarted();
    void slotDemosaicedImage();
    void slotLoadingFailed();
    void slotLoadingProgress(float);
    void slotScaleChanged();

    void slotUpdatePreview();
    void slotAbort();

    void slotOk();
    void slotCancel();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // RAW_IMPORT_DLG_H
