/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-22
 * Description : noise reduction settings view.
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef NRSETTINGS_H
#define NRSETTINGS_H

// Local includes

#include <QWidget>

// KDE includes

#include <kconfiggroup.h>

// Local includes

#include "digikam_export.h"
#include "nrfilter.h"

namespace Digikam
{

class DIGIKAM_EXPORT NRSettings : public QWidget
{
    Q_OBJECT

public:

    explicit NRSettings(QWidget* const parent);
    ~NRSettings();

    NRContainer defaultSettings() const;
    void resetToDefault();

    NRContainer settings() const;
    void setSettings(const NRContainer& settings);

    void setEstimateNoise(bool b);
    bool estimateNoise() const;

    void readSettings(KConfigGroup& group);
    void writeSettings(KConfigGroup& group);

    void loadSettings();
    void saveAsSettings();

Q_SIGNALS:

    void signalSettingsChanged();
    void signalEstimateNoise();

private Q_SLOTS:

    void slotDisableParameters(bool);

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* NRSETTINGS_H */
