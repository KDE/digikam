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

#include "iccprofilescombobox.h"

// Qt includes

#include <QFileInfo>

// KDE includes

#include <kdebug.h>
#include <klocale.h>

// Local includes

namespace Digikam
{

IccProfilesComboBox::IccProfilesComboBox(QWidget *parent)
                : KDcrawIface::SqueezedComboBox( parent )
{
}

IccProfilesComboBox::~IccProfilesComboBox()
{
}

bool iccProfileLessThan(IccProfile a, IccProfile b)
{
    return a.description() < b.description();
}

void IccProfilesComboBox::addProfilesSqueezed(const QList<IccProfile>& givenProfiles)
{
    QList<IccProfile> profiles;
    QSet<QString> filePaths;
    foreach (IccProfile profile, givenProfiles)
    {
        QString filePath = profile.filePath();
        if (!profile.description().isNull() && (filePath.isNull() || !filePaths.contains(filePath)) )
        {
            profiles << profile;
            filePaths << filePath;
        }
    }

    qSort(profiles.begin(), profiles.end(), iccProfileLessThan);

    foreach (IccProfile profile, profiles)
    {
        QFileInfo info(profile.filePath());
        QString fileName = info.fileName();

        QString description = profile.description();
        if (!description.isEmpty() && !fileName.isEmpty())
            description = i18nc("<Profile Description> (<File Name>)", "%1 (%2)", description, fileName);
        else if (!fileName.isEmpty())
            description = fileName;
        else
            continue;

        addSqueezedItem(description, QVariant::fromValue(profile));
    }
}

void IccProfilesComboBox::addProfile(const IccProfile& profile, const QString& d)
{
    QString description = d;
    if (description.isNull())
        description = IccProfile(profile).description();
    addSqueezedItem(description, QVariant::fromValue(profile));
}

void IccProfilesComboBox::replaceProfilesSqueezed(const QList<IccProfile>& profiles)
{
    IccProfile current = currentProfile();
    clear();
    addProfilesSqueezed(profiles);
    setCurrentProfile(current);
}

void IccProfilesComboBox::setNoProfileIfEmpty(const QString& message)
{
    if (count() == 0)
    {
        setEnabled(false);
        addSqueezedItem(message);
        setCurrentIndex(0);
    }
}

IccProfile IccProfilesComboBox::currentProfile() const
{
    return itemData(currentIndex()).value<IccProfile>();
}

void IccProfilesComboBox::setCurrentProfile(const IccProfile& profile)
{
    if (profile.isNull())
    {
        setCurrentIndex(-1);
        return;
    }

    const int size = count();
    for (int i=0; i<size; i++)
    {
        if (itemData(i).value<IccProfile>() == profile)
        {
            setCurrentIndex(i);
            return;
        }
    }
    setCurrentIndex(-1);
}

} // namespace Digikam

