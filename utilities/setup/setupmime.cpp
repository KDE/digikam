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
#include <qwhatsthis.h>

// KDE includes.

#include <klocale.h>
#include <kdialog.h>
#include <klineeditdlg.h>

// Local includes.

#include "albumsettings.h"
#include "setupmime.h"


SetupMime::SetupMime(QWidget* parent )
         : QWidget(parent)
{
   QVBoxLayout *layout = new QVBoxLayout( parent, 10);
   layout->setSpacing( KDialog::spacingHint() );

   // --------------------------------------------------------

   QGroupBox *imageFileFilterBox = new QGroupBox(1, Qt::Horizontal, i18n("Image files"), parent);
   
   QLabel *imageFileFilterLabel = new QLabel(imageFileFilterBox);
   imageFileFilterLabel->setText(i18n("Show only image files with extensions:"));

   m_imageFileFilterEdit = new QLineEdit(imageFileFilterBox);
   QWhatsThis::add( m_imageFileFilterEdit, i18n("<p>You can set here the extension of image files "
                                                "who will displayed on Albums (like JPEG or TIFF)."));
   
   layout->addWidget(imageFileFilterBox);
   
   // --------------------------------------------------------
   
   QGroupBox *movieFileFilterBox = new QGroupBox(1, Qt::Horizontal, i18n("Movie files"), parent);
  
   QLabel *movieFileFilterLabel = new QLabel(movieFileFilterBox);
   movieFileFilterLabel->setText(i18n("Show only movie files with extensions:"));
   
   m_movieFileFilterEdit = new QLineEdit(movieFileFilterBox);
   QWhatsThis::add( m_movieFileFilterEdit, i18n("<p>You can set here the extension of the movie files "
                                                "who will displayed on Albums (like MPEG or AVI)."));

   layout->addWidget(movieFileFilterBox);

   // --------------------------------------------------------
   
   QGroupBox *audioFileFilterBox = new QGroupBox(1, Qt::Horizontal, i18n("Audio files"), parent);
  
   QLabel *audioFileFilterLabel = new QLabel(audioFileFilterBox);
   audioFileFilterLabel->setText(i18n("Show only audio files with extensions:"));
   
   m_audioFileFilterEdit = new QLineEdit(audioFileFilterBox);
   QWhatsThis::add( m_audioFileFilterEdit, i18n("<p>You can set here the extension of audio files "
                                                "who will displayed on Albums (like MP3 or OGG)."));
   
   layout->addWidget(audioFileFilterBox);
      
   // --------------------------------------------------------

   QGroupBox *rawFileFilterBox = new QGroupBox(1, Qt::Horizontal, i18n("Raw files"), parent);

   QLabel *rawFileFilterLabel = new QLabel(rawFileFilterBox);
   rawFileFilterLabel->setText(i18n("Show only Raw files with extensions:"));

   m_rawFileFilterEdit = new QLineEdit(rawFileFilterBox);
   QWhatsThis::add( m_rawFileFilterEdit, i18n("<p>You can set here the extension of RAW image files "
                                                "who will displayed on Albums (like CRW for Canon camera "
                                                "or NEF for Nikon camera)."));
                                                
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

    settings->setImageFileFilter(m_imageFileFilterEdit->text());
    settings->setMovieFileFilter(m_movieFileFilterEdit->text());
    settings->setAudioFileFilter(m_audioFileFilterEdit->text());
    settings->setRawFileFilter(m_rawFileFilterEdit->text());
    
    settings->saveSettings();
}

void SetupMime::readSettings()
{
    AlbumSettings* settings = AlbumSettings::instance();
    
    if (!settings) return;

    m_imageFileFilterEdit->setText(settings->getImageFileFilter());
    m_movieFileFilterEdit->setText(settings->getMovieFileFilter());
    m_audioFileFilterEdit->setText(settings->getAudioFileFilter());
    m_rawFileFilterEdit->setText(settings->getRawFileFilter());    
}


#include "setupmime.moc"
