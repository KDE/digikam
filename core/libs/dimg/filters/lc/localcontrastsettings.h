/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-03-09
 * Description : Local Contrast settings view.
 *               LDR ToneMapper <http://zynaddsubfx.sourceforge.net/other/tonemapping>
 *
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef LOCALCONTRASTSETTINGS_H
#define LOCALCONTRASTSETTINGS_H

// Local includes

#include <QWidget>

// KDE includes

#include <kconfiggroup.h>

// Local includes

#include "digikam_export.h"
#include "localcontrastcontainer.h"

namespace Digikam
{

class DIGIKAM_EXPORT LocalContrastSettings : public QWidget
{
    Q_OBJECT

public:

    explicit LocalContrastSettings(QWidget* const parent);
    ~LocalContrastSettings();

    LocalContrastContainer defaultSettings() const;
    void resetToDefault();

    LocalContrastContainer settings() const;
    void setSettings(const LocalContrastContainer& settings);

    void readSettings(KConfigGroup& group);
    void writeSettings(KConfigGroup& group);

    void loadSettings();
    void saveAsSettings();

Q_SIGNALS:

    void signalSettingsChanged();

private Q_SLOTS:

    void slotStageEnabled(int, bool);

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* LOCALCONTRASTSETTINGS_H */
