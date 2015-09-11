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

#include <QDateTime>
#include <QPushButton>
#include <QPointer>
#include <QDir>
#include <QUrl>
#include <QMenu>
#include <QApplication>
#include <QFileDialog>

// KDE includes

#include <kconfig.h>
#include <klocalizedstring.h>
#include <kwindowconfig.h>

// LibKSane includes

#include <ksane.h>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

class ScanDialog::Private
{
public:

    Private()
    {
        saneWidget = 0;
    }

    KSaneWidget*   saneWidget;
};

ScanDialog::ScanDialog(KSaneWidget* const saneWidget, QWidget* const /*parent*/)
    : KPToolDialog(0),
      d(new Private)
{
    d->saneWidget = saneWidget;
    d->saveThread = new SaveImgThread(this);

    startButton()->hide();
    setWindowTitle(i18n("Scan Image"));
    setModal(false);

    setMainWidget(d->saneWidget);

    // ------------------------------------------------------------------------

    readSettings();

    // ------------------------------------------------------------------------

    connect(d->saneWidget, SIGNAL(imageReady(QByteArray&,int,int,int,int)),
            this, SLOT(slotSaveImage(QByteArray&,int,int,int,int)));

    connect(d->saveThread, SIGNAL(signalComplete(QUrl,bool)),
            this, SLOT(slotThreadDone(QUrl,bool)));

    connect(this, &QDialog::finished,
            this, &ScanDialog::slotDialogFinished);
}

ScanDialog::~ScanDialog()
{
    delete d;
}

void ScanDialog::readSettings()
{
    KConfig config("kipirc");
    KConfigGroup group = config.group(QString("Scan Tool Dialog"));
    KWindowConfig::restoreWindowSize(windowHandle(), group);
}

void ScanDialog::saveSettings()
{
    KConfig config("kipirc");
    KConfigGroup group = config.group(QString("Scan Tool Dialog"));
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
/*
    QStringList writableMimetypes;
    QList<QByteArray> supported = QImageWriter::supportedMimeTypes();
    foreach (QByteArray mimeType, supported)
    {
        writableMimetypes.append(QString::fromLatin1(mimeType));
    }

    // Put first class citizens at first place
    writableMimetypes.removeAll("image/jpeg");
    writableMimetypes.removeAll("image/tiff");
    writableMimetypes.removeAll("image/png");
    writableMimetypes.insert(0, "image/png");
    writableMimetypes.insert(1, "image/jpeg");
    writableMimetypes.insert(2, "image/tiff");

    qCDebug(KIPIPLUGINS_LOG) << "slotSaveImage: Offered mimetypes: " << writableMimetypes;

    QString defaultMimeType("image/png");
    QString defaultFileName("image.png");
    QString place = QDir::homePath();

    if (iface())
        place = iface()->currentAlbum().uploadUrl().toLocalFile();

    QPointer<QFileDialog> imageFileSaveDialog = new QFileDialog(this, i18n("New Image File Name"), place);
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
            KMessageBox::error(0, i18n("The target image file format \"%1\" is unsupported.",
                                       QLatin1String(format)));
            qCWarning(KIPIPLUGINS_LOG) << "target image file format " << format << " is unsupported!";
	    delete imageFileSaveDialog;
            return;
        }
    }

    if (!newURL.isValid())
    {
        KMessageBox::error(0, i18n("Failed to save file\n\"%1\" to\n\"%2\".",
                              newURL.fileName(),
                              newURL.path().section('/', -2, -2)));
        qCWarning(KIPIPLUGINS_LOG) << "target URL is not valid !";
        delete imageFileSaveDialog;
        return;
    }

    // Check for overwrite ----------------------------------------------------------

    if ( fi.exists() )
    {
        int result = KMessageBox::warningYesNo(0, i18n("A file named \"%1\" already "
                                                       "exists. Are you sure you want "
                                                       "to overwrite it?",
                                               newURL.fileName()),
                                               i18n("Overwrite File?"),
                                               KStandardGuiItem::overwrite(),
                                               KStandardGuiItem::cancel());

        if (result != KMessageBox::Yes)
        {
            delete imageFileSaveDialog;
            return;
        }
    }

    delete imageFileSaveDialog;
    setCursor(Qt::WaitCursor);
    setEnabled(false);
    saveSettings();

    // Perform saving ---------------------------------------------------------------

    d->saveThread->setImageData(ksane_data, width, height, bytes_per_line, ksaneformat);
    d->saveThread->setPreviewImage(d->saneWidget->toQImage(ksane_data, width, height,
                                   bytes_per_line, (KSaneWidget::ImageFormat)ksaneformat));
    d->saveThread->setTargetFile(newURL, format);
    d->saveThread->setScannerModel(d->saneWidget->make(), d->saneWidget->model());
    d->saveThread->start();
*/
}

void ScanDialog::slotThreadDone(const QUrl& url, bool success)
{
    if (!success)
        KMessageBox::error(0, i18n("Cannot save \"%1\" file", url.fileName()));

    if (iface())
        iface()->refreshImages( QList<QUrl>() << url );

    unsetCursor();
    setEnabled(true);
}

}  // namespace KIPIAcquireImagesPlugin
