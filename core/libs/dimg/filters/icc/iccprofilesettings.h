/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-02-17
 * Description : Icc profile settings view.
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

#ifndef ICCPROFILESETTINGS_H
#define ICCPROFILESETTINGS_H

// Local includes

#include <QWidget>

// KDE includes

#include <kconfiggroup.h>

// Local includes

#include "dlayoutbox.h"
#include "digikam_export.h"
#include "iccprofile.h"

namespace Digikam
{

class DIGIKAM_EXPORT IccProfilesSettings : public DVBox
{
    Q_OBJECT

public:

    explicit IccProfilesSettings(QWidget* const parent=0);
    ~IccProfilesSettings();

    IccProfile defaultProfile() const;
    void resetToDefault();

    IccProfile currentProfile() const;
    void setCurrentProfile(const IccProfile& prof);

    void readSettings(KConfigGroup& group);
    void writeSettings(KConfigGroup& group);

    static QStringList favoriteProfiles(KConfigGroup& group);

Q_SIGNALS:

    void signalSettingsChanged();

private Q_SLOTS:

    void slotNewProfInfo();
    void slotProfileChanged();

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* ICCPROFILESETTINGS_H */
