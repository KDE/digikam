/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-11-07
 * Description : a tool to print images
 *
 * Copyright (C) 2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "advprinttask.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QImage>
#include <QSize>
#include <QPainter>
#include <QFileInfo>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "advprintwizard.h"
#include "advprintphoto.h"
#include "advprintcaptionpage.h"
#include "dmetadata.h"
#include "dfileoperations.h"
#include "dimg.h"
#include "digikam_debug.h"
#include "digikam_config.h"

namespace Digikam
{

class AdvPrintTask::Private
{
public:

    explicit Private()
      : settings(0),
        mode(AdvPrintTask::PRINT),
        sizeIndex(0)
    {
    }

public:

    AdvPrintSettings* settings;

    PrintMode         mode;
    QSize             size;

    int               sizeIndex;
};

// -------------------------------------------------------

AdvPrintTask::AdvPrintTask(AdvPrintSettings* const settings,
                           PrintMode mode,
                           const QSize& size,
                           int sizeIndex)
    : ActionJob(),
      d(new Private)
{
    d->settings  = settings;
    d->mode      = mode;
    d->size      = size;
    d->sizeIndex = sizeIndex;
}

AdvPrintTask::~AdvPrintTask()
{
    cancel();
    delete d;
}

void AdvPrintTask::run()
{
    switch (d->mode)
    {
        case PREPAREPRINT:

            qCDebug(DIGIKAM_GENERAL_LOG) << "Start prepare to print";
            preparePrint();
            emit signalDone(!m_cancel);
            qCDebug(DIGIKAM_GENERAL_LOG) << "Prepare to print is done";

            break;

        case PRINT:

            qCDebug(DIGIKAM_GENERAL_LOG) << "Start to print";

            if (d->settings->printerName != d->settings->outputName(AdvPrintSettings::FILES) &&
                d->settings->printerName != d->settings->outputName(AdvPrintSettings::GIMP))
            {
                printPhotos();
                emit signalDone(!m_cancel);
            }
            else
            {
                QStringList files = printPhotosToFile();

                if (d->settings->printerName == d->settings->outputName(AdvPrintSettings::GIMP))
                {
                    d->settings->gimpFiles << files;
                }

                emit signalDone(!m_cancel && !files.isEmpty());
            }

            qCDebug(DIGIKAM_GENERAL_LOG) << "Print is done";

            break;

        default:    // PREVIEW

            qCDebug(DIGIKAM_GENERAL_LOG) << "Start to compute preview";

            QImage img(d->size, QImage::Format_ARGB32_Premultiplied);
            QPainter p(&img);
            p.setCompositionMode(QPainter::CompositionMode_Clear);
            p.fillRect(img.rect(), Qt::color0);
            p.setCompositionMode(QPainter::CompositionMode_SourceOver);
            paintOnePage(p,
                         d->settings->photos,
                         d->settings->outputLayouts->m_layouts,
                         d->settings->currentPreviewPage,
                         d->settings->disableCrop,
                         true);
            p.end();

            if (!m_cancel)
                emit signalPreview(img);

            qCDebug(DIGIKAM_GENERAL_LOG) << "Preview computation is done";

            break;
    }
}

void AdvPrintTask::preparePrint()
{
    int photoIndex = 0;

    for (QList<AdvPrintPhoto*>::iterator it = d->settings->photos.begin() ;
         it != d->settings->photos.end() ; ++it)
    {
        AdvPrintPhoto* const photo = static_cast<AdvPrintPhoto*>(*it);

        if (photo && photo->m_cropRegion == QRect(-1, -1, -1, -1))
        {
            QRect* const curr = d->settings->getLayout(photoIndex, d->sizeIndex);

            photo->updateCropRegion(curr->width(),
                                    curr->height(),
                                    d->settings->outputLayouts->m_autoRotate);
        }

        photoIndex++;
        emit signalProgress(photoIndex);

        if (m_cancel)
        {
            signalMessage(i18n("Printing canceled"), true);
            return;
        }
    }
}

void AdvPrintTask::printPhotos()
{
    AdvPrintPhotoSize* const layouts = d->settings->outputLayouts;
    QPrinter* const printer          = d->settings->outputPrinter;

    Q_ASSERT(layouts);
    Q_ASSERT(printer);
    Q_ASSERT(layouts->m_layouts.count() > 1);

    QList<AdvPrintPhoto*> photos = d->settings->photos;
    QPainter p;
    p.begin(printer);

    int current   = 0;
    int pageCount = 1;
    bool printing = true;

    while (printing)
    {
        signalMessage(i18n("Processing page %1", pageCount), false);

        printing = paintOnePage(p,
                                photos,
                                layouts->m_layouts,
                                current,
                                d->settings->disableCrop);

        if (printing)
        {
            printer->newPage();
        }

        pageCount++;
        emit signalProgress(current);

        if (m_cancel)
        {
            printer->abort();
            signalMessage(i18n("Printing canceled"), true);
            return;
        }
    }

    p.end();
}

QStringList AdvPrintTask::printPhotosToFile()
{
    AdvPrintPhotoSize* const layouts = d->settings->outputLayouts;
    QString dir                      = d->settings->outputPath;

    Q_ASSERT(layouts);
    Q_ASSERT(!dir.isEmpty());
    Q_ASSERT(layouts->m_layouts.count() > 1);

    QList<AdvPrintPhoto*> photos = d->settings->photos;

    QStringList files;
    int current          = 0;
    int pageCount        = 1;
    bool printing        = true;
    QRect* const srcPage = layouts->m_layouts.at(0);

    while (printing)
    {
        // make a pixmap to save to file.  Make it just big enough to show the
        // highest-dpi image on the page without losing data.
        double dpi       = layouts->m_dpi;

        if (dpi == 0.0)
        {
            dpi = getMaxDPI(photos, layouts->m_layouts, current) * 1.1;
        }

        int w            = AdvPrintWizard::normalizedInt(srcPage->width());
        int h            = AdvPrintWizard::normalizedInt(srcPage->height());

        QImage image(w, h, QImage::Format_ARGB32_Premultiplied);
        QPainter painter;
        painter.begin(&image);

        QString ext      = d->settings->format();
        QString name     = QLatin1String("output");
        QString filename = dir  + QLatin1String("/")  +
                           name + QLatin1String("_")  +
                           QString::number(pageCount) +
                           QLatin1String(".") + ext;

        if (QFile::exists(filename) &&
            d->settings->conflictRule != FileSaveConflictBox::OVERWRITE)
        {
            filename = DFileOperations::getUniqueFileUrl(QUrl::fromLocalFile(filename)).toLocalFile();
        }

        signalMessage(i18n("Processing page %1", pageCount), false);

        printing = paintOnePage(painter,
                                photos,
                                layouts->m_layouts,
                                current,
                                d->settings->disableCrop);

        painter.end();

        if (!image.save(filename, 0, 100))
        {
            signalMessage(i18n("Could not save file %1", filename), true);
            break;
        }
        else
        {
            files.append(filename);
            signalMessage(i18n("Page %1 saved as %2", pageCount, filename), false);
        }

        pageCount++;
        emit signalProgress(current);

        if (m_cancel)
        {
            signalMessage(i18n("Printing canceled"), true);
            break;
        }
    }

    return files;
}

bool AdvPrintTask::paintOnePage(QPainter& p,
                                const QList<AdvPrintPhoto*>& photos,
                                const QList<QRect*>& layouts,
                                int& current,
                                bool cropDisabled,
                                bool useThumbnails)
{
    if (layouts.isEmpty())
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "Invalid layout content";
        return true;
    }

