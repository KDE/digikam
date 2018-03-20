/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-05-03
 * Description : mime types setup tab
 *
 * Copyright (C) 2004-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "setupmime.h"

// Qt includes

#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWhatsThis>
#include <QApplication>
#include <QStyle>
#include <QLineEdit>
#include <QIcon>
#include <QMessageBox>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "applicationsettings.h"
#include "coredb.h"
#include "coredbaccess.h"
#include "dlayoutbox.h"
#include "scancontroller.h"
#include "setuputils.h"

namespace Digikam
{

class SetupMime::Private
{
public:

    Private() :
        imageFileFilterLabel(0),
        movieFileFilterLabel(0),
        audioFileFilterLabel(0),
        imageFileFilterEdit(0),
        movieFileFilterEdit(0),
        audioFileFilterEdit(0)
    {
    }

    QLabel*    imageFileFilterLabel;
    QLabel*    movieFileFilterLabel;
    QLabel*    audioFileFilterLabel;

    QLineEdit* imageFileFilterEdit;
    QLineEdit* movieFileFilterEdit;
    QLineEdit* audioFileFilterEdit;
};

SetupMime::SetupMime(QWidget* const parent)
    : QScrollArea(parent),
      d(new Private)
{
    QWidget* const panel      = new QWidget(viewport());
    setWidget(panel);
    setWidgetResizable(true);

    const int spacing         = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);
    QVBoxLayout* const layout = new QVBoxLayout(panel);

    // --------------------------------------------------------

    QLabel* const explanationLabel = new QLabel;
    explanationLabel->setText(i18n("<p>Add new file types to show as album items. "
                                   "<p>digiKam attempts to support all of the image formats that digital cameras produce, "
                                   "while being able to handle a few other important video and audio formats.</p> "
                                   "<p>You can add to the already-appreciable list of formats that digiKam handles by "
                                   "adding the extension of the type you want to add. "
                                   "Multiple extensions need to be separated by a semicolon or space.</p>"
                                   "<p><b><u>Note:</u> changes done in this view will perform "
                                   "a database rescan in the background.</b></p>"));
    explanationLabel->setWordWrap(true);

    // --------------------------------------------------------

    QGroupBox* const imageFileFilterBox = new QGroupBox(i18n("Image Files"), panel);
    QGridLayout* const grid1            = new QGridLayout(imageFileFilterBox);
    QLabel* const logoLabel1            = new QLabel(imageFileFilterBox);
    logoLabel1->setPixmap(QIcon::fromTheme(QLatin1String("image-jpeg")).pixmap(48));

    d->imageFileFilterLabel = new QLabel(imageFileFilterBox);
    d->imageFileFilterLabel->setText(i18n("Additional &image file extensions (<a href='image'>Currently-supported types</a>):"));

    DHBox* const hbox1      = new DHBox(imageFileFilterBox);
    d->imageFileFilterEdit  = new QLineEdit(hbox1);
    d->imageFileFilterEdit->setWhatsThis(i18n("<p>Here you can add the extensions of image files (including RAW files) "
                                              "to be displayed in the Album view. Just put \"xyz abc\" "
                                              "to display files with the xyz and abc extensions in your Album view.</p>"
                                              "<p>You can also remove file formats that are shown by default "
                                              "by putting a minus sign in front of the extension: e.g. \"-gif\" would remove all GIF files "
                                              "from your Album view and any trace of them in your database. "
                                              "They would not be deleted, just not shown in digiKam.</p>"
                                              "<p><b>Warning:</b> Removing files from the database means losing "
                                              "all of their tags and ratings.</p>"));
    d->imageFileFilterEdit->setClearButtonEnabled(true);
    d->imageFileFilterEdit->setPlaceholderText(i18n("Enter additional image file extensions."));
    d->imageFileFilterLabel->setBuddy(d->imageFileFilterEdit);
    hbox1->setStretchFactor(d->imageFileFilterEdit, 10);

    grid1->addWidget(logoLabel1,              0, 0, 2, 1);
    grid1->addWidget(d->imageFileFilterLabel, 0, 1, 1, 1);
    grid1->addWidget(hbox1,                   1, 1, 1, 1);
    grid1->setColumnStretch(1, 10);
    grid1->setSpacing(spacing);

    // --------------------------------------------------------

    QGroupBox* const movieFileFilterBox = new QGroupBox(i18n("Video Files"), panel);
    QGridLayout* const grid2            = new QGridLayout(movieFileFilterBox);

    QLabel* const logoLabel2 = new QLabel(movieFileFilterBox);
    logoLabel2->setPixmap(QIcon::fromTheme(QLatin1String("video-x-matroska")).pixmap(48));

