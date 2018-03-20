/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-08-12
 * Description : advanced settings for camera interface.
 *
 * Copyright (C) 2011-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef ADVANCEDSETTINGS_H
#define ADVANCEDSETTINGS_H

// Qt includes

#include <QWidget>
#include <QDateTime>

// KDE includes

#include <kconfiggroup.h>

// Local settings

#include "downloadsettings.h"

namespace Digikam
{

class AdvancedSettings : public QWidget
{
    Q_OBJECT

public:

    explicit AdvancedSettings(QWidget* const parent = 0);
    ~AdvancedSettings();

    void readSettings(KConfigGroup& group);
    void saveSettings(KConfigGroup& group);

    DownloadSettings settings() const;

Q_SIGNALS:

    void signalDownloadNameChanged();

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* ADVANCEDSETTINGS_H */