    if (photos.count() == 0)
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "no photo to print";
        // no photos => last photo
        return true;
    }

    QList<QRect*>::const_iterator it = layouts.begin();
    QRect* const srcPage             = static_cast<QRect*>(*it);
    ++it;
    QRect* layout                    = static_cast<QRect*>(*it);

    // scale the page size to best fit the painter
    // size the rectangle based on the minimum image dimension
    int destW = p.window().width();
    int destH = p.window().height();
    int srcW  = srcPage->width();
    int srcH  = srcPage->height();

    if (destW < destH)
    {
        destH = AdvPrintWizard::normalizedInt((double) destW * ((double) srcH / (double) srcW));

        if (destH > p.window().height())
        {
            destH = p.window().height();
            destW = AdvPrintWizard::normalizedInt((double) destH * ((double) srcW / (double) srcH));
        }
    }
    else
    {
        destW = AdvPrintWizard::normalizedInt((double) destH * ((double) srcW / (double) srcH));

        if (destW > p.window().width())
        {
            destW = p.window().width();
            destH = AdvPrintWizard::normalizedInt((double) destW * ((double) srcH / (double) srcW));
        }
    }

    double xRatio = (double) destW / (double) srcPage->width();
    double yRatio = (double) destH / (double) srcPage->height();
    int left      = (p.window().width()  - destW) / 2;
    int top       = (p.window().height() - destH) / 2;

    // FIXME: may not want to erase the background page
    p.eraseRect(left, top,
                AdvPrintWizard::normalizedInt((double) srcPage->width()  * xRatio),
                AdvPrintWizard::normalizedInt((double) srcPage->height() * yRatio));

    for (; (current < photos.count()) && !m_cancel ; ++current)
    {
        AdvPrintPhoto* const photo = photos.at(current);
        // crop
        QImage img;

        if (useThumbnails)
        {
            img = photo->thumbnail().copyQImage();
        }
        else
        {
            img = photo->loadPhoto().copyQImage();
        }

        // next, do we rotate?
        if (photo->m_rotation != 0)
        {
            // rotate
            QMatrix matrix;
            matrix.rotate(photo->m_rotation);
            img = img.transformed(matrix);
        }

        if (useThumbnails)
        {
            // scale the crop region to thumbnail coords
            double xRatio = 0.0;
            double yRatio = 0.0;

            if (photo->thumbnail().width() != 0)
            {
                xRatio = (double)photo->thumbnail().width()  / (double)photo->width();
            }

            if (photo->thumbnail().height() != 0)
            {
                yRatio = (double)photo->thumbnail().height() / (double)photo->height();
            }

            int x1 = AdvPrintWizard::normalizedInt((double)photo->m_cropRegion.left()   * xRatio);
            int y1 = AdvPrintWizard::normalizedInt((double)photo->m_cropRegion.top()    * yRatio);
            int w  = AdvPrintWizard::normalizedInt((double)photo->m_cropRegion.width()  * xRatio);
            int h  = AdvPrintWizard::normalizedInt((double)photo->m_cropRegion.height() * yRatio);
            img    = img.copy(QRect(x1, y1, w, h));
        }
        else if (!cropDisabled)
        {
            img = img.copy(photo->m_cropRegion);
        }

        int x1 = AdvPrintWizard::normalizedInt((double) layout->left()   * xRatio);
        int y1 = AdvPrintWizard::normalizedInt((double) layout->top()    * yRatio);
        int w  = AdvPrintWizard::normalizedInt((double) layout->width()  * xRatio);
        int h  = AdvPrintWizard::normalizedInt((double) layout->height() * yRatio);

        QRect rectViewPort    = p.viewport();
        QRect newRectViewPort = QRect(x1 + left, y1 + top, w, h);
        QSize imageSize       = img.size();
/*
        qCDebug(DIGIKAM_GENERAL_LOG) << "Image         "
                                     << photo->filename
                                     << " size " << imageSize;
        qCDebug(DIGIKAM_GENERAL_LOG) << "viewport size "
                                     << newRectViewPort.size();
*/
        QPoint point;

        if (cropDisabled)
        {
            imageSize.scale(newRectViewPort.size(), Qt::KeepAspectRatio);
            int spaceLeft = (newRectViewPort.width()  - imageSize.width())  / 2;
            int spaceTop  = (newRectViewPort.height() - imageSize.height()) / 2;
            p.setViewport(spaceLeft + newRectViewPort.x(),
                          spaceTop  + newRectViewPort.y(),
                          imageSize.width(),
                          imageSize.height());
            point         = QPoint(newRectViewPort.x() + spaceLeft + imageSize.width(),
                                   newRectViewPort.y() + spaceTop  + imageSize.height());
        }
        else
        {
            p.setViewport(newRectViewPort);
            point = QPoint(x1 + left + w, y1 + top + w);
        }

        QRect rectWindow = p.window();
        p.setWindow(img.rect());
        p.drawImage(0, 0, img);
        p.setViewport(rectViewPort);
        p.setWindow(rectWindow);
        p.setBrushOrigin(point);

        if (photo->m_pAdvPrintCaptionInfo &&
            photo->m_pAdvPrintCaptionInfo->m_captionType != AdvPrintSettings::NONE)
        {
            p.save();
            QString caption = AdvPrintCaptionPage::captionFormatter(photo);

            qCDebug(DIGIKAM_GENERAL_LOG) << "Caption for"
                                         << photo->m_url
                                         << ":"
                                         << caption;

            // draw the text at (0,0), but we will translate and rotate the world
            // before drawing so the text will be in the correct location
            // next, do we rotate?
            int captionW        = w - 2;
            double ratio        = photo->m_pAdvPrintCaptionInfo->m_captionSize * 0.01;
            int captionH        = (int)(qMin(w, h) * ratio);
            int exifOrientation = DMetadata::ORIENTATION_NORMAL;
            int orientatation   = photo->m_rotation;

            if (photo->m_iface)
            {
                DItemInfo info(photo->m_iface->itemInfo(photo->m_url));
                exifOrientation = info.orientation();
            }
            else
            {
                DMetadata meta(photo->m_url.toLocalFile());
                exifOrientation = meta.getImageOrientation();
            }

            // ROT_90_HFLIP .. ROT_270

            if (exifOrientation == DMetadata::ORIENTATION_ROT_90_HFLIP ||
                exifOrientation == DMetadata::ORIENTATION_ROT_90       ||
                exifOrientation == DMetadata::ORIENTATION_ROT_90_VFLIP ||
                exifOrientation == DMetadata::ORIENTATION_ROT_270)
            {
                orientatation = (photo->m_rotation + 270) % 360;   // -90 degrees
            }

            if (orientatation == 90 || orientatation == 270)
            {
                captionW = h;
            }

            p.rotate(orientatation);
            qCDebug(DIGIKAM_GENERAL_LOG) << "rotation "
                                         << photo->m_rotation
                                         << " orientation "
                                         << orientatation;
            int tx = left;
            int ty = top;

            switch (orientatation)
            {
                case 0 :
                {
                    tx += x1 + 1;
                    ty += y1 + (h - captionH - 1);
                    break;
                }
                case 90 :
                {
                    tx = top + y1 + 1;
                    ty = -left - x1 - captionH - 1;
                    break;
                }
                case 180 :
                {
                    tx = -left - x1 - w + 1;
                    ty = -top - y1 - (captionH + 1);
                    break;
                }
                case 270 :
                {
                    tx = -top - y1 - h + 1;
                    ty = left + x1 + (w - captionH) - 1;
                    break;
                }
            }

            p.translate(tx, ty);
            printCaption(p, photo, captionW, captionH, caption);
            p.restore();
        }

        // iterate to the next position
        ++it;
        layout = (it == layouts.end()) ? 0 : static_cast<QRect*>(*it);

        if (layout == 0)
        {
            current++;
            break;
        }
    }

    // did we print the last photo?
    return (current < photos.count());
}