    d->movieFileFilterLabel  = new QLabel(movieFileFilterBox);
    d->movieFileFilterLabel->setText(i18n("Additional &video file extensions (<a href='video'>Currently-supported types</a>):"));

    DHBox* const hbox2       = new DHBox(movieFileFilterBox);
    d->movieFileFilterEdit   = new QLineEdit(hbox2);
    d->movieFileFilterEdit->setWhatsThis(i18n("<p>Here you can add extra extensions of video files "
                                              "to be displayed in your Album view. Just write \"xyz abc\" "
                                              "to support files with the *.xyz and *.abc extensions. "
                                              "Clicking on these files will "
                                              "play them in an embedded video player.</p>"
                                              "<p>You can also remove file formats that are supported by default "
                                              "by putting a minus sign in front of the extension: e.g. \"-avi\" would remove "
                                              "all AVI files from your Album view and any trace of them in your database. "
                                              "They would not be deleted, just not shown in digiKam.</p>"
                                              "<p><b>Warning:</b> Removing files from the database means losing "
                                              "all of their tags and ratings.</p>"));
    d->movieFileFilterEdit->setClearButtonEnabled(true);
    d->movieFileFilterEdit->setPlaceholderText(i18n("Enter additional video file extensions."));
    d->movieFileFilterLabel->setBuddy(d->movieFileFilterEdit);
    hbox2->setStretchFactor(d->movieFileFilterEdit, 10);

    grid2->addWidget(logoLabel2,                0, 0, 2, 1);
    grid2->addWidget(d->movieFileFilterLabel,   0, 1, 1, 1);
    grid2->addWidget(hbox2,                     1, 1, 1, 1);
    grid2->setColumnStretch(1, 10);
    grid2->setSpacing(spacing);

    // --------------------------------------------------------

    QGroupBox* const audioFileFilterBox = new QGroupBox(i18n("Audio Files"), panel);
    QGridLayout* const grid3            = new QGridLayout(audioFileFilterBox);

    QLabel* const logoLabel3 = new QLabel(audioFileFilterBox);
    logoLabel3->setPixmap(QIcon::fromTheme(QLatin1String("audio-x-mpeg")).pixmap(48));

    d->audioFileFilterLabel  = new QLabel(audioFileFilterBox);
    d->audioFileFilterLabel->setText(i18n("Additional &audio file extensions (<a href='audio'>Currently-supported types</a>):"));

    DHBox* const hbox3       = new DHBox(audioFileFilterBox);
    d->audioFileFilterEdit   = new QLineEdit(hbox3);
    d->audioFileFilterEdit->setWhatsThis(i18n("<p>Here you can add extra extensions of audio files "
                                              "to be displayed in your Album view. Just write \"xyz abc\" "
                                              "to support files with the *.xyz and *.abc extensions. "
                                              "Clicking on these files will "
                                              "play them in an embedded audio player.</p>"
                                              "<p>You can also remove file formats that are supported by default "
                                              "by putting a minus sign in front of the extension: e.g. \"-ogg\" would "
                                              "remove all OGG files from your Album view and any trace of them in your database. "
                                              "They would not be deleted, just not shown in digiKam.</p>"
                                              "<p><b>Warning:</b> Removing files from the database means losing "
                                              "all of their tags and ratings.</p>"));
    d->audioFileFilterEdit->setClearButtonEnabled(true);
    d->audioFileFilterEdit->setPlaceholderText(i18n("Enter additional audio file extensions."));
    d->audioFileFilterLabel->setBuddy(d->audioFileFilterEdit);
    hbox3->setStretchFactor(d->audioFileFilterEdit, 10);

    grid3->addWidget(logoLabel3,              0, 0, 2, 1);
    grid3->addWidget(d->audioFileFilterLabel, 0, 1, 1, 1);
    grid3->addWidget(hbox3,                   1, 1, 1, 1);
    grid3->setColumnStretch(1, 10);
    grid3->setSpacing(spacing);

    // --------------------------------------------------------

    layout->setContentsMargins(spacing, spacing, spacing, spacing);
    layout->setSpacing(spacing);
    layout->addWidget(explanationLabel);
    layout->addWidget(imageFileFilterBox);
    layout->addWidget(movieFileFilterBox);
    layout->addWidget(audioFileFilterBox);
    layout->addStretch();

    // --------------------------------------------------------

    connect(d->imageFileFilterLabel, SIGNAL(linkActivated(QString)),
            this, SLOT(slotShowCurrentImageSettings()));

