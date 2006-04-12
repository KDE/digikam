/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2003-05-03
 * Description : mime types setup tab
 *
 * Copyright 2004-2006 by Gilles Caulier
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

namespace Digikam
{

class SetupMimePriv
{
public:

    SetupMimePriv()
    {
        imageFileFilterEdit = 0;
        movieFileFilterEdit = 0;
        audioFileFilterEdit = 0;
        rawFileFilterEdit   = 0;
    }

    QLineEdit *imageFileFilterEdit;
    QLineEdit *movieFileFilterEdit;
    QLineEdit *audioFileFilterEdit;
    QLineEdit *rawFileFilterEdit;
};

SetupMime::SetupMime(QWidget* parent )
         : QWidget(parent)
{
    d = new SetupMimePriv;
    QVBoxLayout *layout = new QVBoxLayout( parent, 0, KDialog::spacingHint() );
    
    // --------------------------------------------------------
    
    QGroupBox *imageFileFilterBox = new QGroupBox(1, Qt::Horizontal, i18n("Image Files"), parent);
    
    QLabel *imageFileFilterLabel = new QLabel(imageFileFilterBox);
    imageFileFilterLabel->setText(i18n("Show only &image files with extensions:"));
    
    d->imageFileFilterEdit = new QLineEdit(imageFileFilterBox);
    QWhatsThis::add( d->imageFileFilterEdit, i18n("<p>Here you can set the extensions of image files "
                                                    "to be displayed in Albums (such as JPEG or TIFF); "
                                                    "when these files are double-clicked on "
                                                    "they will be opened with the digiKam Image Editor."));
    imageFileFilterLabel->setBuddy(d->imageFileFilterEdit);
    layout->addWidget(imageFileFilterBox);
    
    // --------------------------------------------------------
    
    QGroupBox *movieFileFilterBox = new QGroupBox(1, Qt::Horizontal, i18n("Movie Files"), parent);
    
    QLabel *movieFileFilterLabel = new QLabel(movieFileFilterBox);
    movieFileFilterLabel->setText(i18n("Show only &movie files with extensions:"));
    
    d->movieFileFilterEdit = new QLineEdit(movieFileFilterBox);
    QWhatsThis::add( d->movieFileFilterEdit, i18n("<p>Here you can set the extensions of movie files "
                                                    "to be displayed in Albums (such as MPEG or AVI); "
                                                    "when these files are double-clicked on they will "
                                                    "be opened with the default KDE movie player."));
    movieFileFilterLabel->setBuddy(d->movieFileFilterEdit);
    layout->addWidget(movieFileFilterBox);
    
    // --------------------------------------------------------
    
    QGroupBox *audioFileFilterBox = new QGroupBox(1, Qt::Horizontal, i18n("Audio Files"), parent);
    
    QLabel *audioFileFilterLabel = new QLabel(audioFileFilterBox);
    audioFileFilterLabel->setText(i18n("Show only &audio files with extensions:"));
    
    d->audioFileFilterEdit = new QLineEdit(audioFileFilterBox);
    QWhatsThis::add( d->audioFileFilterEdit, i18n("<p>Here you can set the extensions of audio files "
                                                    "to be displayed in Albums (such as MP3 or OGG); "
                                                    "when these files are double-clicked on they will "
                                                    "be opened with the default KDE audio player."));
    
    audioFileFilterLabel->setBuddy(d->audioFileFilterEdit);
    
    layout->addWidget(audioFileFilterBox);
    
    // --------------------------------------------------------
    
    QGroupBox *rawFileFilterBox = new QGroupBox(1, Qt::Horizontal, i18n("Raw Files"), parent);
    
    QLabel *rawFileFilterLabel = new QLabel(rawFileFilterBox);
    rawFileFilterLabel->setText(i18n("Show only &raw files with extensions:"));
    
    d->rawFileFilterEdit = new QLineEdit(rawFileFilterBox);
    QWhatsThis::add( d->rawFileFilterEdit, i18n("<p>Here you can set the extensions of RAW image files "
                                                "to be displayed in Albums (such as CRW, for Canon cameras, "
                                                "or NEF, for Nikon cameras)."));
    rawFileFilterLabel->setBuddy(d->rawFileFilterEdit);
    layout->addWidget(rawFileFilterBox);
    
    // --------------------------------------------------------
    
    layout->addStretch();
    
    readSettings();
}

SetupMime::~SetupMime()
{
    delete d;
}

void SetupMime::applySettings()
{
    AlbumSettings* settings = AlbumSettings::instance();

    if (!settings) return;

    settings->setImageFileFilter(d->imageFileFilterEdit->text());
    settings->setMovieFileFilter(d->movieFileFilterEdit->text());
    settings->setAudioFileFilter(d->audioFileFilterEdit->text());
    settings->setRawFileFilter(d->rawFileFilterEdit->text());

    settings->saveSettings();
}

void SetupMime::readSettings()
{
    AlbumSettings* settings = AlbumSettings::instance();

    if (!settings) return;

    d->imageFileFilterEdit->setText(settings->getImageFileFilter());
    d->movieFileFilterEdit->setText(settings->getMovieFileFilter());
    d->audioFileFilterEdit->setText(settings->getAudioFileFilter());
    d->rawFileFilterEdit->setText(settings->getRawFileFilter());
}

}  // namespace Digikam

#include "setupmime.moc"
