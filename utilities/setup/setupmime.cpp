//////////////////////////////////////////////////////////////////////////////
//
//    SETUPMIME.CPP
//
//    Copyright (C) 2004 Gilles CAULIER <caulier dot gilles at free.fr>
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
#include <qhgroupbox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qcombobox.h>

// KDE includes.

#include <klocale.h>
#include <kdialog.h>
#include <klineeditdlg.h>

// // Local includes.

#include "albumsettings.h"
#include "setupmime.h"


SetupMime::SetupMime(QWidget* parent )
         : QWidget(parent)
{
   QVBoxLayout *layout = new QVBoxLayout( parent, 10);
   layout->setSpacing( KDialog::spacingHint() );

   // --------------------------------------------------------

   QHGroupBox *imageFileFilterBox = new QHGroupBox(i18n("Image files"), parent);

   QLabel *imageFileFilterLabel = new QLabel(imageFileFilterBox);
   imageFileFilterLabel->setText(i18n("Show only image files with extensions:"));

   imageFileFilterEdit = new QLineEdit(imageFileFilterBox);

   layout->addWidget(imageFileFilterBox);
   
   // --------------------------------------------------------
   
   QGroupBox *movieFileFilterBox = new QGroupBox(i18n("Movie files"), parent);
   QGridLayout* movieFileFilterGroupLayout = new QGridLayout( movieFileFilterBox->layout() );
   movieFileFilterGroupLayout->setAlignment( Qt::AlignTop );
   movieFileFilterGroupLayout->addMultiCellWidget( movieFileFilterBox, 0, 2, 0, 2 );
   
   QLabel *movieFileFilterLabel = new QLabel(movieFileFilterBox);
   movieFileFilterLabel->setText(i18n("Show only movie files with extensions:"));
   movieFileFilterGroupLayout->addWidget( movieFileFilterLabel, 0, 1);
   
   movieFileFilterEdit = new QLineEdit(movieFileFilterBox);
   movieFileFilterGroupLayout->addWidget( movieFileFilterEdit, 0, 2);

   QLabel *moviePlayerLabel = new QLabel(movieFileFilterBox);
   moviePlayerLabel->setText(i18n("open movie files in:"));
   movieFileFilterGroupLayout->addWidget( moviePlayerLabel, 1, 1);
      
   moviePlayer = new QComboBox(movieFileFilterBox);
   movieFileFilterGroupLayout->addWidget( moviePlayer, 1, 2);
   
   layout->addWidget(movieFileFilterBox);
   
   // --------------------------------------------------------

   QHGroupBox *rawFileFilterBox = new QHGroupBox(i18n("Raw files"), parent);

   QLabel *rawFileFilterLabel = new QLabel(rawFileFilterBox);
   rawFileFilterLabel->setText(i18n("Show only Raw files with extensions:"));

   rawFileFilterEdit = new QLineEdit(rawFileFilterBox);

   layout->addWidget(rawFileFilterBox);
   
   // --------------------------------------------------------

   layout->addStretch();

   readSettings();
}

SetupMime::~SetupMime()
{
}

void SetupMime::applySettings()
{
    AlbumSettings* settings = AlbumSettings::instance();
    
    if (!settings) return;

    settings->setImageFileFilter(imageFileFilterEdit->text());
    settings->setMovieFileFilter(movieFileFilterEdit->text());
    settings->setRawFileFilter(rawFileFilterEdit->text());
    
    settings->saveSettings();
}

void SetupMime::readSettings()
{
    AlbumSettings* settings = AlbumSettings::instance();
    
    if (!settings) return;

    imageFileFilterEdit->setText(settings->getImageFileFilter());
    movieFileFilterEdit->setText(settings->getMovieFileFilter());
    rawFileFilterEdit->setText(settings->getRawFileFilter());    
}


#include "setupmime.moc"
