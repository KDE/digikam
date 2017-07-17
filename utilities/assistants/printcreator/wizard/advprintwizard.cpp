/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-01-11
 * Description : a tool to print images
 *
 * Copyright (C) 2008-2012 by Angelo Naselli <anaselli at linux dot it>
 * Copyright (C) 2006-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "advprintwizard.h"

// C++ includes

#include <memory>

// Qt includes

#include <QFileInfo>
#include <QPainter>
#include <QPalette>
#include <QtGlobal>
#include <QPrintDialog>
#include <QProgressDialog>
#include <QDomDocument>
#include <QContextMenuEvent>
#include <QStringRef>
#include <QStandardPaths>
#include <QMenu>
#include <QIcon>
#include <QLocale>
#include <QMessageBox>
#include <QListWidgetItem>
#include <QTemporaryDir>
#include <QProcess>
#include <QDesktopServices>
#include <QKeyEvent>

// KDE includes

#include <kconfig.h>
#include <kconfiggroup.h>
#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"
#include "digikam_debug.h"
#include "advprintthread.h"
#include "advprintintropage.h"
#include "advprintalbumspage.h"
#include "advprintphotopage.h"
#include "advprintcaptionpage.h"
#include "advprintcroppage.h"
#include "advprintoutputpage.h"
#include "advprintfinalpage.h"
#include "templateicon.h"
#include "dwizardpage.h"
#include "dinfointerface.h"
#include "dfiledialog.h"
#include "dmetadata.h"

namespace Digikam
{

class AdvPrintWizard::Private
{
public:

    Private()
      : introPage(0),
        albumsPage(0),
        photoPage(0),
        captionPage(0),
        cropPage(0),
        outputPage(0),
        finalPage(0),
        settings(0),
        previewThread(0),
        iface(0)
    {
    }

