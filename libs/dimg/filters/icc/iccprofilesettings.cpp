/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-02-17
 * Description : Icc profile settings view.
 *
 * Copyright (C) 2010-2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "iccprofilesettings.moc"

// Qt includes

#include <QCache>
#include <QGridLayout>
#include <QLabel>
#include <QString>
#include <QPushButton>

// KDE includes

#include <kdebug.h>
#include <kdialog.h>
#include <klocale.h>
#include <kapplication.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kstandarddirs.h>

// LibKDcraw includes

#include <libkdcraw/rnuminput.h>
#include <libkdcraw/rexpanderbox.h>

// Local includes

#include "iccprofilescombobox.h"
#include "iccprofileinfodlg.h"
#include "iccsettings.h"

using namespace KDcrawIface;

namespace Digikam
{

class IccProfilesSettings::Private
{
public:

    Private() :
        profilesBox(0)
    {
        favoriteProfiles.setMaxCost(10);
    }

    static const QString  configRecentlyUsedProfilesEntry;
    QCache<QString, bool> favoriteProfiles;
    IccProfilesComboBox*  profilesBox;
};

const QString IccProfilesSettings::Private::configRecentlyUsedProfilesEntry("Recently Used Profiles");

// --------------------------------------------------------

IccProfilesSettings::IccProfilesSettings(QWidget* const parent)
    : KVBox(parent),
      d(new Private)
{
    QLabel* const newProfileLabel  = new QLabel(i18n("Convert to:"), this);
    d->profilesBox                 = new IccProfilesComboBox(this);
    d->profilesBox->addProfilesSqueezed(IccSettings::instance()->workspaceProfiles());
    d->profilesBox->setWhatsThis(i18n("Select the profile of the color space to convert to."));
    newProfileLabel->setBuddy(d->profilesBox);
    QPushButton* const newProfInfo = new QPushButton(i18n("Info..."), this);

    layout()->setAlignment(newProfInfo, Qt::AlignLeft);
    setMargin(0);
    setSpacing(KDialog::spacingHint());

    // -------------------------------------------------------------

    connect(d->profilesBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotProfileChanged()));

    connect(newProfInfo, SIGNAL(clicked()),
            this, SLOT(slotNewProfInfo()));
}

IccProfilesSettings::~IccProfilesSettings()
{
    delete d;
}

void IccProfilesSettings::slotNewProfInfo()
{
    ICCProfileInfoDlg infoDlg(kapp->activeWindow(), QString(), d->profilesBox->currentProfile());
    infoDlg.exec();
}

void IccProfilesSettings::slotProfileChanged()
{
    d->favoriteProfiles.insert(d->profilesBox->currentProfile().filePath(), new bool(true));
    emit signalSettingsChanged();
}

IccProfile IccProfilesSettings::currentProfile() const
{
    return d->profilesBox->currentProfile();
}

void IccProfilesSettings::setCurrentProfile(const IccProfile& prof)
{
    blockSignals(true);
    d->profilesBox->setCurrentProfile(prof);
    blockSignals(false);
}

void IccProfilesSettings::resetToDefault()
{
    blockSignals(true);
    d->profilesBox->setCurrentIndex(0);
    blockSignals(false);
}

IccProfile IccProfilesSettings::defaultProfile() const
{
    return d->profilesBox->itemData(0).value<IccProfile>();
}

void IccProfilesSettings::readSettings(KConfigGroup& group)
{
    QStringList lastProfiles = group.readPathEntry(d->configRecentlyUsedProfilesEntry, QStringList());

    foreach(const QString& path, lastProfiles)
    {
        d->favoriteProfiles.insert(path, new bool(true));
    }
}

void IccProfilesSettings::writeSettings(KConfigGroup& group)
{
    group.writePathEntry(d->configRecentlyUsedProfilesEntry, d->favoriteProfiles.keys());
}

// Static Methods.

QStringList IccProfilesSettings::favoriteProfiles(KConfigGroup& group)
{
    Private d;
    return group.readPathEntry(d.configRecentlyUsedProfilesEntry, QStringList());
}

}  // namespace Digikam
