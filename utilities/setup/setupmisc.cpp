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
#include <qhgroupbox.h>
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

   QHGroupBox *useTrashGroupBox = new QHGroupBox(i18n("Delete Settings"),
                                               parent);
   m_useTrashCheck = new QCheckBox(i18n("Deleting items should move them to trash"),
                                     useTrashGroupBox);
   layout->addWidget(useTrashGroupBox);

   // --------------------------------------------------------

   QHGroupBox *splashGroupBox = new QHGroupBox(i18n("Splash Screen Settings"),
                                               parent);
   m_showSplashCheck = new QCheckBox(i18n("Show Splash Screen at startup"),
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