    connect(d->movieFileFilterLabel, SIGNAL(linkActivated(QString)),
            this, SLOT(slotShowCurrentMovieSettings()));

    connect(d->audioFileFilterLabel, SIGNAL(linkActivated(QString)),
            this, SLOT(slotShowCurrentAudioSettings()));

    // --------------------------------------------------------

    readSettings();
}

SetupMime::~SetupMime()
{
    delete d;
}

void SetupMime::applySettings()
{
    // Display warning if user removes a core format
    QStringList coreImageFormats, removedImageFormats;
    coreImageFormats << QLatin1String("jpg") << QLatin1String("jpeg") << QLatin1String("jpe") // JPEG
                     << QLatin1String("tif") << QLatin1String("tiff")                         // TIFF
                     << QLatin1String("png");                                                 // PNG

    QString imageFilter = d->imageFileFilterEdit->text();

    foreach(const QString& format, coreImageFormats)
    {
        if (imageFilter.contains(QLatin1Char('-')     + format) ||
            imageFilter.contains(QLatin1String("-*.") + format))
        {
            removedImageFormats << format;
        }
    }

    if (!removedImageFormats.isEmpty())
    {
        int result = QMessageBox::warning(this, qApp->applicationName(),
                                          i18n("<p>You have chosen to remove the following image formats "
                                               "from the list of supported formats: <b>%1</b>.</p>"
                                               "<p>These are very common formats. If you have images in your collection "
                                               "with these formats, they will be removed from the database and you will "
                                               "lose all information about them, including rating and tags.</p>"
                                               "<p>Are you sure you want to apply your changes and lose the support for these formats?</p>",
                                               removedImageFormats.join(QLatin1String(" "))),
                                          QMessageBox::Yes | QMessageBox::No);

        if (result != QMessageBox::Yes)
        {
            return;
        }
    }

    QString imageFilterString;
    QString movieFilterString;
    QString audioFilterString;

    CoreDbAccess().db()->getUserFilterSettings(&imageFilterString, &movieFilterString, &audioFilterString);

    if (d->imageFileFilterEdit->text() != imageFilterString ||
        d->movieFileFilterEdit->text() != movieFilterString ||
        d->audioFileFilterEdit->text() != audioFilterString)
    {
        CoreDbAccess().db()->setUserFilterSettings(cleanUserFilterString(d->imageFileFilterEdit->text()),
                                                   cleanUserFilterString(d->movieFileFilterEdit->text()),
                                                   cleanUserFilterString(d->audioFileFilterEdit->text()));
        ScanController::instance()->completeCollectionScanInBackground(false);
    }
}

void SetupMime::readSettings()
{
    QString image;
    QString audio;
    QString video;

    CoreDbAccess().db()->getUserFilterSettings(&image, &video, &audio);

    d->imageFileFilterEdit->setText(image.replace(QLatin1Char(';'), QLatin1Char(' ')));
    d->movieFileFilterEdit->setText(video.replace(QLatin1Char(';'), QLatin1Char(' ')));
    d->audioFileFilterEdit->setText(audio.replace(QLatin1Char(';'), QLatin1Char(' ')));
}

void SetupMime::slotShowCurrentImageSettings()
{
    QStringList imageList;
    CoreDbAccess().db()->getFilterSettings(&imageList, 0, 0);
    QString text = i18n("<p>Files with these extensions will be recognized as images "
                        "and included into the database:<br/> <code>%1</code></p>",
                        imageList.join(QLatin1String(" ")));
    QWhatsThis::showText(d->imageFileFilterLabel->mapToGlobal(QPoint(0, 0)), text, d->imageFileFilterLabel);
}

void SetupMime::slotShowCurrentMovieSettings()
{
    QStringList movieList;
    CoreDbAccess().db()->getFilterSettings(0, &movieList, 0);
    QString text = i18n("<p>Files with these extensions will be recognized as video files "
                        "and included into the database:<br/> <code>%1</code></p>",
                        movieList.join(QLatin1String(" ")));
    QWhatsThis::showText(d->movieFileFilterLabel->mapToGlobal(QPoint(0, 0)), text, d->movieFileFilterLabel);
}

void SetupMime::slotShowCurrentAudioSettings()
{
    QStringList audioList;
    CoreDbAccess().db()->getFilterSettings(0, 0, &audioList);
    QString text = i18n("<p>Files with these extensions will be recognized as audio files "
                        "and included into the database:<br/> <code>%1</code></p>",
                        audioList.join(QLatin1String(" ")));
    QWhatsThis::showText(d->audioFileFilterLabel->mapToGlobal(QPoint(0, 0)), text, d->audioFileFilterLabel);
}

} // namespace Digikam
