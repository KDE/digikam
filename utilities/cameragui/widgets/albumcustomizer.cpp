/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-08-11
 * Description : a widget to customize album name created by
 *               camera interface.
 *
 * Copyright (C) 2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "albumcustomizer.moc"

// Qt includes

#include <QVBoxLayout>
#include <QLabel>
#include <QCheckBox>

// KDE includes

#include <kdialog.h>
#include <klocale.h>
#include <kcombobox.h>
#include <khbox.h>

namespace Digikam
{

class AlbumCustomizer::AlbumCustomizerPriv
{
public:

    AlbumCustomizerPriv()
      : autoAlbumDateCheck(0),
        autoAlbumExtCheck(0),
        folderDateLabel(0),
        folderDateFormat(0)
    {
    }

    QCheckBox* autoAlbumDateCheck;
    QCheckBox* autoAlbumExtCheck;

    QLabel*    folderDateLabel;

    KComboBox* folderDateFormat;
};

AlbumCustomizer::AlbumCustomizer(QWidget* parent)
    : QWidget(parent), d(new AlbumCustomizerPriv)
{
    QVBoxLayout* albumVlay = new QVBoxLayout(this);
    d->autoAlbumExtCheck   = new QCheckBox(i18n("Extension-based sub-albums"), this);
    d->autoAlbumDateCheck  = new QCheckBox(i18n("Date-based sub-albums"), this);
    KHBox* hbox1           = new KHBox(this);
    d->folderDateLabel     = new QLabel(i18n("Date format:"), hbox1);
    d->folderDateFormat    = new KComboBox(hbox1);
    d->folderDateFormat->insertItem(IsoDateFormat,   i18n("ISO"));
    d->folderDateFormat->insertItem(TextDateFormat,  i18n("Full Text"));
    d->folderDateFormat->insertItem(LocalDateFormat, i18n("Local Settings"));

    albumVlay->addWidget(d->autoAlbumExtCheck);
    albumVlay->addWidget(d->autoAlbumDateCheck);
    albumVlay->addWidget(hbox1);
    albumVlay->addStretch();
    albumVlay->setMargin(KDialog::spacingHint());
    albumVlay->setSpacing(KDialog::spacingHint());

    setWhatsThis( i18n("Set how digiKam creates albums automatically when downloading."));
    d->autoAlbumExtCheck->setWhatsThis( i18n("Enable this option if you want to download your "
                                             "pictures into automatically created file extension-based sub-albums of the destination "
                                             "album. This way, you can separate JPEG and RAW files as they are downloaded from your camera."));
    d->autoAlbumDateCheck->setWhatsThis( i18n("Enable this option if you want to "
                                              "download your pictures into automatically created file date-based sub-albums "
                                              "of the destination album."));
    d->folderDateFormat->setWhatsThis( i18n("<p>Select your preferred date format used to "
                                            "create new albums. The options available are:</p>"
                                            "<p><b>ISO</b>: the date format is in accordance with ISO 8601 "
                                            "(YYYY-MM-DD). E.g.: <i>2006-08-24</i></p>"
                                            "<p><b>Full Text</b>: the date format is in a user-readable string. "
                                            "E.g.: <i>Thu Aug 24 2006</i></p>"
                                            "<p><b>Local Settings</b>: the date format depending on KDE control panel settings.</p>"));

    // --------------------------------------------------------------------------------------

    connect(d->autoAlbumDateCheck, SIGNAL(toggled(bool)),
            d->folderDateFormat, SLOT(setEnabled(bool)));

    connect(d->autoAlbumDateCheck, SIGNAL(toggled(bool)),
            d->folderDateLabel, SLOT(setEnabled(bool)));
}

AlbumCustomizer::~AlbumCustomizer()
{
    delete d;
}

void AlbumCustomizer::readSettings(KConfigGroup& group)
{
    d->autoAlbumDateCheck->setChecked(group.readEntry("AutoAlbumDate",       false));
    d->autoAlbumExtCheck->setChecked(group.readEntry("AutoAlbumExt",         false));
    d->folderDateFormat->setCurrentIndex(group.readEntry("FolderDateFormat", (int)IsoDateFormat));

    d->folderDateFormat->setEnabled(d->autoAlbumDateCheck->isChecked());
    d->folderDateLabel->setEnabled(d->autoAlbumDateCheck->isChecked());
}

void AlbumCustomizer::saveSettings(KConfigGroup& group)
{
    group.writeEntry("AutoAlbumDate",    d->autoAlbumDateCheck->isChecked());
    group.writeEntry("AutoAlbumExt",     d->autoAlbumExtCheck->isChecked());
    group.writeEntry("FolderDateFormat", d->folderDateFormat->currentIndex());
}

bool AlbumCustomizer::autoAlbumDateEnabled() const
{
    return d->autoAlbumDateCheck->isChecked();
}

bool AlbumCustomizer::autoAlbumExtEnabled() const
{
    return d->autoAlbumExtCheck->isChecked();
}

int AlbumCustomizer::folderDateFormat() const
{
    return d->folderDateFormat->currentIndex();
}

}  // namespace Digikam
