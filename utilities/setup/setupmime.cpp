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
#include <kmessagebox.h>
#include <kvbox.h>

// Local includes.

#include "albumsettings.h"
#include "databaseaccess.h"
#include "albumdb.h"
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

    QLabel *explanationLabel = new QLabel;
    explanationLabel->setText(i18n("<qt><p>Digikam supports most common image formats - "
                                   "most notably among others JPEG, PNG, TIFF, JPEG2000, "
                                   "and a large number of RAW image formats - "
                                   "as well as common video and audio formats.</p> "
                                   "<p>If you have special needs, you can adjust here the list of supported "
                                   "image formats. Add your file extensions to the lists of either image, "
                                   "audio or video files.</p></qt>"));
    explanationLabel->setWordWrap(true);

    // --------------------------------------------------------
    
    QGroupBox *imageFileFilterBox = new QGroupBox(i18n("Image Files"), this);
    QGridLayout* grid1            = new QGridLayout(imageFileFilterBox);

    QLabel *logoLabel1 = new QLabel(imageFileFilterBox);
    logoLabel1->setPixmap(DesktopIcon("image-jpeg2000"));

    QLabel *imageFileFilterLabel = new QLabel(imageFileFilterBox);
    imageFileFilterLabel->setText(i18n("Add &image file extensions:"));
    
    KHBox *hbox1 = new KHBox(imageFileFilterBox);    
    d->imageFileFilterEdit = new QLineEdit(hbox1);
    d->imageFileFilterEdit->setWhatsThis( i18n("<p>Here you can add extra extensions of image files (including RAW files) "
                                               "to be displayed in Albums. Just write \"xyz abc\" "
                                               "to support files with the *.xyz and *.abc extensions. "
                                               "You can as well remove file formats that are supported by default "
                                               "by prepending a minus sign: \"-gif\" will remove all GIF files "
                                               "from the database.</p>"
                                               "<p><b>Attention:</b> Removing files from the database means losing "
                                               "all their tags and ratings!<p>"));
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
    movieFileFilterLabel->setText(i18n("Add &movie file extensions:"));

    KHBox *hbox2 = new KHBox(movieFileFilterBox);    
    d->movieFileFilterEdit = new QLineEdit(hbox2);
    d->movieFileFilterEdit->setWhatsThis( i18n("<p>Here you can add extra extensions of video files "
                                               "to be displayed in Albums. Just write \"xyz abc\" "
                                               "to support files with the *.xyz and *.abc extensions. "
                                               "Clicking on these files will "
                                               "play them in an embedded KDE movie player. "
                                               "You can as well remove file formats that are supported by default "
                                               "by prepending a minus sign: \"-avi\" will remove all AVI files "
                                               "from the database.</p>"));
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
    audioFileFilterLabel->setText(i18n("Add &audio file extensions:"));

    KHBox *hbox3 = new KHBox(audioFileFilterBox);  
    d->audioFileFilterEdit = new QLineEdit(hbox3);
    d->audioFileFilterEdit->setWhatsThis( i18n("<p>Here you can add extra extensions of video files "
                                               "to be displayed in Albums. Just write \"xyz abc\" "
                                               "to support files with the *.xyz and *.abc extensions. "
                                               "Clicking on these files will "
                                               "play them with an embedded KDE audio player. "
                                               "You can as well remove file formats that are supported by default "
                                               "by prepending a minus sign: \"-mp3\" will for example remove all MP3 files "
                                               "from the database.</p>"));
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

    /*
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
    */

    // --------------------------------------------------------

    layout->setMargin(0);
    layout->setSpacing(KDialog::spacingHint());
    layout->addWidget(explanationLabel);
    layout->addWidget(imageFileFilterBox);
    layout->addWidget(movieFileFilterBox);
    layout->addWidget(audioFileFilterBox);
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
    //TODO: fix crash
    return;
    // Display warning if user removes a core format
    QStringList coreImageFormats, removedImageFormats;
    coreImageFormats << "jpg" << "jpeg" << "jpe"               // JPEG
                     << "tif" << "tiff"                        // TIFF
                     << "png";                                 // PNG

    QString imageFilter = d->imageFileFilterEdit->text();
    foreach(QString format, coreImageFormats)
    {
        if (imageFilter.contains('-' + format)
            || imageFilter.contains("-*." + format))
            removedImageFormats << format;
    }

    if (!removedImageFormats.isEmpty())
    {
        int result = KMessageBox::warningYesNo(this,
                                           i18n("You have chosen to remove the following image formats "
                                                "from the list of supported formats: %1.\n"
                                                "These are very common formats. If you have images in your collection "
                                                "with these formats, they will be removed from the database and you will "
                                                "lose all information about them, including rating and tags. "
                                                "Are you sure you want to apply your changes about the supported image formats?",
                                                removedImageFormats.join(" "))
                                              );
        if (result != KMessageBox::Yes)
            return;
    }

    DatabaseAccess().db()->setUserFilterSettings(d->imageFileFilterEdit->text(),
                                                 d->movieFileFilterEdit->text(),
                                                 d->audioFileFilterEdit->text());
}

void SetupMime::readSettings()
{
    QString image, audio, video;
    DatabaseAccess().db()->getUserFilterSettings(&image, &video, &audio);

    d->imageFileFilterEdit->setText(image);
    d->movieFileFilterEdit->setText(audio);
    d->audioFileFilterEdit->setText(video);
}

void SetupMime::slotRevertImageFileFilter()
{
    d->imageFileFilterEdit->setText(QString());
}

void SetupMime::slotRevertMovieFileFilter()
{
    d->movieFileFilterEdit->setText(QString());
}

void SetupMime::slotRevertAudioFileFilter()
{
    d->audioFileFilterEdit->setText(QString());
}

void SetupMime::slotRevertRawFileFilter()
{
    d->rawFileFilterEdit->setText(QString());
}

}  // namespace Digikam