    AdvPrintIntroPage*        introPage;
    AdvPrintAlbumsPage*       albumsPage;
    AdvPrintPhotoPage*        photoPage;
    AdvPrintCaptionPage*      captionPage;
    AdvPrintCropPage*         cropPage;
    AdvPrintOutputPage*       outputPage;
    AdvPrintFinalPage*        finalPage;
    AdvPrintSettings*         settings;
    AdvPrintThread*           previewThread;
    DInfoInterface*           iface;
};

AdvPrintWizard::AdvPrintWizard(QWidget* const parent, DInfoInterface* const iface)
    : DWizardDlg(parent, QLatin1String("PrintCreatorDialog")),
      d(new Private)
{
    setWindowTitle(i18n("Print Creator"));

    d->iface         = iface;
    d->settings      = new AdvPrintSettings;
    d->previewThread = new AdvPrintThread(this);

    KConfig config;
    KConfigGroup group = config.group("PrintCreator");
    d->settings->readSettings(group);

    d->introPage   = new AdvPrintIntroPage(this,   i18n("Welcome to Print Creator"));
    d->albumsPage  = new AdvPrintAlbumsPage(this,  i18n("Albums Selection"));
    d->photoPage   = new AdvPrintPhotoPage(this,   i18n("Select Page Layout"));
    d->captionPage = new AdvPrintCaptionPage(this, i18n("Caption Settings"));
    d->cropPage    = new AdvPrintCropPage(this,    i18n("Crop and Rotate Photos"));
    d->outputPage  = new AdvPrintOutputPage(this,  i18n("Images Output Settings"));
    d->finalPage   = new AdvPrintFinalPage(this,   i18n("Render Printing"));

    // -----------------------------------

    connect(button(QWizard::CancelButton), SIGNAL(clicked()),
            this, SLOT(reject()));

    connect(d->photoPage->imagesList(), SIGNAL(signalImageListChanged()),
            d->captionPage, SLOT(slotUpdateImagesList()));

    connect(d->previewThread, SIGNAL(signalPreview(QImage)),
            this, SLOT(slotPreview(QImage)));

    installEventFilter(this);
}

AdvPrintWizard::~AdvPrintWizard()
{
    d->previewThread->cancel();

    KConfig config;
    KConfigGroup group = config.group("PrintCreator");
    d->settings->writeSettings(group);

    delete d;
}

DInfoInterface* AdvPrintWizard::iface() const
{
    return d->iface;
}

AdvPrintSettings* AdvPrintWizard::settings() const
{
    return d->settings;
}

int AdvPrintWizard::nextId() const
{
    if (d->settings->selMode == AdvPrintSettings::ALBUMS)
    {
        if (currentPage() == d->introPage)
        {
            return d->albumsPage->id();
        }
    }
    else
    {
        if (currentPage() == d->introPage)
        {
            return d->photoPage->id();
        }
    }

    if (d->settings->printerName == d->settings->outputName(AdvPrintSettings::FILES))
    {
        if (currentPage() == d->cropPage)
        {
            return d->outputPage->id();
        }
    }
    else
    {
        if (currentPage() == d->cropPage)
        {
            return d->finalPage->id();
        }
    }

    return DWizardDlg::nextId();
}

QList<QUrl> AdvPrintWizard::itemsList() const
{
    QList<QUrl> urls;

    for (QList<AdvPrintPhoto*>::iterator it = d->settings->photos.begin() ;
         it != d->settings->photos.end() ; ++it)
    {
        AdvPrintPhoto* const photo = static_cast<AdvPrintPhoto*>(*it);
        urls << photo->m_url;
    }

    return urls;
}

void AdvPrintWizard::setItemsList(const QList<QUrl>& fileList)
{
    QList<QUrl> list = fileList;

    for (int i = 0 ; i < d->settings->photos.count() ; ++i)
    {
        delete d->settings->photos.at(i);
    }

    d->settings->photos.clear();

    if (list.isEmpty() && d->iface)
    {
        list = d->iface->currentSelectedItems();
    }

    for (int i = 0 ; i < list.count() ; ++i)
    {
        AdvPrintPhoto* const photo = new AdvPrintPhoto(150, d->iface);
        photo->m_url               = list[i];
        photo->m_first             = true;
        d->settings->photos.append(photo);
    }

    QTemporaryDir tempPath;
    d->settings->tempPath = tempPath.path();
    d->cropPage->ui()->BtnCropPrev->setEnabled(false);

    if (d->settings->photos.count() == 1)
    {
        d->cropPage->ui()->BtnCropNext->setEnabled(false);
    }

    emit currentIdChanged(d->photoPage->id());
}

void AdvPrintWizard::updateCropFrame(AdvPrintPhoto* const photo, int photoIndex)
{
    AdvPrintPhotoSize* const s = d->settings->photosizes.at(d->photoPage->ui()->ListPhotoSizes->currentRow());

    d->cropPage->ui()->cropFrame->init(photo,
                                       d->photoPage->getLayout(photoIndex)->width(),
                                       d->photoPage->getLayout(photoIndex)->height(),
                                       s->m_autoRotate,
                                       true);

    d->cropPage->ui()->LblCropPhoto->setText(i18n("Photo %1 of %2",
                                             photoIndex + 1,
                                             QString::number(d->settings->photos.count())));
}

void AdvPrintWizard::previewPhotos()
{
    if (d->settings->photosizes.isEmpty())
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Photo sizes is null";
        return;
    }

    d->previewThread->cancel();

    // get the selected layout
    int photoCount             = d->settings->photos.count();
    int curr                   = d->photoPage->ui()->ListPhotoSizes->currentRow();
    AdvPrintPhotoSize* const s = d->settings->photosizes.at(curr);
    int emptySlots             = 0;
    int pageCount              = 0;
    int photosPerPage          = 0;

    if (photoCount > 0)
    {
        // how many pages?  Recall that the first layout item is the paper size
        photosPerPage = s->m_layouts.count() - 1;
        int remainder = photoCount % photosPerPage;

        if (remainder > 0)
            emptySlots = photosPerPage - remainder;

        pageCount     = photoCount / photosPerPage;

        if (emptySlots > 0)
            pageCount++;
    }

    d->photoPage->ui()->LblPhotoCount->setText(QString::number(photoCount));
    d->photoPage->ui()->LblSheetsPrinted->setText(QString::number(pageCount));
    d->photoPage->ui()->LblEmptySlots->setText(QString::number(emptySlots));

