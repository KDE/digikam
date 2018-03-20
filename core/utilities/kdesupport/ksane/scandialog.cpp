/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-09-09
 * Description : scanner dialog
 *
 * Copyright (C) 2007-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include <QMessageBox>
#include <QImageWriter>

// KDE includes

#include <kconfig.h>
#include <klocalizedstring.h>

// LibKSane includes

#include <ksanewidget.h>

// Local includes

#include "digikam_debug.h"
#include "saveimgthread.h"
#include "statusprogressbar.h"
#include "dxmlguiwindow.h"
#include "dfiledialog.h"

namespace Digikam
{

class ScanDialog::Private
{
public:

    Private()
    {
        progress   = 0;
        saneWidget = 0;
    }

    QString            targetDir;
    QString            configGroupName;
    StatusProgressBar* progress;
    KSaneWidget*       saneWidget;
};

ScanDialog::ScanDialog(KSaneWidget* const saneWdg, const QString& config, QWidget* const parent)
    : QDialog(parent),
      d(new Private)
{
    setWindowTitle(i18n("Scan Image"));
    setModal(false);

    d->saneWidget      = saneWdg;
    d->configGroupName = config;

    d->progress = new StatusProgressBar(this);
    d->progress->setProgressBarMode(StatusProgressBar::ProgressBarMode);
    d->progress->setProgressTotalSteps(100);
    d->progress->setNotify(true);
    d->progress->setNotificationTitle(i18n("Scan Images"), QIcon::fromTheme(QLatin1String("scanner")));

    QVBoxLayout* const vbx = new QVBoxLayout(this);
    vbx->addWidget(d->saneWidget, 10);
    vbx->addWidget(d->progress);
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

void ScanDialog::setTargetDir(const QString& targetDir)
{
    d->targetDir = targetDir;
}

void ScanDialog::readSettings()
{
    KConfig config(d->configGroupName);
    KConfigGroup group = config.group(QLatin1String("Scan Tool Dialog"));

    winId();
    DXmlGuiWindow::restoreWindowSize(windowHandle(), group);
    resize(windowHandle()->size());
}

void ScanDialog::saveSettings()
{
    KConfig config(d->configGroupName);
    KConfigGroup group = config.group(QLatin1String("Scan Tool Dialog"));
    DXmlGuiWindow::saveWindowSize(windowHandle(), group);
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

    QPointer<DFileDialog> imageFileSaveDialog = new DFileDialog(0, i18n("New Image File Name"), d->targetDir);
    imageFileSaveDialog->setAcceptMode(DFileDialog::AcceptSave);
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
    QString format;

    if (triggerPos != -1)
    {
        format = selectedFilterString.mid(triggerPos + triggerString.size());
        format = format.left(format.size() - 1);
        format = format.toUpper();
    }

    // If name filter was selected, we guess image type using file extension.
    if (format.isEmpty())
    {
        format = fi.suffix().toUpper();

        QList<QByteArray> imgExtList = QImageWriter::supportedImageFormats();
        imgExtList << "TIF";
        imgExtList << "TIFF";
        imgExtList << "JPG";
        imgExtList << "JPE";

        if (!imgExtList.contains(format.toLatin1()) && !imgExtList.contains(format.toLower().toLatin1()))
        {
            QMessageBox::critical(0, i18n("Unsupported Format"),
                                  i18n("The target image file format \"%1\" is unsupported.", format));
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
                              QDir::toNativeSeparators(newURL.toLocalFile().section(QLatin1Char('/'), -2, -2))));
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

    connect(thread, SIGNAL(signalProgress(QUrl,int)),
            this, SLOT(slotThreadProgress(QUrl,int)));

    connect(thread, SIGNAL(signalComplete(QUrl,bool)),
            this, SLOT(slotThreadDone(QUrl,bool)));

    thread->setImageData(ksane_data, width, height, bytes_per_line, ksaneformat);
    thread->setTargetFile(newURL, format);
    thread->setScannerModel(d->saneWidget->make(), d->saneWidget->model());
    thread->start();
}

void ScanDialog::slotThreadProgress(const QUrl& url, int percent)
{
    d->progress->setProgressText(i18n("Saving file %1 -", url.fileName()));
    d->progress->setProgressValue(percent);
}

void ScanDialog::slotThreadDone(const QUrl& url, bool success)
{
    if (!success)
    {
        QMessageBox::critical(0, i18n("File Not Saved"), i18n("Cannot save \"%1\" file", url.fileName()));
    }

    d->progress->setProgressText(QLatin1String(""));
    QApplication::restoreOverrideCursor();
    setEnabled(true);

    if (success)
    {
        emit signalImportedImage(url);
    }
}

}  // namespace Digikam
