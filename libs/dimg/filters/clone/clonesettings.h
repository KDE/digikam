/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-07-08
 * Description : a digiKam image plugin to clone area .
 *
 * Copyright (C) 2011-07-08 by Zhang Jie <zhangjiehangyuan2005 dot at gmail dot com>
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

#ifndef CLONESETTINGS_H
#define CLONESETTINGS_H

//Qt includes

#include <QWidget>

//KDE includes

#include <kconfig.h>

//Local includes

#include "digikam_export.h"
#include "clonecontainer.h"

namespace Digikam
{

class CloneSettingsPriv;

class DIGIKAM_EXPORT CloneSettings : public QWidget
{
    Q_OBJECT

public:

    CloneSettings(QWidget* parent);
    ~CloneSettings();

    CloneContainer defaultSettings() const;
    void resetToDefault();
    CloneContainer settings() const;
    void setSettings(const CloneContainer& settings);

    void readSettings(KConfigGroup& group);
    void writeSettings(KConfigGroup& group);

    void blockWidgetSignals(bool b);

Q_SIGNALS:

    void signalSettingsChanged();

public Q_SLOTS:

    void slotBrushIdChanged(int id);
    void slotSelectModeChanged();
    void slotDrawModeChanged();

private:

    CloneSettingsPriv* const d;
};

} // namespace Digikam

#endif // CLONESETTINGS_H
