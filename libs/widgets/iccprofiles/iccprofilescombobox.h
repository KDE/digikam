/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-11
 * Description : a combo box containing ICC profiles
 *
 * Copyright (C) 2009 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
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

#ifndef ICCPROFILESCOMBOBOX_H
#define ICCPROFILESCOMBOBOX_H

// KDE includes

// LibKDcraw includes

#include <libkdcraw/squeezedcombobox.h>

// Local includes

#include "digikam_export.h"
#include "iccprofile.h"

namespace Digikam
{

class DIGIKAM_EXPORT IccProfilesComboBox : public KDcrawIface::SqueezedComboBox
{

    Q_OBJECT

public:

    IccProfilesComboBox(QWidget *parent = 0);
    ~IccProfilesComboBox();

    void addProfilesSqueezed(const QList<IccProfile>& profiles);
    void addProfile(const IccProfile& profile, const QString& description = QString());
    void replaceProfilesSqueezed(const QList<IccProfile>& profiles);
    void setNoProfileIfEmpty(const QString& message);
    IccProfile currentProfile() const;
    void setCurrentProfile(const IccProfile& profile);

};

} // namespace Digikam

#endif /* ICCPROFILESCOMBOBOX_H */
