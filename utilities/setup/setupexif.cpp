//////////////////////////////////////////////////////////////////////////////
//
//    SETUPEXIF.CPP
//
//    Copyright (C) 2003-2004 Gilles CAULIER <caulier dot gilles at free.fr>
//                            Ralf Holzer <ralf at well.com>
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
//////////////////////////////////////////////////////////////////////////////

// QT includes.

#include <qlayout.h>
#include <qvbuttongroup.h>
#include <qvgroupbox.h>
#include <qhgroupbox.h>
#include <qgroupbox.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qwhatsthis.h>

// KDE includes.

#include <klocale.h>
#include <kdialog.h>

// // Local includes.

#include "albumsettings.h"
#include "setupexif.h"


SetupExif::SetupExif(QWidget* parent )
         : QWidget(parent)
{
   QVBoxLayout *layout = new QVBoxLayout( parent, 10);
   layout->setSpacing( KDialog::spacingHint() );

   // --------------------------------------------------------

   QGroupBox *iconExifGroup = new QGroupBox(1, Qt::Horizontal, 
                                            i18n("EXIF actions"),
                                            parent);

   iconSaveExifBox_ = new QCheckBox(iconExifGroup);
   iconSaveExifBox_->setText(i18n("Save Album items comments as EXIF Comments in Jpeg images"));

   iconExifRotateBox_ = new QCheckBox(iconExifGroup);
   iconExifRotateBox_->setText(i18n("Rotate images and thumbnails according to EXIF tag"));

   layout->addWidget(iconExifGroup);

   // --------------------------------------------------------

   layout->addStretch();

   readSettings();
}

SetupExif::~SetupExif()
{
}

void SetupExif::applySettings()
{
    AlbumSettings* settings = AlbumSettings::instance();
    
    if (!settings) return;

    settings->setSaveExifComments(iconSaveExifBox_->isChecked());
    settings->setExifRotate(iconExifRotateBox_->isChecked());

    settings->saveSettings();
}

void SetupExif::readSettings()
{
    AlbumSettings* settings = AlbumSettings::instance();
    
    if (!settings) return;

    iconSaveExifBox_->setChecked(settings->getSaveExifComments());
    iconExifRotateBox_->setChecked(settings->getExifRotate());
}


#include "setupexif.moc"
