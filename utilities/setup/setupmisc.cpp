/* ============================================================
 * File  : setupmisc.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-08-23
 * Description :
 *
 * Copyright 2004 by Renchi Raju

 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published bythe Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include <qlayout.h>
#include <qvgroupbox.h>
#include <qcheckbox.h>

#include <klocale.h>
#include <kdialog.h>

#include "albumsettings.h"
#include "setupmisc.h"

SetupMisc::SetupMisc(QWidget* parent)
    : QWidget( parent )
{
   QVBoxLayout *layout = new QVBoxLayout( parent, 10);
   layout->setSpacing( KDialog::spacingHint() );

   // --------------------------------------------------------

   QGroupBox *trashGroupBox = new QGroupBox(i18n("Delete Settings"),
                                               parent);
   trashGroupBox->setColumnLayout(0, Qt::Vertical);
   trashGroupBox->layout()->setSpacing(5);
   trashGroupBox->layout()->setMargin(5);

   QGridLayout* lay = new QGridLayout(trashGroupBox->layout());

   m_useTrashCheck = new QCheckBox(i18n("Deleting items should move them to trash"),
                                   trashGroupBox);
   lay->addMultiCellWidget(m_useTrashCheck, 0, 0, 0, 1);
   layout->addWidget(trashGroupBox);

   // --------------------------------------------------------

   QVGroupBox *splashGroupBox = new QVGroupBox(i18n("Splash Screen Settings"),
                                               parent);
   m_showSplashCheck = new QCheckBox(i18n("Show splash screen at startup"),
                                     splashGroupBox);
   layout->addWidget(splashGroupBox);

   // --------------------------------------------------------

   layout->addStretch();
   readSettings();
}

SetupMisc::~SetupMisc()
{

}

void SetupMisc::applySettings()
{
    AlbumSettings* settings = AlbumSettings::instance();

    settings->setShowSplashScreen(m_showSplashCheck->isChecked());
    settings->setUseTrash(m_useTrashCheck->isChecked());
    settings->saveSettings();
}

void SetupMisc::readSettings()
{
    AlbumSettings* settings = AlbumSettings::instance();

    m_showSplashCheck->setChecked(settings->getShowSplashScreen());
    m_useTrashCheck->setChecked(settings->getUseTrash());
}
