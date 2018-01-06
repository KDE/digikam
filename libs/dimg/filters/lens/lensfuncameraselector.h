/* ============================================================
 *
 * Date        : 2008-02-10
 * Description : a tool to fix automatically camera lens aberrations
 *
 * Copyright (C) 2008      by Adrian Schroeter <adrian at suse dot de>
 * Copyright (C) 2008-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef LENSFUNCAMERASELECTOR_H
#define LENSFUNCAMERASELECTOR_H

// Qt includes

#include <QWidget>

// Local includes

#include "dmetadata.h"
#include "digikam_export.h"
#include "lensfunfilter.h"
#include "lensfuniface.h"

namespace Digikam
{

class DIGIKAM_EXPORT LensFunCameraSelector : public QWidget
{
    Q_OBJECT

public:

    typedef QMap<QString, QString> Device;

public:

    explicit LensFunCameraSelector(QWidget* const parent=0);
    virtual ~LensFunCameraSelector();

    void setEnabledUseMetadata(bool b);

    void setUseMetadata(bool b);
    bool useMetadata() const;

    LensFunContainer settings();
    void             setSettings(const LensFunContainer& settings);

    void readSettings(KConfigGroup& group);
    void writeSettings(KConfigGroup& group);

    void resetToDefault();

    /** Special mode used with BQM which processes multiple items at the same time.
     */
    void setPassiveMetadataUsage(bool b);

    LensFunIface* iface() const;

    void setMetadata(const DMetadata&);

Q_SIGNALS:

    void signalLensSettingsChanged();

private Q_SLOTS:

    void slotUseMetadata(bool);
    void slotMakeSelected();
    void slotModelChanged();
    void slotModelSelected();
    void slotLensSelected();
    void slotFocalChanged();
    void slotApertureChanged();
    void slotDistanceChanged();

private:

    LensFunIface::MetadataMatch findFromMetadata();
    void refreshSettingsView();
    void populateDeviceCombos();
    void populateLensCombo();

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* LENSFUNCAMERASELECTOR_H */
