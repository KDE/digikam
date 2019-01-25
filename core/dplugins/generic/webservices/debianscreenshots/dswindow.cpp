/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-11-29
 * Description : a tool to export images to Debian Screenshots
 *
 * Copyright (C) 2010 by Pau Garcia i Quiles <pgquiles at elpauer dot org>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "dswindow.h"

// Qt includes

#include <QFileInfo>
#include <QCloseEvent>
#include <QImageReader>
#include <QComboBox>
#include <QMenu>
#include <QLineEdit>
#include <QPushButton>
#include <QApplication>

// Local includes

#include "drawdecoder.h"
#include "dmessagebox.h"
#include "ditemslist.h"
#include "dmetadata.h"
#include "digikam_debug.h"
#include "dprogresswdg.h"
#include "dstalker.h"
#include "dswidget.h"

namespace GenericDigikamDebianScreenshotsPlugin
{

static int maxWidth  = 800;
static int maxHeight = 600;

DSWindow::DSWindow(const QString& tmpFolder, QWidget* const /*parent*/)
    : WSToolDialog(0, QLatin1String("DebianScreenshots Export Dialog")),
      m_uploadEnabled(false),
      m_imagesCount(0),
      m_imagesTotal(0),
      m_tmpDir(tmpFolder)
{
    m_tmpPath.clear();
    m_talker = new DSTalker(this);
    m_widget = new DSWidget(this);
    m_widget->setMinimumSize(700, 500);

    setMainWidget(m_widget);
    setWindowIcon(QIcon::fromTheme(QLatin1String("dk-debianscreenshots")));
    setModal(false);
    setWindowTitle(i18n("Export to Debian Screenshots"));
    
    startButton()->setText(i18n("Start Upload"));
    startButton()->setToolTip(i18n("Start upload to Debian Screenshots web service"));
    startButton()->setEnabled(false); // Disable until package and version data have been fulfilled

    // ------------------------------------------------------------------------

    connect(m_widget->m_imgList, SIGNAL(signalImageListChanged()),
            this, SLOT(slotMaybeEnableStartButton()));

    connect(m_widget, SIGNAL(requiredPackageInfoAvailable(bool)),
            this, SLOT(slotRequiredPackageInfoAvailableReceived(bool)));

    connect(this, SIGNAL(user1Clicked()),
            this, SLOT(slotStartTransfer()));

    connect(m_widget->progressBar(), SIGNAL(signalProgressCanceled()),
            this, SLOT(slotStopAndCloseProgressBar()));

    connect(m_talker, SIGNAL(signalAddScreenshotDone(int,QString)),
            this, SLOT(slotAddScreenshotDone(int,QString)));
    
    connect(m_buttons, SIGNAL(clicked(QAbstractButton*)),
            this, SLOT(slotButtonClicked(QAbstractButton*)));
    
}

DSWindow::~DSWindow()
{
}

void DSWindow::slotStopAndCloseProgressBar()
{
    m_transferQueue.clear();
    m_widget->m_imgList->cancelProcess();
    m_widget->imagesList()->listView()->clear();
    m_widget->progressBar()->progressCompleted();
    done(QDialog::Accepted);
}

void DSWindow::slotButtonClicked(QAbstractButton* button)
{
    switch (m_buttons->buttonRole(button))
    {
        case QDialogButtonBox::RejectRole:
        {
            if (m_widget->progressBar()->isHidden())
            {
                m_widget->imagesList()->listView()->clear();
                m_widget->progressBar()->progressCompleted();
                done(QDialog::Rejected);
            }
            else // cancel login/transfer
            {
                m_transferQueue.clear();
                m_widget->m_imgList->cancelProcess();
                m_widget->progressBar()->hide();
                m_widget->progressBar()->progressCompleted();
            }
            break;
        }
        case QDialogButtonBox::ActionRole:
        {
            slotStartTransfer();
            break;
        }
    }
}

void DSWindow::reactivate()
{
    m_widget->imagesList()->loadImagesFromCurrentSelection();
    show();
}

void DSWindow::closeEvent(QCloseEvent* e)
{
    if (!e) return;

    m_widget->imagesList()->listView()->clear();
    e->accept();
}

void DSWindow::slotStartTransfer()
{
    m_widget->m_imgList->clearProcessedStatus();
    m_transferQueue = m_widget->m_imgList->imageUrls();

    if (m_transferQueue.isEmpty())
    {
        return;
    }

    m_imagesTotal = m_transferQueue.count();
    m_imagesCount = 0;

    m_widget->progressBar()->setFormat(i18n("%v / %m"));
    m_widget->progressBar()->setMaximum(m_imagesTotal);
    m_widget->progressBar()->setValue(0);
    m_widget->progressBar()->show();
    m_widget->progressBar()->progressScheduled(i18n("Debian Screenshots export"), true, true);
    m_widget->progressBar()->progressThumbnailChanged(QIcon::fromTheme(QLatin1String("dk-debianscreenshots")).pixmap(22, 22));

    uploadNextPhoto();
}

bool DSWindow::prepareImageForUpload(const QString& imgPath, MassageType massage)
{
    QImage image;

    if ( massage == DSWindow::ImageIsRaw )
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Get RAW preview " << imgPath;
        DRawDecoder::loadRawPreview(image, imgPath);
    }
    else
    {
        image.load(imgPath);
    }