    // photo previews
    // preview the first page.
    // find the first page of photos
    int count   = 0;
    int page    = 0;
    int current = 0;

    for (QList<AdvPrintPhoto*>::iterator it = d->settings->photos.begin() ;
         it != d->settings->photos.end() ; ++it)
    {
        AdvPrintPhoto* const photo = static_cast<AdvPrintPhoto*>(*it);

        if (page == d->settings->currentPreviewPage)
        {
            photo->m_cropRegion.setRect(-1, -1, -1, -1);
            photo->m_rotation = 0;
            d->cropPage->ui()->cropFrame->init(photo,
                                               s->m_layouts.at(count + 1)->width(),
                                               s->m_layouts.at(count + 1)->height(),
                                               s->m_autoRotate,
                                               false);
        }

        count++;

        if (count >= photosPerPage)
        {
            if (page == d->settings->currentPreviewPage)
                break;

            page++;
            current += photosPerPage;
            count    = 0;
        }
    }

    // send this photo list to the painter
    if (photoCount > 0)
    {
        AdvPrintSettings* const pwSettings = new AdvPrintSettings;
        pwSettings->photos                 = d->settings->photos;
        pwSettings->outputLayouts          = s;
        pwSettings->currentPreviewPage     = current;
        pwSettings->disableCrop            = d->cropPage->ui()->m_disableCrop->isChecked();

        d->previewThread->preview(pwSettings, d->photoPage->ui()->BmpFirstPagePreview->size());
        d->previewThread->start();
    }
    else
    {
        d->photoPage->ui()->BmpFirstPagePreview->clear();
        d->photoPage->ui()->LblPreview->clear();
        d->photoPage->ui()->LblPreview->setText(i18n("Page %1 of %2", 0, 0));
        d->photoPage->manageBtnPreviewPage();
        d->photoPage->update();
    }
}

void AdvPrintWizard::slotPreview(const QImage& img)
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "Recieve Preview" << img.size();

    d->photoPage->ui()->BmpFirstPagePreview->clear();
    d->photoPage->ui()->BmpFirstPagePreview->setPixmap(QPixmap::fromImage(img));
    d->photoPage->ui()->LblPreview->setText(i18n("Page %1 of %2",
                                                 d->settings->currentPreviewPage + 1,
                                                 d->photoPage->getPageCount()));
    d->photoPage->manageBtnPreviewPage();
    d->photoPage->update();
}

