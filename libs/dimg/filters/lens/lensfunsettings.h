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

#ifndef LENSFUNSETTINGS_H
#define LENSFUNSETTINGS_H

// Qt includes

#include <QWidget>

// Local includes

#include "dmetadata.h"
#include "lensfunfilter.h"
#include "digikam_export.h"

class QCheckBox;

namespace KDcrawIface
{
class RComboBox;
class RDoubleNumInput;
}

using namespace KDcrawIface;

namespace Digikam
{

class DIGIKAM_EXPORT LensFunSettings : public QWidget
{
    Q_OBJECT

public:

    typedef QMap<QString,QString> Device;
    typedef const lfCamera*       DevicePtr;
    typedef const lfLens*         LensPtr;

public:

    LensFunSettings(QWidget* parent);
    virtual ~LensFunSettings();

//    Device getDevice();
    void setDevice(Device&);

    LensFunIface* getKLFObject(){ return m_klf; };

public Q_SLOTS:

    void findFromMetadata(const DMetadata&);

Q_SIGNALS:

    void signalLensSettingsChanged();

protected Q_SLOTS:

    void slotUpdateCombos();
    void slotUpdateLensCombo();
    void slotUseExif(int);
    void slotLensSelected();
    void slotFocalChanged(double);
    void slotApertureChanged(double);
    void slotDistanceChanged(double);

private:

    void findFromMetadata();

private:

    QCheckBox*       m_exifUsage;

    RComboBox*       m_make;
    RComboBox*       m_model;
    RComboBox*       m_lens;

    RDoubleNumInput* m_focal;
    RDoubleNumInput* m_aperture;
    RDoubleNumInput* m_distance;

    DMetadata        m_metadata;

    LensFunIface*    m_klf;
};

}  // namespace Digikam

#endif /* LENSFUNSETTINGS_H */