    // rescale image if required
    if ( massage == DSWindow::ResizeRequired )
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Resizing image";
        image = image.scaled(maxWidth, maxHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    if ( image.isNull() )
    {
        return false;
    }

    // get temporary file name
    m_tmpPath = m_tmpDir + QFileInfo(imgPath).baseName().trimmed() + QLatin1String(".png");

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Saving to temp file: " << m_tmpPath;
    image.save(m_tmpPath, "PNG");

    return true;
}

void DSWindow::uploadNextPhoto()
{
    if (m_transferQueue.isEmpty())
    {
        m_widget->progressBar()->hide();
        return;
    }

    m_widget->m_imgList->processing(m_transferQueue.first());
    QString imgPath = m_transferQueue.first().toLocalFile();

    m_widget->progressBar()->setMaximum(m_imagesTotal);
    m_widget->progressBar()->setValue(m_imagesCount);

    // screenshots.debian.net only accepts PNG images, 800x600 max
    MassageType massageRequired = DSWindow::None;

    // check if format is PNG
    QImageReader imgReader(imgPath);
    QByteArray imgFormat = imgReader.format();

    if ( QString::fromLatin1(imgFormat).compare(QLatin1String("PNG"), Qt::CaseInsensitive) != 0 )
    {
        massageRequired = DSWindow::NotPNG;
    }

    // check if image > 800x600
    QImage img = imgReader.read();

    if ( (img.width() > maxWidth) || (img.height() > maxHeight) )
    {
        massageRequired = DSWindow::ResizeRequired;
    }

    // check if we have to RAW file -> use preview image then
    if (DRawDecoder::isRawFile(QUrl::fromLocalFile(imgPath)))
    {
        massageRequired = DSWindow::ImageIsRaw;
    }

    bool res;

    if (massageRequired)
    {
        if (!prepareImageForUpload(imgPath, massageRequired))
        {
            slotAddScreenshotDone(666, i18n("Cannot open file"));
            return;
        }
        res = m_talker->addScreenshot(m_tmpPath,
                                      m_widget->m_pkgLineEdit->text(),
                                      m_widget->m_versionsComboBox->currentText(),
                                      m_widget->m_descriptionLineEdit->text());
    }
    else
    {
        m_tmpPath.clear();
        res = m_talker->addScreenshot(imgPath, m_widget->m_pkgLineEdit->text(),
                                      m_widget->m_versionsComboBox->currentText(),
                                      m_widget->m_descriptionLineEdit->text());
    }

    if (!res)
    {
        slotAddScreenshotDone(666, i18n("Cannot open file"));
        return;
    }
}

void DSWindow::slotAddScreenshotDone(int errCode, const QString& errMsg)
{
    // Remove temporary file if it was used
    if (!m_tmpPath.isEmpty())
    {
        QFile::remove(m_tmpPath);
        m_tmpPath.clear();
    }

    m_widget->m_imgList->processed(m_transferQueue.first(), (errCode == 0));

    if (errCode == 0)
    {
        m_transferQueue.pop_front();
        m_imagesCount++;
    }
    else
    {
        if (DMessageBox::showContinueCancel(QMessageBox::Warning,
                                            qApp->activeWindow(),
                                            qApp->applicationName(),
                                            i18n("Failed to upload photo to Debian Screenshots: %1\n"
                                                 "Do you want to continue?", errMsg))
            == QMessageBox::Yes)
        {
            m_widget->progressBar()->hide();
            m_transferQueue.clear();
            return;
        }
    }

    uploadNextPhoto();
}

void DSWindow::slotMaybeEnableStartButton()
{
    startButton()->setEnabled(!(m_widget->m_imgList->imageUrls().isEmpty()) && m_uploadEnabled);
}

void DSWindow::slotRequiredPackageInfoAvailableReceived(bool enabled)
{
    m_uploadEnabled = enabled; // Save the all-data-completed status to be able to enable the upload
                               // button later in case the image list is empty at the moment

    slotMaybeEnableStartButton();
}

} // namespace GenericDigikamDebianScreenshotsPlugin