bool AdvPrintWizard::prepareToPrint()
{
    if (!d->settings->photos.empty())
    {
        // set the default crop regions if not already set
        d->settings->outputLayouts = d->settings->photosizes.at(
                                        d->photoPage->ui()->ListPhotoSizes->currentRow());
        int i                      = 0;

        for (QList<AdvPrintPhoto*>::iterator it = d->settings->photos.begin() ;
            it != d->settings->photos.end() ; ++it)
        {
            AdvPrintPhoto* const photo = static_cast<AdvPrintPhoto*>(*it);

            if (photo && photo->m_cropRegion == QRect(-1, -1, -1, -1))
            {
                d->cropPage->ui()->cropFrame->init(photo,
                                                   d->photoPage->getLayout(i)->width(),
                                                   d->photoPage->getLayout(i)->height(),
                                                   d->settings->outputLayouts->m_autoRotate,
                                                   true);
            }

            i++;
        }

        // Real printer to use.

        if (d->settings->printerName != d->settings->outputName(AdvPrintSettings::FILES) &&
            d->settings->printerName != d->settings->outputName(AdvPrintSettings::GIMP))
        {
            // tell him again!
            d->photoPage->printer()->setFullPage(true);

            qreal left, top, right, bottom;
            d->photoPage->printer()->getPageMargins(&left,
                                                    &top,
                                                    &right,
                                                    &bottom,
                                                    QPrinter::Millimeter);

            qCDebug(DIGIKAM_GENERAL_LOG) << "Margins before print dialog: left "
                                        << left
                                        << " right "
                                        << right
                                        << " top "
                                        << top
                                        << " bottom "
                                        << bottom;

            qCDebug(DIGIKAM_GENERAL_LOG) << "(1) paper page "
                                        << d->photoPage->printer()->paperSize()
                                        << " size "
                                        << d->photoPage->printer()->paperSize(QPrinter::Millimeter);

            QPrinter::PaperSize paperSize = d->photoPage->printer()->paperSize();
            QPrintDialog* const dialog    = new QPrintDialog(d->photoPage->printer(), this);
            dialog->setWindowTitle(i18n("Print Creator"));

            qCDebug(DIGIKAM_GENERAL_LOG) << "(2) paper page "
                                        << dialog->printer()->paperSize()
                                        << " size "
                                        << dialog->printer()->paperSize(QPrinter::Millimeter);

            if (dialog->exec() != QDialog::Accepted)
            {
                return false;
            }

            qCDebug(DIGIKAM_GENERAL_LOG) << "(3) paper page "
                                        << dialog->printer()->paperSize()
                                        << " size "
                                        << dialog->printer()->paperSize(QPrinter::Millimeter);

            // Why paperSize changes if printer properties is not pressed?
            if (paperSize != d->photoPage->printer()->paperSize())
            {
                d->photoPage->printer()->setPaperSize(paperSize);
            }

            qCDebug(DIGIKAM_GENERAL_LOG) << "(4) paper page "
                                        << dialog->printer()->paperSize()
                                        << " size "
                                        << dialog->printer()->paperSize(QPrinter::Millimeter);

            dialog->printer()->getPageMargins(&left, &top, &right, &bottom, QPrinter::Millimeter);

            qCDebug(DIGIKAM_GENERAL_LOG) << "Dialog exit, new margins: left "
                                        << left
                                        << " right "
                                        << right
                                        << " top "
                                        << top
                                        << " bottom "
                                        << bottom;

            d->settings->outputPrinter = d->photoPage->printer();

            return true;
        }
        else if (d->settings->printerName == d->settings->outputName(AdvPrintSettings::GIMP))
        {
            d->settings->imageFormat = AdvPrintSettings::JPEG;

            if (!AdvPrintCheckTempPath(this, d->settings->tempPath))
            {
                return false;
            }

            if (d->settings->gimpFiles.count() > 0)
            {
                d->finalPage->removeGimpFiles();
            }

            d->settings->outputPath = d->settings->tempPath;

            return true;
        }
        else if (d->settings->printerName == d->settings->outputName(AdvPrintSettings::FILES))
        {
            d->settings->outputPath = d->settings->outputDir.toLocalFile();

            return true;
        }
    }

    return false;
}

bool AdvPrintWizard::AdvPrintCheckTempPath(QWidget* const parent, const QString& tempPath) const
{
    // does the temp path exist?
    QDir tempDir(tempPath);

    if (!tempDir.exists())
    {
        if (!tempDir.mkdir(tempDir.path()))
        {
            QMessageBox::information(parent, QString(),
                                     i18n("Unable to create a temporary folder. "
                                          "Please make sure you have proper permissions "
                                          "to this folder and try again."));
            return false;
        }
    }

    return true;
}

int AdvPrintWizard::normalizedInt(double n)
{
    return (int)(n + 0.5);
}

bool AdvPrintWizard::eventFilter(QObject* o, QEvent* e)
{
    if (e && e->type() == QEvent::KeyRelease)
    {
        QKeyEvent* const k = (QKeyEvent*)e;

        if ((k->key() == Qt::Key_PageUp)   ||
            (k->key() == Qt::Key_PageDown) ||
            (k->key() == Qt::Key_Up)       ||
            (k->key() == Qt::Key_Down))
        {
            if (currentPage() == d->cropPage)
            {
                // Pass the key event to move crop frame region.
                d->cropPage->ui()->cropFrame->setFocus();
                QApplication::sendEvent(d->cropPage->ui()->cropFrame, e);
                return true; // eat event
            }
        }
    }

    return QWizard::eventFilter(o, e);
}

} // namespace Digikam
