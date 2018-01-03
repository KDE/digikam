/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-05-31
 * Description : Figure out camera clock delta from a clock picture.
 *
 * Copyright (C) 2009      by Pieter Edelman <p dot edelman at gmx dot net>
 * Copyright (C) 2011-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2014      by Michael G. Hansen <mike at mghansen dot de>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "clockphotodialog.h"

// Qt includes

#include <QDialogButtonBox>
#include <QStandardPaths>
#include <QDateTimeEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QWindow>
#include <QLabel>
#include <QIcon>
#include <QSize>
#include <QDir>

// KDE includes

#include <ksharedconfig.h>
#include <klocalizedstring.h>

// Local includes

#include "albummanager.h"
#include "dmetadata.h"
#include "imagedialog.h"
#include "timeadjustcontainer.h"
#include "graphicsdimgview.h"
#include "dimgpreviewitem.h"
#include "collectionmanager.h"
#include "dxmlguiwindow.h"

namespace Digikam
{

class ClockPhotoDialog::Private
{

public:

    Private()
    {
        buttons        = 0;
        calendar       = 0;
        dtLabel        = 0;
        previewManager = 0;
    }

    DeltaTime         deltaValues;

    QDialogButtonBox* buttons;
    QDateTimeEdit*    calendar;
    QDateTime         photoDateTime;
    QLabel*           dtLabel;

