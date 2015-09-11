/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.digikam.org
 *
 * Date        : 2007-09-09
 * Description : scanner dialog
 *
 * Copyright (C) 2007-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "scandialog.h"

// Qt includes

#include <QVBoxLayout>
#include <QDateTime>
#include <QPushButton>
#include <QPointer>
#include <QDir>
#include <QUrl>
#include <QMenu>
#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QImageWriter>

// KDE includes

#include <kconfig.h>
#include <klocalizedstring.h>
#include <kwindowconfig.h>

// LibKSane includes

#include <ksane.h>

// Local includes

#include "digikam_debug.h"
#include "saveimgthread.h"

namespace Digikam
{

class ScanDialog::Private
{
public:

    Private()
    {
        saneWidget = 0;
    }

    KSaneWidget* saneWidget;
};

ScanDialog::ScanDialog(KSaneWidget* const saneWdg, QWidget* const parent)
    : QDialog(parent),
      d(new Private)
{
    setWindowTitle(i18n("Scan Image"));
    setModal(false);
 
    d->saneWidget = saneWdg;
    d->saneWidget->show();

    QVBoxLayout* const vbx = new QVBoxLayout(this);
    vbx->addWidget(d->saneWidget);
    //vbx->addWidget(buttons);
    setLayout(vbx);

    // ------------------------------------------------------------------------

    readSettings();

    // ------------------------------------------------------------------------

    connect(d->saneWidget, SIGNAL(imageReady(QByteArray&,int,int,int,int)),
            this, SLOT(slotSaveImage(QByteArray&,int,int,int,int)));

    connect(this, &QDialog::finished,
            this, &ScanDialog::slotDialogFinished);
}

ScanDialog::~ScanDialog()
{
    delete d;
}

void ScanDialog::readSettings()
{
    KConfig config(QLatin1String("kipirc"));
    KConfigGroup group = config.group(QLatin1String("Scan Tool Dialog"));
    KWindowConfig::restoreWindowSize(windowHandle(), group);
}

void ScanDialog::saveSettings()
{
    KConfig config(QLatin1String("kipirc"));
    KConfigGroup group = config.group(QLatin1String("Scan Tool Dialog"));
    KWindowConfig::saveWindowSize(windowHandle(), group);
    config.sync();
}

void ScanDialog::closeEvent(QCloseEvent* e)
{
    if (!e)
    {
        return;
    }

    slotDialogFinished();
    e->accept();
}

void ScanDialog::slotDialogFinished()
{
    d->saneWidget->closeDevice();
    saveSettings();
}

void ScanDialog::slotSaveImage(QByteArray& ksane_data, int width, int height, int bytes_per_line, int ksaneformat)
{
    QStringList writableMimetypes;
    QList<QByteArray> supported = QImageWriter::supportedMimeTypes();

    foreach (QByteArray mimeType, supported)
    {
        writableMimetypes.append(QString::fromLatin1(mimeType));
    }

    // Put first class citizens at first place
    writableMimetypes.removeAll(QLatin1String("image/jpeg"));
    writableMimetypes.removeAll(QLatin1String("image/tiff"));
    writableMimetypes.removeAll(QLatin1String("image/png"));
    writableMimetypes.insert(0, QLatin1String("image/png"));
    writableMimetypes.insert(1, QLatin1String("image/jpeg"));
    writableMimetypes.insert(2, QLatin1String("image/tiff"));

    qCDebug(DIGIKAM_GENERAL_LOG) << "slotSaveImage: Offered mimetypes: " << writableMimetypes;

    QLatin1String defaultMimeType("image/png");
    QLatin1String defaultFileName("image.png");
    QString place = QDir::homePath();
/*
    Album* const album = AlbumManager::instance()->currentAlbums().first();

    if (album->type() == Album::PHYSICAL)
    {
        PAlbum* const p = dynamic_cast<PAlbum*>(album);

        if (p)
        {
            place = p->folderPath();
        }
    }
*/
    QPointer<QFileDialog> imageFileSaveDialog = new QFileDialog(0, i18n("New Image File Name"), place);
    imageFileSaveDialog->setAcceptMode(QFileDialog::AcceptSave);
    imageFileSaveDialog->setMimeTypeFilters(writableMimetypes);
    imageFileSaveDialog->selectMimeTypeFilter(defaultMimeType);
    imageFileSaveDialog->selectFile(defaultFileName);

    // Start dialog and check if canceled.
    if (imageFileSaveDialog->exec() != QDialog::Accepted)
    {
        delete imageFileSaveDialog;
        return;
    }

    QUrl newURL = imageFileSaveDialog->selectedUrls().at(0);
    QFileInfo fi(newURL.toLocalFile());

    // Parse name filter and extract file extension
    QString selectedFilterString = imageFileSaveDialog->selectedNameFilter();
    QLatin1String triggerString("*.");
    int triggerPos = selectedFilterString.lastIndexOf(triggerString);
    QByteArray format;

    if (triggerPos != -1)
    {
        QString formatStr = selectedFilterString.mid(triggerPos + triggerString.size());
        formatStr = formatStr.left(formatStr.size() - 1);
        format = formatStr.toUpper().toLatin1();
    }

    // If name filter was selected, we guess image type using file extension.
    if (format.isEmpty())
    {
        format = fi.suffix().toUpper().toLatin1();

        QList<QByteArray> imgExtList = QImageWriter::supportedImageFormats();
        imgExtList << "TIF";
        imgExtList << "TIFF";
        imgExtList << "JPG";
        imgExtList << "JPE";

        if (!imgExtList.contains(format))
        {
            QMessageBox::critical(0, i18n("Unsupported Format"), 
                                  i18n("The target image file format \"%1\" is unsupported.",
                                       QLatin1String(format)));
            qCWarning(DIGIKAM_GENERAL_LOG) << "target image file format " << format << " is unsupported!";
            delete imageFileSaveDialog;
            return;
        }
    }

    if (!newURL.isValid())
    {
        QMessageBox::critical(0, i18n("Cannot Save File"), 
                              i18n("Failed to save file\n\"%1\" to\n\"%2\".",
                              newURL.fileName(),
                              newURL.path().section(QLatin1Char('/'), -2, -2)));
        qCWarning(DIGIKAM_GENERAL_LOG) << "target URL is not valid !";
        delete imageFileSaveDialog;
        return;
    }

    // Check for overwrite ----------------------------------------------------------

    if ( fi.exists() )
    {
        int result = QMessageBox::warning(0, i18n("Overwrite File?"),
                                          i18n("A file named \"%1\" already "
                                               "exists. Are you sure you want "
                                               "to overwrite it?",
                                               newURL.fileName()),
                                               QMessageBox::Yes | QMessageBox::No);

        if (result != QMessageBox::Yes)
        {
            delete imageFileSaveDialog;
            return;
        }
    }

    delete imageFileSaveDialog;
    QApplication::setOverrideCursor(Qt::WaitCursor);
    setEnabled(false);

    // Perform saving ---------------------------------------------------------------

    SaveImgThread* const thread = new SaveImgThread(this);

    connect(thread, SIGNAL(signalComplete(QUrl,bool)),
            this, SLOT(slotThreadDone(QUrl,bool)));

    thread->setImageData(ksane_data, width, height, bytes_per_line, ksaneformat);
    thread->setTargetFile(newURL, QLatin1String(format));
    thread->setScannerModel(d->saneWidget->make(), d->saneWidget->model());
    thread->start();
}

void ScanDialog::slotThreadDone(const QUrl& url, bool success)
{
    if (!success)
        QMessageBox::critical(0, i18n("File Not Saved"), i18n("Cannot save \"%1\" file", url.fileName()));

    QApplication::restoreOverrideCursor();
    setEnabled(true);
}

}  // namespace KIPIAcquireImagesPlugin
