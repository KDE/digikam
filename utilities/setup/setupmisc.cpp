/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-08-23
 * Description :
 *
 * Copyright 2004 by Renchi Raju

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

SetupMisc::SetupMisc(QWidget* parent)
    : QWidget( parent )
{
    QVBoxLayout *mainLayout = new QVBoxLayout(parent);

    QVBoxLayout *layout = new QVBoxLayout( this, 0, KDialog::spacingHint() );

   // --------------------------------------------------------
   m_useTrashCheck = new QCheckBox(i18n("&Deleting items should move them to trash"),
                                   this);
   layout->addWidget(m_useTrashCheck);

   // --------------------------------------------------------
   m_showSplashCheck = new QCheckBox(i18n("&Show splash screen at startup"),
                                     this);
   layout->addWidget(m_showSplashCheck);

   // --------------------------------------------------------
   m_scanAtStart = new QCheckBox(i18n("&Scan for new items on startup (slows down startup)"),
                                     this);
   layout->addWidget(m_scanAtStart);

   // --------------------------------------------------------

   layout->addStretch();
   readSettings();
   adjustSize();
   mainLayout->addWidget(this);
}

SetupMisc::~SetupMisc()
{
}

void SetupMisc::applySettings()
{
    AlbumSettings* settings = AlbumSettings::instance();

    settings->setShowSplashScreen(m_showSplashCheck->isChecked());
    settings->setUseTrash(m_useTrashCheck->isChecked());
    settings->setScanAtStart(m_scanAtStart->isChecked());
    settings->saveSettings();
}

void SetupMisc::readSettings()
{
    AlbumSettings* settings = AlbumSettings::instance();

    m_showSplashCheck->setChecked(settings->getShowSplashScreen());
    m_useTrashCheck->setChecked(settings->getUseTrash());
    m_scanAtStart->setChecked(settings->getScanAtStart());
}

}  // namespace Digikam