    GraphicsDImgView* previewManager;
};

ClockPhotoDialog::ClockPhotoDialog(QWidget* const parent, const QUrl& defaultUrl)
    : QDialog(parent),
      d(new Private)
{
    // This dialog should be modal with three buttons: Ok, Cancel, and load
    // photo. For this third button, the User1 button from KDialog is used.
    // The Ok button is only enable when a photo is loaded.

    setWindowTitle(i18n("Determine Time Difference With Clock Photo"));

    d->buttons = new QDialogButtonBox(QDialogButtonBox::Apply | QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    d->buttons->button(QDialogButtonBox::Cancel)->setDefault(true);
    d->buttons->button(QDialogButtonBox::Ok)->setEnabled(false);
    setMinimumWidth(500);
    setMinimumHeight(500);
    d->buttons->button(QDialogButtonBox::Apply)->setText(i18n("Load different photo"));
    d->buttons->button(QDialogButtonBox::Apply)->setIcon(QIcon::fromTheme(QLatin1String("document-open")));

    QWidget* const mainWidget = new QWidget(this);
    QVBoxLayout* const vBox   = new QVBoxLayout(mainWidget);

    // Some explanation.
    QLabel* const explanationLabel = new QLabel(i18n("If you have a photo in your set with a clock or "
                                                     "another external time source on it, you can load "
                                                     "it here and set the indicator to the (date and) "
                                                     "time displayed. The difference of your internal "
                                                     "camera clock will be determined from this "
                                                     "setting."));
    explanationLabel->setWordWrap(true);
    vBox->addWidget(explanationLabel);

    d->previewManager = new GraphicsDImgView(this);
    d->previewManager->setItem(new DImgPreviewItem());
    d->previewManager->setMinimumSize(QSize(200, 200));
    vBox->addWidget(d->previewManager);

    // The date and time entry widget allows the user to enter the date and time
    // displayed in the image. The format is explicitly set, otherwise seconds
    // might not get displayed.
    d->dtLabel  = new QLabel();
    d->calendar = new QDateTimeEdit();
    d->calendar->setDisplayFormat(QLatin1String("d MMMM yyyy, hh:mm:ss"));
    d->calendar->setCalendarPopup(true);
    d->calendar->setEnabled(false);
    QHBoxLayout* const hBox2 = new QHBoxLayout(mainWidget);
    hBox2->addStretch();
    hBox2->addWidget(d->dtLabel);
    hBox2->addWidget(d->calendar);
    vBox->addLayout(hBox2);

    vBox->addWidget(d->buttons);
    setLayout(vBox);

    // Setup the signals and slots.
    connect(d->buttons->button(QDialogButtonBox::Ok), SIGNAL(clicked()),
            this, SLOT(slotOk()));

    connect(d->buttons->button(QDialogButtonBox::Cancel), SIGNAL(clicked()),
            this, SLOT(slotCancel()));

    connect(d->buttons->button(QDialogButtonBox::Apply), SIGNAL(clicked()),
            this, SLOT(slotLoadPhoto()));

    // Show the window.
    loadSettings();
    show();

    if (defaultUrl.isValid())
    {
        setImage(defaultUrl);
    }
    else
    {
        // No default url was given.
        // Upon initialization, present the user with a photo loading dialog. This
        // is done before the main dialog is drawn.
        slotLoadPhoto();
    }
}

ClockPhotoDialog::~ClockPhotoDialog()
{
    delete d;
}

DeltaTime ClockPhotoDialog::deltaValues() const
{
    return d->deltaValues;
}

bool ClockPhotoDialog::setImage(const QUrl& imageFile)
{
    bool success = false;
    QString msg = i18n("<font color=\"red\"><b>Could not load<br>"
                       "image %1.</b></font>",
                       imageFile.fileName());

    d->previewManager->previewItem()->setPath(imageFile.toLocalFile(), true);

    // Try to read the datetime data.
    DMetadata meta;

    if (meta.load(imageFile.toLocalFile()))
    {
        d->photoDateTime = meta.getImageDateTime();

        if (d->photoDateTime.isValid())
        {
            msg = i18n("The clock date and time:");

            // Set the datetime widget to the photo datetime.
            d->calendar->setDateTime(d->photoDateTime);
            d->calendar->setEnabled(true);
            success = true;
        }
    }

    d->dtLabel->setText(msg);

    // Disable all the GUI elements if loading failed.
    d->calendar->setEnabled(success);

    // enable the ok button if loading succeeded
    d->buttons->button(QDialogButtonBox::Ok)->setEnabled(success);

    return success;
}

void ClockPhotoDialog::loadSettings()
{
    KConfigGroup group = KSharedConfig::openConfig()->group(QLatin1String("Clock Photo Dialog"));

    winId();
    DXmlGuiWindow::restoreWindowSize(windowHandle(), group);
    resize(windowHandle()->size());
}

void ClockPhotoDialog::saveSettings()
{
    KConfigGroup group = KSharedConfig::openConfig()->group(QLatin1String("Clock Photo Dialog"));
    DXmlGuiWindow::saveWindowSize(windowHandle(), group);
}

void ClockPhotoDialog::slotLoadPhoto()
{
    QString place      = QDir::homePath();
    QStringList pics   = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
    Album* const album = AlbumManager::instance()->currentAlbums().first();

    if (album->type() == Album::PHYSICAL)
    {
        PAlbum* const p = dynamic_cast<PAlbum*>(album);

        if (p)
        {
            place = p->folderPath();
        }
    }
    else
    {
        QStringList cols = CollectionManager::instance()->allAvailableAlbumRootPaths();

        if (!cols.isEmpty())
            place = cols.first();
    }

    ImageDialog dlg(this, QUrl(), true, i18n("Select Image to Extract Clock Photo"));

    if (!dlg.url().isEmpty())
    {
        // If the user selected a proper photo, try to load it.
        setImage(dlg.url());
    }
}

void ClockPhotoDialog::slotOk()
{
    // Called when the ok button is pressed. Calculate the time difference
    // between the photo and the user set datetime information, and store it in
    // the public variables.

    // Determine the number of seconds between the dates.
    int delta = d->photoDateTime.secsTo(d->calendar->dateTime());

    // If the photo datetime is newer than the user datetime, it results in
    // subtraction.
    if (delta < 0)
    {
        d->deltaValues.deltaNegative = true;
        delta                       *= -1;
    }
    else
    {
        d->deltaValues.deltaNegative = false;
    }

    // Calculate the number of days, hours, minutes and seconds.
    d->deltaValues.deltaDays    = delta / 86400;
    delta                       = delta % 86400;
    d->deltaValues.deltaHours   = delta / 3600;
    delta                       = delta % 3600;
    d->deltaValues.deltaMinutes = delta / 60;
    delta                       = delta % 60;
    d->deltaValues.deltaSeconds = delta;

    // Accept the dialog.
    saveSettings();
    accept();
}

/** If the cancel button is clicked, reject the dialog.
 */
void ClockPhotoDialog::slotCancel()
{
    saveSettings();
    reject();
}

}  // namespace Digikam
