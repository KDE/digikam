/* ============================================================
 *
 * Date        : 2008-02-10
 * Description : a plugin to fix automatically camera lens aberrations
 *
 * Copyright (C) 2008 by Adrian Schroeter <adrian at suse dot de>
 * Copyright (C) 2008-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include "lensfunfilter.h"
#include "lensfuniface.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT LensFunCameraSelector : public QWidget
{
    Q_OBJECT

public:

    typedef QMap<QString, QString> Device;

public:

    LensFunCameraSelector(LensFunIface* iface, QWidget* parent);
    virtual ~LensFunCameraSelector();

//    Device getDevice();
    void setDevice(Device&);

public Q_SLOTS:

    void findFromMetadata(const DMetadata&);

Q_SIGNALS:

    void signalLensSettingsChanged();

protected Q_SLOTS:

    void slotUpdateCombos();
    void slotUpdateLensCombo();
    void slotUseExif(bool);
    void slotLensSelected();
    void slotFocalChanged(double);
    void slotApertureChanged(double);
    void slotDistanceChanged(double);

private:

    void findFromMetadata();

private:

    class LensFunCameraSelectorPriv;
    LensFunCameraSelectorPriv* const d;
};

}  // namespace Digikam

#endif /* LENSFUNCAMERASELECTOR_H */
