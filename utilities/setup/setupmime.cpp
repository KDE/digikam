/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2003-05-03
 * Description : mime types setup tab
 *
 * Copyright 2004 by Gilles Caulier
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
   QVBoxLayout *layout = new QVBoxLayout( parent );

   // --------------------------------------------------------

   QGroupBox *imageFileFilterBox = new QGroupBox(1, Qt::Horizontal, i18n("Image Files"), parent);

   QLabel *imageFileFilterLabel = new QLabel(imageFileFilterBox);
   imageFileFilterLabel->setText(i18n("Show only &image files with extensions:"));

   m_imageFileFilterEdit = new QLineEdit(imageFileFilterBox);
   QWhatsThis::add( m_imageFileFilterEdit, i18n("<p>Here you can set the extensions of image files "
                                                "to be displayed in Albums (such as JPEG or TIFF); "
                                                "when these files are double-clicked on "
                                                "they will be opened with the digiKam Image Editor."));
   imageFileFilterLabel->setBuddy(m_imageFileFilterEdit);
   layout->addWidget(imageFileFilterBox);

   // --------------------------------------------------------

   QGroupBox *movieFileFilterBox = new QGroupBox(1, Qt::Horizontal, i18n("Movie Files"), parent);

   QLabel *movieFileFilterLabel = new QLabel(movieFileFilterBox);
   movieFileFilterLabel->setText(i18n("Show only &movie files with extensions:"));

   m_movieFileFilterEdit = new QLineEdit(movieFileFilterBox);
   QWhatsThis::add( m_movieFileFilterEdit, i18n("<p>Here you can set the extensions of movie files "
                                                "to be displayed in Albums (such as MPEG or AVI); "
                                                "when these files are double-clicked on they will "
                                                "be opened with the default KDE movie player."));
   movieFileFilterLabel->setBuddy(m_movieFileFilterEdit);
   layout->addWidget(movieFileFilterBox);

   // --------------------------------------------------------

   QGroupBox *audioFileFilterBox = new QGroupBox(1, Qt::Horizontal, i18n("Audio Files"), parent);

   QLabel *audioFileFilterLabel = new QLabel(audioFileFilterBox);
   audioFileFilterLabel->setText(i18n("Show only &audio files with extensions:"));

   m_audioFileFilterEdit = new QLineEdit(audioFileFilterBox);
   QWhatsThis::add( m_audioFileFilterEdit, i18n("<p>Here you can set the extensions of audio files "
                                                "to be displayed in Albums (such as MP3 or OGG); "
                                                "when these files are double-clicked on they will "
                                                "be opened with the default KDE audio player."));

   audioFileFilterLabel->setBuddy(m_audioFileFilterEdit);

   layout->addWidget(audioFileFilterBox);

   // --------------------------------------------------------

   QGroupBox *rawFileFilterBox = new QGroupBox(1, Qt::Horizontal, i18n("Raw Files"), parent);

   QLabel *rawFileFilterLabel = new QLabel(rawFileFilterBox);
   rawFileFilterLabel->setText(i18n("Show only &raw files with extensions:"));

   m_rawFileFilterEdit = new QLineEdit(rawFileFilterBox);
   QWhatsThis::add( m_rawFileFilterEdit, i18n("<p>Here you can set the extensions of RAW image files "
                                              "to be displayed in Albums (such as CRW, for Canon cameras, "
                                              "or NEF, for Nikon cameras)."));
   rawFileFilterLabel->setBuddy(m_rawFileFilterEdit);
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
