/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-10-10
 * Description : Lut3D color adjustment tool.
 *
 * Copyright (C) 2015 by Andrej Krutak <dev at andree dot sk>
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

#ifndef LUT3DSETTINGS_H
#define LUT3DSETTINGS_H

// Qt includes

#include <QWidget>

// KDE includes

#include <kconfiggroup.h>

// Local includes

#include "digikam_export.h"
#include "lut3dfilter.h"

namespace Digikam
{

class DIGIKAM_EXPORT Lut3DSettings : public QWidget
{
    Q_OBJECT

public:

    explicit Lut3DSettings(QWidget* const parent, bool useGenericImg = false);
    ~Lut3DSettings();

    Lut3DContainer defaultSettings() const;
    void resetToDefault();

    Lut3DContainer settings() const;
    void setSettings(const Lut3DContainer& settings);

    void readSettings(KConfigGroup& group);
    void writeSettings(KConfigGroup& group);

    void findLuts();

    void startPreviewFilters();

Q_SIGNALS:

    void signalSettingsChanged();

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* LUT3DSETTINGS_H */
