/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-08-11
 * Description : Raw import settings box
 *
 * Copyright (C) 2008-2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef RAWSETTINGSBOX_H
#define RAWSETTINGSBOX_H

// KDE includes

#include <kurl.h>

// Local includes

#include "editortoolsettings.h"
#include "dimg.h"
#include "digikam_export.h"

class KUrlRequester;

namespace Digikam
{

class CurvesWidget;

class DIGIKAM_EXPORT RawSettingsBox : public EditorToolSettings
{
    Q_OBJECT

public:

    RawSettingsBox(const KUrl& url, QWidget* const parent);
    ~RawSettingsBox();

    void setBusy(bool b);

    CurvesWidget* curvesWidget() const;
    DRawDecoding  settings()     const;

    void writeSettings();
    void readSettings();

    void setDemosaicedImage(DImg& img);
    void setPostProcessedImage(DImg& img);

    void resetSettings();

    void enableUpdateBtn(bool b);
    bool updateBtnEnabled() const;

Q_SIGNALS:

    void signalUpdatePreview();
    void signalAbortPreview();
    void signalPostProcessingChanged();

private Q_SLOTS:

    void slotDemosaicingChanged();
    void slotResetCurve();
    void slotFileDialogAboutToOpen(KUrlRequester* requester);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // RAWSETTINGSBOX_H
