/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date   : 2004-08-23
 * Description : mics configuration setup tab
 * 
 * Copyright 2004 by Renchi Raju
 * Copyright 2005-2006 by Gilles Caulier
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

// Qt includes.

#include <qlayout.h>
#include <qvgroupbox.h>
#include <qcheckbox.h>

// KDE includes.

#include <klocale.h>
#include <kdialog.h>

// Local includes.

#include "albumsettings.h"
#include "setupmisc.h"

namespace Digikam
{

class SetupMiscPriv
{
public:

    SetupMiscPriv()
    {
        showSplashCheck = 0;
        useTrashCheck   = 0;
        scanAtStart     = 0;
    }

    QCheckBox* showSplashCheck;
    QCheckBox* useTrashCheck;
    QCheckBox* scanAtStart;
};

SetupMisc::SetupMisc(QWidget* parent)
         : QWidget( parent )
{
    d = new SetupMiscPriv;

    QVBoxLayout *mainLayout = new QVBoxLayout(parent);
    QVBoxLayout *layout = new QVBoxLayout( this, 0, KDialog::spacingHint() );

   // --------------------------------------------------------

   d->useTrashCheck = new QCheckBox(i18n("&Deleting items should move them to trash"), this);
   layout->addWidget(d->useTrashCheck);

   // --------------------------------------------------------

   d->showSplashCheck = new QCheckBox(i18n("&Show splash screen at startup"), this);
   layout->addWidget(d->showSplashCheck);

   // --------------------------------------------------------

   d->scanAtStart = new QCheckBox(i18n("&Scan for new items on startup (slows down startup)"), this);
   layout->addWidget(d->scanAtStart);

   // --------------------------------------------------------

   layout->addStretch();
   readSettings();
   adjustSize();
   mainLayout->addWidget(this);
}

SetupMisc::~SetupMisc()
{
    delete d;
}

void SetupMisc::applySettings()
{
    AlbumSettings* settings = AlbumSettings::instance();

    settings->setShowSplashScreen(d->showSplashCheck->isChecked());
    settings->setUseTrash(d->useTrashCheck->isChecked());
    settings->setScanAtStart(d->scanAtStart->isChecked());
    settings->saveSettings();
}

void SetupMisc::readSettings()
{
    AlbumSettings* settings = AlbumSettings::instance();

    d->showSplashCheck->setChecked(settings->getShowSplashScreen());
    d->useTrashCheck->setChecked(settings->getUseTrash());
    d->scanAtStart->setChecked(settings->getScanAtStart());
}

}  // namespace Digikam
