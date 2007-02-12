/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date   : 2003-05-03
 * Description : mime types setup tab
 *
 * Copyright 2004-2007 by Gilles Caulier
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
#include <qhbox.h>
#include <qhgroupbox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qwhatsthis.h>
#include <qtoolbutton.h>
#include <qtooltip.h>

// KDE includes.

#include <klocale.h>
#include <kdialog.h>
#include <klineeditdlg.h>
#include <kiconloader.h>

// Local includes.

#include "albumsettings.h"
#include "setupmime.h"
#include "setupmime.moc"

namespace Digikam
{

class SetupMimePriv
{
public:

    SetupMimePriv()
    {
        imageFileFilterEdit      = 0;
        movieFileFilterEdit      = 0;
        audioFileFilterEdit      = 0;
        rawFileFilterEdit        = 0;
        revertImageFileFilterBtn = 0;
        revertMovieFileFilterBtn = 0;
        revertAudioFileFilterBtn = 0;
        revertRawFileFilterBtn   = 0;
    }

    QToolButton *revertImageFileFilterBtn;
    QToolButton *revertMovieFileFilterBtn;
    QToolButton *revertAudioFileFilterBtn;
    QToolButton *revertRawFileFilterBtn;

    QLineEdit   *imageFileFilterEdit;
    QLineEdit   *movieFileFilterEdit;
    QLineEdit   *audioFileFilterEdit;
    QLineEdit   *rawFileFilterEdit;
};

SetupMime::SetupMime(QWidget* parent )
         : QWidget(parent)
{
    d = new SetupMimePriv;
    QVBoxLayout *layout = new QVBoxLayout(parent, 0, KDialog::spacingHint());
    
    // --------------------------------------------------------
    
    QGroupBox *imageFileFilterBox = new QGroupBox(1, Qt::Horizontal, i18n("Image Files"), parent);
    
    QLabel *imageFileFilterLabel = new QLabel(imageFileFilterBox);
    imageFileFilterLabel->setText(i18n("Show only &image files with extensions:"));
    
    QHBox *hbox1 = new QHBox(imageFileFilterBox);    
    d->imageFileFilterEdit = new QLineEdit(hbox1);
    QWhatsThis::add( d->imageFileFilterEdit, i18n("<p>Here you can set the extensions of image files "
                                                  "to be displayed in Albums (such as JPEG or TIFF); "
                                                  "when these files are clicked on "
                                                  "they will be opened with the digiKam Image Editor."));
    imageFileFilterLabel->setBuddy(d->imageFileFilterEdit);
    hbox1->setStretchFactor(d->imageFileFilterEdit, 10);

    d->revertImageFileFilterBtn = new QToolButton(hbox1);
    d->revertImageFileFilterBtn->setIconSet(SmallIcon("reload_page"));
    QToolTip::add(d->revertImageFileFilterBtn, i18n("Revert to default settings"));
 
    layout->addWidget(imageFileFilterBox);
    
    // --------------------------------------------------------
    
    QGroupBox *movieFileFilterBox = new QGroupBox(1, Qt::Horizontal, i18n("Movie Files"), parent);
    
    QLabel *movieFileFilterLabel = new QLabel(movieFileFilterBox);
    movieFileFilterLabel->setText(i18n("Show only &movie files with extensions:"));
    
    QHBox *hbox2 = new QHBox(movieFileFilterBox);    
    d->movieFileFilterEdit = new QLineEdit(hbox2);
    QWhatsThis::add( d->movieFileFilterEdit, i18n("<p>Here you can set the extensions of movie files "
                                                  "to be displayed in Albums (such as MPEG or AVI); "
                                                  "when these files are clicked on they will "
                                                  "be opened with the default KDE movie player."));
    movieFileFilterLabel->setBuddy(d->movieFileFilterEdit);
    hbox2->setStretchFactor(d->movieFileFilterEdit, 10);

    d->revertMovieFileFilterBtn = new QToolButton(hbox2);
    d->revertMovieFileFilterBtn->setIconSet(SmallIcon("reload_page"));
    QToolTip::add(d->revertMovieFileFilterBtn, i18n("Revert to default settings"));

    layout->addWidget(movieFileFilterBox);
    
    // --------------------------------------------------------
    
    QGroupBox *audioFileFilterBox = new QGroupBox(1, Qt::Horizontal, i18n("Audio Files"), parent);
    
    QLabel *audioFileFilterLabel = new QLabel(audioFileFilterBox);
    audioFileFilterLabel->setText(i18n("Show only &audio files with extensions:"));
    
    QHBox *hbox3 = new QHBox(audioFileFilterBox);  
    d->audioFileFilterEdit = new QLineEdit(hbox3);
    QWhatsThis::add( d->audioFileFilterEdit, i18n("<p>Here you can set the extensions of audio files "
                                                  "to be displayed in Albums (such as MP3 or OGG); "
                                                  "when these files are clicked on they will "
                                                  "be opened with the default KDE audio player."));
    audioFileFilterLabel->setBuddy(d->audioFileFilterEdit);
    hbox3->setStretchFactor(d->audioFileFilterEdit, 10);

    d->revertAudioFileFilterBtn = new QToolButton(hbox3);
    d->revertAudioFileFilterBtn->setIconSet(SmallIcon("reload_page"));
    QToolTip::add(d->revertAudioFileFilterBtn, i18n("Revert to default settings"));

    layout->addWidget(audioFileFilterBox);
    
    // --------------------------------------------------------
    
    QGroupBox *rawFileFilterBox = new QGroupBox(1, Qt::Horizontal, i18n("Raw Files"), parent);
    
    QLabel *rawFileFilterLabel = new QLabel(rawFileFilterBox);
    rawFileFilterLabel->setText(i18n("Show only &raw files with extensions:"));
    
    QHBox *hbox4 = new QHBox(rawFileFilterBox);  
    d->rawFileFilterEdit = new QLineEdit(hbox4);
    QWhatsThis::add( d->rawFileFilterEdit, i18n("<p>Here you can set the extensions of RAW image files "
                                                "to be displayed in Albums (such as CRW, for Canon cameras, "
                                                "or NEF, for Nikon cameras)."));
    rawFileFilterLabel->setBuddy(d->rawFileFilterEdit);
    hbox4->setStretchFactor(d->rawFileFilterEdit, 10);

    d->revertRawFileFilterBtn = new QToolButton(hbox4);
    d->revertRawFileFilterBtn->setIconSet(SmallIcon("reload_page"));
    QToolTip::add(d->revertRawFileFilterBtn, i18n("Revert to default settings"));

    layout->addWidget(rawFileFilterBox);
    layout->addStretch();
    
    // --------------------------------------------------------
    
    connect(d->revertImageFileFilterBtn, SIGNAL(clicked()),
            this, SLOT(slotRevertImageFileFilter()));

    connect(d->revertMovieFileFilterBtn, SIGNAL(clicked()),
            this, SLOT(slotRevertMovieFileFilter()));

    connect(d->revertAudioFileFilterBtn, SIGNAL(clicked()),
            this, SLOT(slotRevertAudioFileFilter()));

    connect(d->revertRawFileFilterBtn, SIGNAL(clicked()),
            this, SLOT(slotRevertRawFileFilter()));

    // --------------------------------------------------------
    
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

void SetupMime::slotRevertImageFileFilter()
{
    d->imageFileFilterEdit->setText(AlbumSettings::instance()->getDefaultImageFileFilter());
}

void SetupMime::slotRevertMovieFileFilter()
{
    d->movieFileFilterEdit->setText(AlbumSettings::instance()->getDefaultMovieFileFilter());
}

void SetupMime::slotRevertAudioFileFilter()
{
    d->audioFileFilterEdit->setText(AlbumSettings::instance()->getDefaultAudioFileFilter());
}

void SetupMime::slotRevertRawFileFilter()
{
    d->rawFileFilterEdit->setText(AlbumSettings::instance()->getDefaultRawFileFilter());
}

}  // namespace Digikam