double AdvPrintTask::getMaxDPI(const QList<AdvPrintPhoto*>& photos,
                               const QList<QRect*>& layouts,
                               int current)
{
    Q_ASSERT(layouts.count() > 1);

    QList<QRect*>::const_iterator it = layouts.begin();
    QRect* layout                    = static_cast<QRect*>(*it);
    double maxDPI                    = 0.0;

    for (; current < photos.count(); ++current)
    {
        AdvPrintPhoto* const photo   = photos.at(current);
        double dpi                   = ((double) photo->m_cropRegion.width() +
                                        (double) photo->m_cropRegion.height()) /
                                       (((double) layout->width()  / 1000.0) +
                                        ((double) layout->height() / 1000.0));

        if (dpi > maxDPI)
            maxDPI = dpi;

        // iterate to the next position
        ++it;
        layout = (it == layouts.end()) ? 0 : static_cast<QRect*>(*it);

        if (layout == 0)
        {
            break;
        }
    }

    return maxDPI;
}

void AdvPrintTask::printCaption(QPainter& p,
                                AdvPrintPhoto* const photo,
                                int captionW,
                                int captionH,
                                const QString& caption)
{
    QStringList captionByLines;

    int captionIndex = 0;

    while (captionIndex < caption.length())
    {
        QString newLine;
        bool breakLine = false; // End Of Line found
        int currIndex;          // Caption QString current index

        // Check minimal lines dimension
        // TODO: fix length, maybe useless
        int captionLineLocalLength = 40;

        for (currIndex = captionIndex ;
             currIndex < caption.length() && !breakLine ; ++currIndex)
        {
            if (caption[currIndex] == QLatin1Char('\n') ||
                caption[currIndex].isSpace())
            {
                breakLine = true;
            }
        }

        if (captionLineLocalLength <= (currIndex - captionIndex))
        {
            captionLineLocalLength = (currIndex - captionIndex);
        }

        breakLine = false;

        for (currIndex = captionIndex;
             (currIndex <= captionIndex + captionLineLocalLength) &&
             (currIndex < caption.length()) && !breakLine;
             ++currIndex)
        {
            breakLine = (caption[currIndex] == QLatin1Char('\n')) ? true : false;

            if (breakLine)
                newLine.append(QLatin1Char(' '));
            else
                newLine.append(caption[currIndex]);
        }

        captionIndex = currIndex; // The line is ended

        if (captionIndex != caption.length())
        {
            while (!newLine.endsWith(QLatin1Char(' ')))
            {
                newLine.truncate(newLine.length() - 1);
                captionIndex--;
            }
        }

        captionByLines.prepend(newLine.trimmed());
    }

    QFont font(photo->m_pAdvPrintCaptionInfo->m_captionFont);
    font.setStyleHint(QFont::SansSerif);
    font.setPixelSize((int)(captionH * 0.8F)); // Font height ratio
    font.setWeight(QFont::Normal);

    QFontMetrics fm(font);
    int pixelsHigh = fm.height();

    p.setFont(font);
    p.setPen(photo->m_pAdvPrintCaptionInfo->m_captionColor);

    qCDebug(DIGIKAM_GENERAL_LOG) << "Number of lines "
                                 << (int) captionByLines.count() ;

    // Now draw the caption
    // TODO allow printing captions  per photo and on top, bottom and vertically
    for (int lineNumber = 0 ;
         lineNumber < (int) captionByLines.count() ; ++lineNumber)
    {
        if (lineNumber > 0)
        {
            p.translate(0, - (int)(pixelsHigh));
        }

        QRect r(0, 0, captionW, captionH);

        p.drawText(r, Qt::AlignLeft, captionByLines[lineNumber], &r);
    }
}

} // namespace Digikam
