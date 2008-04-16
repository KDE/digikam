/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-05-03
 * Description : mime types setup tab
 *
 * Copyright (C) 2004-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QToolButton>
#include <QGridLayout>
#include <QVBoxLayout>

// KDE includes.

#include <klocale.h>
#include <kdialog.h>
#include <kiconloader.h>
#include <kvbox.h>

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

SetupMime::SetupMime(QWidget* parent)
         : QWidget(parent)
{
    d = new SetupMimePriv;
    QVBoxLayout *layout = new QVBoxLayout(this);

    // --------------------------------------------------------
    
    QGroupBox *imageFileFilterBox = new QGroupBox(i18n("Image Files"), this);
    QGridLayout* grid1            = new QGridLayout(imageFileFilterBox);

    QLabel *logoLabel1 = new QLabel(imageFileFilterBox);
    logoLabel1->setPixmap(DesktopIcon("image-jpeg2000"));

    QLabel *imageFileFilterLabel = new QLabel(imageFileFilterBox);
    imageFileFilterLabel->setText(i18n("Show only &image files with extensions:"));
    
    KHBox *hbox1 = new KHBox(imageFileFilterBox);    
    d->imageFileFilterEdit = new QLineEdit(hbox1);
    d->imageFileFilterEdit->setWhatsThis( i18n("<p>Here you can set the extensions of the image files "
                                               "to be displayed in Albums (such as JPEG or TIFF); "
                                               "when these files are clicked on "
                                               "they will be opened with the digiKam Image Editor."));
    imageFileFilterLabel->setBuddy(d->imageFileFilterEdit);
    hbox1->setStretchFactor(d->imageFileFilterEdit, 10);

    d->revertImageFileFilterBtn = new QToolButton(hbox1);
    d->revertImageFileFilterBtn->setIcon(SmallIcon("view-refresh"));
    d->revertImageFileFilterBtn->setToolTip(i18n("Revert to default settings"));
 
    grid1->addWidget(logoLabel1, 0, 0, 2, 1);
    grid1->addWidget(imageFileFilterLabel, 0, 1, 1, 1);
    grid1->addWidget(hbox1, 1, 1, 1, 1);
    grid1->setColumnStretch(1, 10);
    grid1->setMargin(KDialog::spacingHint());
    grid1->setSpacing(KDialog::spacingHint());

    // --------------------------------------------------------
    
    QGroupBox *movieFileFilterBox = new QGroupBox(i18n("Movie Files"), this);
    QGridLayout* grid2            = new QGridLayout(movieFileFilterBox);

    QLabel *logoLabel2 = new QLabel(movieFileFilterBox);
    logoLabel2->setPixmap(DesktopIcon("video-mpeg"));

    QLabel *movieFileFilterLabel = new QLabel(movieFileFilterBox);
    movieFileFilterLabel->setText(i18n("Show only &movie files with extensions:"));
    
    KHBox *hbox2 = new KHBox(movieFileFilterBox);    
    d->movieFileFilterEdit = new QLineEdit(hbox2);
    d->movieFileFilterEdit->setWhatsThis( i18n("<p>Here you can set the extensions of movie files "
                                               "to be displayed in Albums (such as MPEG or AVI); "
                                               "when these files are clicked on they will "
                                               "be opened with the default KDE movie player."));
    movieFileFilterLabel->setBuddy(d->movieFileFilterEdit);
    hbox2->setStretchFactor(d->movieFileFilterEdit, 10);

    d->revertMovieFileFilterBtn = new QToolButton(hbox2);
    d->revertMovieFileFilterBtn->setIcon(SmallIcon("view-refresh"));
    d->revertMovieFileFilterBtn->setToolTip(i18n("Revert to default settings"));

    grid2->addWidget(logoLabel2, 0, 0, 2, 1);
    grid2->addWidget(movieFileFilterLabel, 0, 1, 1, 1);
    grid2->addWidget(hbox2, 1, 1, 1, 1);
    grid2->setColumnStretch(1, 10);
    grid2->setMargin(KDialog::spacingHint());
    grid2->setSpacing(KDialog::spacingHint());

    // --------------------------------------------------------
    
    QGroupBox *audioFileFilterBox = new QGroupBox(i18n("Audio Files"), this);
    QGridLayout* grid3            = new QGridLayout(audioFileFilterBox);

    QLabel *logoLabel3 = new QLabel(audioFileFilterBox);
    logoLabel3->setPixmap(DesktopIcon("audio-basic"));

    QLabel *audioFileFilterLabel = new QLabel(audioFileFilterBox);
    audioFileFilterLabel->setText(i18n("Show only &audio files with extensions:"));
    
    KHBox *hbox3 = new KHBox(audioFileFilterBox);  
    d->audioFileFilterEdit = new QLineEdit(hbox3);
    d->audioFileFilterEdit->setWhatsThis( i18n("<p>Here you can set the extensions of audio files "
                                               "to be displayed in Albums (such as MP3 or OGG); "
                                               "when these files are clicked on they will "
                                               "be opened with the default KDE audio player."));
    audioFileFilterLabel->setBuddy(d->audioFileFilterEdit);
    hbox3->setStretchFactor(d->audioFileFilterEdit, 10);

    d->revertAudioFileFilterBtn = new QToolButton(hbox3);
    d->revertAudioFileFilterBtn->setIcon(SmallIcon("view-refresh"));
    d->revertAudioFileFilterBtn->setToolTip(i18n("Revert to default settings"));

    grid3->addWidget(logoLabel3, 0, 0, 2, 1);
    grid3->addWidget(audioFileFilterLabel, 0, 1, 1, 1);
    grid3->addWidget(hbox3, 1, 1, 1, 1);
    grid3->setColumnStretch(1, 10);
    grid3->setMargin(KDialog::spacingHint());
    grid3->setSpacing(KDialog::spacingHint());

    // --------------------------------------------------------
    
    QGroupBox *rawFileFilterBox = new QGroupBox(i18n("RAW Files"), this);
    QGridLayout* grid4          = new QGridLayout(rawFileFilterBox);

    QLabel *logoLabel4 = new QLabel(rawFileFilterBox);
    logoLabel4->setPixmap(DesktopIcon("kdcraw"));

    QLabel *rawFileFilterLabel = new QLabel(rawFileFilterBox);
    rawFileFilterLabel->setText(i18n("Show only &RAW files with extensions:"));
    
    KHBox *hbox4 = new KHBox(rawFileFilterBox);  
    d->rawFileFilterEdit = new QLineEdit(hbox4);
    d->rawFileFilterEdit->setWhatsThis( i18n("<p>Here you can set the extensions of the RAW image files "
                                             "to be displayed in Albums (such as CRW, for Canon cameras, "
                                             "or NEF, for Nikon cameras)."));
    rawFileFilterLabel->setBuddy(d->rawFileFilterEdit);
    hbox4->setStretchFactor(d->rawFileFilterEdit, 10);

    d->revertRawFileFilterBtn = new QToolButton(hbox4);
    d->revertRawFileFilterBtn->setIcon(SmallIcon("view-refresh"));
    d->revertRawFileFilterBtn->setToolTip(i18n("Revert to default settings"));

    grid4->addWidget(logoLabel4, 0, 0, 2, 1);
    grid4->addWidget(rawFileFilterLabel, 0, 1, 1, 1);
    grid4->addWidget(hbox4, 1, 1, 1, 1);
    grid4->setColumnStretch(1, 10);
    grid4->setMargin(KDialog::spacingHint());
    grid4->setSpacing(KDialog::spacingHint());

    // --------------------------------------------------------

    layout->setMargin(0);
    layout->setSpacing(KDialog::spacingHint());
    layout->addWidget(imageFileFilterBox);
    layout->addWidget(movieFileFilterBox);
    layout->addWidget(audioFileFilterBox);
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
