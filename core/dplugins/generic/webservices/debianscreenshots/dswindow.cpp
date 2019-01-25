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

// KDE includes

#include <QMenu>
#include <klineedit.h>
#include <QComboBox>
#include <kpushbutton.h>
#include <kmessagebox.h>
#include "digikam_debug.h"

// LibKDcraw includes

#include <KDCRAW/KDcraw>

// Local includes

#include "ditemslist.h"
#include "dmetadata.h"
#include "dprogresswidget.h"
#include "dstalker.h"
#include "dswidget.h"

namespace GenericDigikamDebianScreenshotsPlugin
{

static int maxWidth  = 800;
static int maxHeight = 600;

DsWindow::DsWindow(const QString& tmpFolder, QWidget* const /*parent*/)
    : KP4ToolDialog(0),
      m_uploadEnabled(false),
      m_imagesCount(0),
      m_imagesTotal(0),
//      m_talker(new DsTalker(this)),
      m_tmpDir(tmpFolder)
{
    m_tmpPath.clear();
//    m_tmpDir      = tmpFolder;
//    m_imagesCount = 0;
//    m_imagesTotal = 0;
    m_talker = new DsTalker(this);
    m_widget = new DsWidget(this);

    setMainWidget(m_widget);
    setWindowIcon(QIcon::fromTheme("dk-debianscreenshots"));
    setButtons(Help|User1|Close);
    setDefaultButton(Close);
    setModal(false);

    setWindowTitle(i18n("Export to Debian Screenshots"));
    setButtonGuiItem(User1,
                     KGuiItem(i18n("Start Upload"), "network-workgroup",
                              i18n("Start upload to Debian Screenshots")));
    enableButton(User1, false); // Disable until package and version data have been fulfilled
    m_widget->setMinimumSize(700, 500);

    // ------------------------------------------------------------------------

    connect(m_widget->m_imgList, SIGNAL(signalImageListChanged()),
            this, SLOT(slotMaybeEnableUser1()) );

    connect(m_widget, SIGNAL(requiredPackageInfoAvailable(bool)),
            this, SLOT(slotRequiredPackageInfoAvailableReceived(bool)));

    connect(this, SIGNAL(user1Clicked()),
            this, SLOT(slotStartTransfer()) );

    connect(m_widget->progressBar(), SIGNAL(signalProgressCanceled()),
            this, SLOT(slotStopAndCloseProgressBar()));

    // ------------------------------------------------------------------------

    KPAboutData* const about = new KPAboutData(ki18n("Debian Screenshots Export"), 0,
                                   KAboutLicense::GPL,
                                   ki18n("A Kipi plugin to export an image collection "
                                         "to the Debian Screenshots web site."),
                                   ki18n("(c) 2010, Pau Garcia i Quiles\n"));

    about->addAuthor(ki18n("Pau Garcia i Quiles"), ki18n("Author and maintainer"),
                     "pgquiles at elpauer dot org");

    about->setHandbookEntry("debianscreenshots");
    setAboutData(about);

    // ------------------------------------------------------------------------

    connect(m_talker, SIGNAL(signalAddScreenshotDone(int,QString)),
            this, SLOT(slotAddScreenshotDone(int,QString)));
}

DsWindow::~DsWindow()
{
}

void DsWindow::slotStopAndCloseProgressBar()
{
    m_transferQueue.clear();
    m_widget->m_imgList->cancelProcess();
    m_widget->imagesList()->listView()->clear();
    m_widget->progressBar()->progressCompleted();
    done(Close);
}

void DsWindow::slotButtonClicked(int button)
{
    switch (button)
    {
        case Close:
        {
            if (m_widget->progressBar()->isHidden())
            {
                m_widget->imagesList()->listView()->clear();
                m_widget->progressBar()->progressCompleted();
                done(Close);
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
        case User1:
        {
            slotStartTransfer();
            break;
        }
        default:
        {
             KDialog::slotButtonClicked(button);
             break;
        }
    }
}

void DsWindow::reactivate()
{
    m_widget->imagesList()->loadImagesFromCurrentSelection();
    show();
}

void DsWindow::closeEvent(QCloseEvent* e)
{
    if (!e) return;

    m_widget->imagesList()->listView()->clear();
    e->accept();
}

void DsWindow::slotStartTransfer()
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
    m_widget->progressBar()->progressThumbnailChanged(QIcon::fromTheme("dk-debianscreenshots").pixmap(22, 22));

    uploadNextPhoto();
}

bool DsWindow::prepareImageForUpload(const QString& imgPath, MassageType massage)
{
    QImage image;

    if ( massage == DsWindow::ImageIsRaw )
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Get RAW preview " << imgPath;
        KDcrawIface::KDcraw::loadRawPreview(image, imgPath);
    }
    else
    {
        image.load(imgPath);
    }

    // rescale image if required
    if ( massage == DsWindow::ResizeRequired )
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Resizing image";
        image = image.scaled(maxWidth, maxHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    if ( image.isNull() )
    {
        return false;
    }

    // get temporary file name
    m_tmpPath = m_tmpDir + QFileInfo(imgPath).baseName().trimmed() + ".png";

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Saving to temp file: " << m_tmpPath;
    image.save(m_tmpPath, "PNG");

    return true;
}

void DsWindow::uploadNextPhoto()
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
    MassageType massageRequired = DsWindow::None;

    // check if format is PNG
    QImageReader imgReader(imgPath);
    QByteArray imgFormat = imgReader.format();

    if( QString::compare(QString(imgFormat), QString("PNG"), Qt::CaseInsensitive) != 0 )
    {
        massageRequired = DsWindow::NotPNG;
    }

    // check if image > 800x600
    QImage img = imgReader.read();

    if( (img.width() > maxWidth) || (img.height() > maxHeight) )
    {
        massageRequired = DsWindow::ResizeRequired;
    }

    // check if we have to RAW file -> use preview image then
    if( KPMetadata::isRawFile(imgPath) )
    {
        massageRequired = DsWindow::ImageIsRaw;
    }

    bool res;

    if (massageRequired)
    {
        if (!prepareImageForUpload(imgPath, massageRequired))
        {
            slotAddScreenshotDone(666, i18n("Cannot open file"));
            return;
        }
        res = m_talker->addScreenshot(m_tmpPath, m_widget->m_pkgLineEdit->text(),
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

void DsWindow::slotAddScreenshotDone(int errCode, const QString& errMsg)
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
        if (KMessageBox::warningContinueCancel(this,
                         i18n("Failed to upload photo to Debian Screenshots: %1\n"
                              "Do you want to continue?", errMsg))
                         != KMessageBox::Continue)
        {
            m_widget->progressBar()->hide();
            m_transferQueue.clear();
            return;
        }
    }

    uploadNextPhoto();
}

void DsWindow::slotMaybeEnableUser1()
{
    enableButton(User1, !(m_widget->m_imgList->imageUrls().isEmpty()) && m_uploadEnabled );
}

void DsWindow::slotRequiredPackageInfoAvailableReceived(bool enabled)
{
    m_uploadEnabled = enabled; // Save the all-data-completed status to be able to enable the upload
                               // button later in case the image list is empty at the moment

    slotMaybeEnableUser1();
}

} // namespace GenericDigikamDebianScreenshotsPlugin
