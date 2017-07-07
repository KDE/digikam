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

// KDE includes

#include <kconfigdialogmanager.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdesktopfile.h>
#include <klocalizedstring.h>

// Local includes

#include "advprintutils.h"
#include "advprintintropage.h"
#include "advprintalbumspage.h"
#include "advprintphotopage.h"
#include "advprintcaptionpage.h"
#include "advprintcroppage.h"
#include "digikam_debug.h"
#include "templateicon.h"
#include "dwizardpage.h"
#include "dinfointerface.h"
#include "dfiledialog.h"

namespace Digikam
{

class AdvPrintWizard::Private
{
public:

    Private()
      : FONT_HEIGHT_RATIO(0.8F)
    {
        introPage            = 0;
        albumsPage           = 0;
        photoPage            = 0;
        captionPage          = 0;
        cropPage             = 0;
        cancelPrinting       = false;
        iface                = 0;
    }

    const float               FONT_HEIGHT_RATIO;

    AdvPrintIntroPage*        introPage;
    AdvPrintAlbumsPage*       albumsPage;
    AdvPrintPhotoPage*        photoPage;
    AdvPrintCaptionPage*      captionPage;
    AdvPrintCropPage*         cropPage;

    bool                      cancelPrinting;

    AdvPrintSettings*         settings;
    DInfoInterface*           iface;
};

AdvPrintWizard::AdvPrintWizard(QWidget* const parent, DInfoInterface* const iface)
    : DWizardDlg(parent, QLatin1String("PrintCreatorDialog")),
      d(new Private)
{
    setWindowTitle(i18n("Print Creator"));

    d->iface       = iface;
    d->settings    = new AdvPrintSettings;
    d->introPage   = new AdvPrintIntroPage(this,   i18n(INTRO_PAGE_NAME));
    d->albumsPage  = new AdvPrintAlbumsPage(this,  i18n("Albums Selection"));
    d->photoPage   = new AdvPrintPhotoPage(this,   i18n(PHOTO_PAGE_NAME));
    d->captionPage = new AdvPrintCaptionPage(this, i18n(CAPTION_PAGE_NAME));
    d->cropPage    = new AdvPrintCropPage(this,    i18n(CROP_PAGE_NAME));

    // -----------------------------------

    connect(this, SIGNAL(currentIdChanged(int)),
            this, SLOT(slotPageChanged(int)));

    connect(button(QWizard::CancelButton), SIGNAL(clicked()),
            this, SLOT(reject()));

    connect(d->photoPage->imagesList(), SIGNAL(signalImageListChanged()),
            d->captionPage, SLOT(slotUpdateImagesList()));
}

AdvPrintWizard::~AdvPrintWizard()
{
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
            return d->albumsPage->id();
    }
    else
    {
        if (currentPage() == d->introPage)
            return d->photoPage->id();
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

void AdvPrintWizard::parseTemplateFile(const QString& fn, const QSizeF& pageSize)
{
    QDomDocument doc(QLatin1String("mydocument"));
    qCDebug(DIGIKAM_GENERAL_LOG) << " XXX: " <<  fn;

    if (fn.isEmpty())
    {
        return;
    }

    QFile file(fn);

    if (!file.open(QIODevice::ReadOnly))
        return;

    if (!doc.setContent(&file))
    {
        file.close();
        return;
    }

    file.close();

    AdvPrintPhotoSize* p = 0;

    // print out the element names of all elements that are direct children
    // of the outermost element.
    QDomElement docElem = doc.documentElement();
    qCDebug(DIGIKAM_GENERAL_LOG) << docElem.tagName(); // the node really is an element.

    QSizeF size;
    QString unit;
    int scaleValue;
    QDomNode n = docElem.firstChild();

    while (!n.isNull())
    {
        size          = QSizeF(0, 0);
        scaleValue    = 10; // 0.1 mm
        QDomElement e = n.toElement(); // try to convert the node to an element.

        if (!e.isNull())
        {
            if (e.tagName() == QLatin1String("paper"))
            {
                size = QSizeF(e.attribute(QLatin1String("width"),  QLatin1String("0")).toFloat(),
                              e.attribute(QLatin1String("height"), QLatin1String("0")).toFloat());
                unit = e.attribute(QLatin1String("unit"), QLatin1String("mm"));

                qCDebug(DIGIKAM_GENERAL_LOG) <<  e.tagName()
                                             << QLatin1String(" name=")
                                             << e.attribute(QLatin1String("name"), QLatin1String("??"))
                                             << " size= " << size
                                             << " unit= " << unit;

                if (size == QSizeF(0.0, 0.0) && size == pageSize)
                {
                    // skipping templates without page size since pageSize is not set
                    n = n.nextSibling();
                    continue;
                }
                else if (unit != QLatin1String("mm") &&
                         size != QSizeF(0.0, 0.0))      // "cm", "inches" or "inch"
                {
                    // convert to mm
                    if (unit == QLatin1String("inches") ||
                        unit == QLatin1String("inch"))
                    {
                        size      *= 25.4;
                        scaleValue = 1000;
                        qCDebug(DIGIKAM_GENERAL_LOG) << "template size " << size << " page size " << pageSize;
                    }
                    else if (unit == QLatin1String("cm"))
                    {
                        size      *= 10;
                        scaleValue = 100;
                        qCDebug(DIGIKAM_GENERAL_LOG) << "template size " << size << " page size " << pageSize;
                    }
                    else
                    {
                        qCWarning(DIGIKAM_GENERAL_LOG) << "Wrong unit " << unit << " skipping layout";
                        n = n.nextSibling();
                        continue;
                    }
                }

                static const float round_value = 0.01F;

                if (size == QSizeF(0, 0))
                {
                    size = pageSize;
                    unit = QLatin1String("mm");
                }
                else if (pageSize     != QSizeF(0, 0) &&
                         (size.height() > (pageSize.height() + round_value) ||
                          size.width()  > (pageSize.width() + round_value)))
                {
                    qCDebug(DIGIKAM_GENERAL_LOG) << "skipping size "
                                                 << size
                                                 << " page size "
                                                 << pageSize;
                    // skipping layout it can't fit
                    n = n.nextSibling();
                    continue;
                }

                // Next templates are good
                qCDebug(DIGIKAM_GENERAL_LOG) << "layout size "
                                             << size
                                             << " page size "
                                             << pageSize;
                QDomNode np = e.firstChild();

                while (!np.isNull())
                {
                    QDomElement ep = np.toElement(); // try to convert the node to an element.

                    if (!ep.isNull())
                    {
                        if (ep.tagName() == QLatin1String("template"))
                        {
                            p = new AdvPrintPhotoSize;
                            QSizeF sizeManaged;

                            // set page size
                            if (pageSize == QSizeF(0, 0))
                            {
                                sizeManaged = size * scaleValue;
                            }
                            else if (unit == QLatin1String("inches") ||
                                     unit == QLatin1String("inch"))
                            {
                                sizeManaged = pageSize * scaleValue / 25.4;
                            }
                            else
                            {
                                sizeManaged = pageSize * 10;
                            }

                            p->layouts.append(new QRect(0,
                                                        0,
                                                        (int)sizeManaged.width(),
                                                        (int)sizeManaged.height()));

                            // create a small preview of the template
                            // TODO check if iconsize here is useless
                            TemplateIcon iconpreview(80, sizeManaged.toSize());
                            iconpreview.begin();

                            QString desktopFileName = ep.attribute(QLatin1String("name"), QLatin1String("XXX")) +
                                                                   QLatin1String(".desktop");

                            QDir dir(QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                                            QLatin1String("digikam/templates"),
                                                            QStandardPaths::LocateDirectory));
                            const QStringList list  = dir.entryList(QStringList() << desktopFileName);

                            qCDebug(DIGIKAM_GENERAL_LOG) << "Template desktop files list: "
                                                         << list;

                            QStringList::ConstIterator it  = list.constBegin();
                            QStringList::ConstIterator end = list.constEnd();

                            if (it != end)
                            {
                                p->label = KDesktopFile(dir.absolutePath() + QLatin1String("/") + *it).readName();
                            }
                            else
                            {
                                p->label = ep.attribute(QLatin1String("name"), QLatin1String("XXX"));
                                qCWarning(DIGIKAM_GENERAL_LOG) << "missed template translation "
                                                               << desktopFileName;
                            }

                            p->dpi        = ep.attribute(QLatin1String("dpi"), QLatin1String("0")).toInt();
                            p->autoRotate = (ep.attribute(QLatin1String("autorotate"), QLatin1String("false")) == QLatin1String("true")) ? true : false;
                            QDomNode nt   = ep.firstChild();

                            while (!nt.isNull())
                            {
                                QDomElement et = nt.toElement(); // try to convert the node to an element.

                                if (!et.isNull())
                                {
                                    if (et.tagName() == QLatin1String("photo"))
                                    {
                                        float value = et.attribute(QLatin1String("width"), QLatin1String("0")).toFloat();
                                        int width   = (int)((value == 0 ? size.width() : value) * scaleValue);
                                        value       = et.attribute(QLatin1String("height"), QLatin1String("0")).toFloat();
                                        int height  = (int)((value == 0 ? size.height() : value) * scaleValue);
                                        int photoX  = (int)((et.attribute(QLatin1String("x"), QLatin1String("0")).toFloat() * scaleValue));
                                        int photoY  = (int)((et.attribute(QLatin1String("y"), QLatin1String("0")).toFloat() * scaleValue));
                                        p->layouts.append(new QRect(photoX, photoY, width, height));
                                        iconpreview.fillRect(photoX, photoY, width, height, Qt::color1);
                                    }
                                    else if (et.tagName() == QLatin1String("photogrid"))
                                    {
                                        float value    = et.attribute(QLatin1String("pageWidth"), QLatin1String("0")).toFloat();
                                        int pageWidth  = (int)((value == 0 ? size.width() : value) * scaleValue);
                                        value          = et.attribute(QLatin1String("pageHeight"), QLatin1String("0")).toFloat();
                                        int pageHeight = (int)((value == 0 ? size.height() : value) * scaleValue);
                                        int rows       = et.attribute(QLatin1String("rows"), QLatin1String("0")).toInt();
                                        int columns    = et.attribute(QLatin1String("columns"), QLatin1String("0")).toInt();

                                        if (rows > 0 && columns > 0)
                                        {
                                            d->photoPage->createPhotoGrid(p,
                                                                          pageWidth,
                                                                          pageHeight,
                                                                          rows,
                                                                          columns,
                                                                          &iconpreview);
                                        }
                                        else
                                        {
                                            qCWarning(DIGIKAM_GENERAL_LOG)
                                                << " Wrong grid configuration, rows "
                                                << rows
                                                << ", columns "
                                                << columns;
                                        }
                                    }
                                    else
                                    {
                                        qCDebug(DIGIKAM_GENERAL_LOG) << "    "
                                                                     <<  et.tagName();
                                    }
                                }

                                nt = nt.nextSibling();
                            }

                            iconpreview.end();
                            p->icon = iconpreview.getIcon();
                            d->settings->photosizes.append(p);
                        }
                        else
                        {
                            qCDebug(DIGIKAM_GENERAL_LOG) << "? "
                                                         <<  ep.tagName()
                                                         << " attr="
                                                         << ep.attribute(QLatin1String("name"),
                                                                         QLatin1String("??"));
                        }
                    }

                    np = np.nextSibling();
                }
            }
            else
            {
                qCDebug(DIGIKAM_GENERAL_LOG) << "??"
                                             << e.tagName()
                                             << " name="
                                             << e.attribute(QLatin1String("name"), QLatin1String("??"));
            }
        }

        n = n.nextSibling();
    }
}

void AdvPrintWizard::initPhotoSizes(const QSizeF& pageSize)
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "New page size "
                                 << pageSize
                                 << ", old page size "
                                 << d->settings->pageSize;

    // don't refresh anything if we haven't changed page sizes.
    if (pageSize == d->settings->pageSize)
        return;

    d->settings->pageSize = pageSize;

    // cleaning pageSize memory before invoking clear()
    for (int i = 0 ; i < d->settings->photosizes.count() ; ++i)
        delete d->settings->photosizes.at(i);

    d->settings->photosizes.clear();

    // get template-files and parse them

    QDir dir(QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                    QLatin1String("digikam/templates"),
                                    QStandardPaths::LocateDirectory));
    const QStringList list = dir.entryList(QStringList() << QLatin1String("*.xml"));

    qCDebug(DIGIKAM_GENERAL_LOG) << "Template XML files list: "
                                 << list;

    foreach(const QString& fn, list)
    {
        parseTemplateFile(dir.absolutePath() + QLatin1String("/") + fn, pageSize);
    }

    qCDebug(DIGIKAM_GENERAL_LOG) << "photosizes count() ="
                                 << d->settings->photosizes.count();
    qCDebug(DIGIKAM_GENERAL_LOG) << "photosizes isEmpty() ="
                                 << d->settings->photosizes.isEmpty();

    if (d->settings->photosizes.isEmpty())
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Empty photoSize-list, create default size";

        // There is no valid page size yet.  Create a default page (B10) to prevent crashes.
        AdvPrintPhotoSize* const p = new AdvPrintPhotoSize;
        p->dpi                     = 0;
        p->autoRotate              = false;
        p->label                   = i18n("Unsupported Paper Size");
        // page size: B10 (32 x 45 mm)
        p->layouts.append(new QRect(0, 0, 3200, 4500));
        p->layouts.append(new QRect(0, 0, 3200, 4500));
        // add to the list
        d->settings->photosizes.append(p);
    }

    // load the photo sizes into the listbox
    d->photoPage->ui()->ListPhotoSizes->blockSignals(true);
    d->photoPage->ui()->ListPhotoSizes->clear();
    QList<AdvPrintPhotoSize*>::iterator it;

    for (it = d->settings->photosizes.begin() ;
         it != d->settings->photosizes.end() ; ++it)
    {
        AdvPrintPhotoSize* const s = static_cast<AdvPrintPhotoSize*>(*it);

        if (s)
        {
            QListWidgetItem* const pWItem = new QListWidgetItem(s->label);
            pWItem->setIcon(s->icon);
            d->photoPage->ui()->ListPhotoSizes->addItem(pWItem);
        }
    }

    // Adding custom choice
    QListWidgetItem* const pWItem = new QListWidgetItem(i18n(CUSTOM_PAGE_LAYOUT_NAME));

    TemplateIcon ti(80, pageSize.toSize());
    ti.begin();
    QPainter& painter = ti.getPainter();
    painter.setPen(Qt::color1);
    painter.drawText(painter.viewport(), Qt::AlignCenter, i18n("Custom layout"));
    ti.end();

    pWItem->setIcon(ti.getIcon());
    d->photoPage->ui()->ListPhotoSizes->addItem(pWItem);
    d->photoPage->ui()->ListPhotoSizes->blockSignals(false);
    d->photoPage->ui()->ListPhotoSizes->setCurrentRow(0, QItemSelectionModel::Select);
}

double AdvPrintWizard::getMaxDPI(const QList<AdvPrintPhoto*>& photos,
                                 const QList<QRect*>& layouts,
                                 int current)
{
    Q_ASSERT(layouts.count() > 1);

    QList<QRect*>::const_iterator it = layouts.begin();
    QRect* layout                    = static_cast<QRect*>(*it);
    double maxDPI                    = 0.0;

    for (; current < photos.count(); ++current)
    {
        AdvPrintPhoto* const photo = photos.at(current);
        double dpi          = ((double) photo->m_cropRegion.width() + (double) photo->m_cropRegion.height()) /
                              (((double) layout->width() / 1000.0) + ((double) layout->height() / 1000.0));

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

QRect* AdvPrintWizard::getLayout(int photoIndex) const
{
    AdvPrintPhotoSize* const s = d->settings->photosizes.at(d->photoPage->ui()->ListPhotoSizes->currentRow());

    // how many photos would actually be printed, including copies?
    int photoCount      = (photoIndex + 1);

    // how many pages?  Recall that the first layout item is the paper size
    int photosPerPage   = s->layouts.count() - 1;
    int remainder       = photoCount % photosPerPage;
    int retVal          = remainder;

    if (remainder == 0)
        retVal = photosPerPage;

    return s->layouts.at(retVal);
}

void AdvPrintWizard::printCaption(QPainter& p,
                                  AdvPrintPhoto* const photo,
                                  int captionW,
                                  int captionH,
                                  const QString& caption)
{
    // PENDING: AdvPrintPhoto* photo will be needed to add a per photo caption management
    QStringList captionByLines;

    int captionIndex = 0;

    while (captionIndex < caption.length())
    {
        QString newLine;
        bool breakLine            = false; // End Of Line found
        int currIndex;                     //  Caption QString current index

        // Check minimal lines dimension
        //TODO fix length, maybe useless
        int captionLineLocalLength = 40;

        for (currIndex = captionIndex; currIndex < caption.length() && !breakLine; ++currIndex)
        {
            if (caption[currIndex] == QLatin1Char('\n') || caption[currIndex].isSpace())
                breakLine = true;
        }

        if (captionLineLocalLength <= (currIndex - captionIndex))
            captionLineLocalLength = (currIndex - captionIndex);

        breakLine = false;

        for (currIndex = captionIndex;
             (currIndex <= captionIndex + captionLineLocalLength) && (currIndex < caption.length()) && !breakLine;
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
    font.setPixelSize((int)(captionH * d->FONT_HEIGHT_RATIO));
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
            p.translate(0, - (int)(pixelsHigh));

        QRect r(0, 0, captionW, captionH);

        p.drawText(r, Qt::AlignLeft, captionByLines[lineNumber], &r);
    }
}

bool AdvPrintWizard::paintOnePage(QPainter& p,
                                  const QList<AdvPrintPhoto*>& photos,
                                  const QList<QRect*>& layouts,
                                  int& current,
                                  bool cropDisabled,
                                  bool useThumbnails)
{
    Q_ASSERT(layouts.count() > 1);

    if (photos.count() == 0)
        return true;   // no photos => last photo

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
        destH = AdvPrintNint((double) destW * ((double) srcH / (double) srcW));

        if (destH > p.window().height())
        {
            destH = p.window().height();
            destW = AdvPrintNint((double) destH * ((double) srcW / (double) srcH));
        }
    }
    else
    {
        destW = AdvPrintNint((double) destH * ((double) srcW / (double) srcH));

        if (destW > p.window().width())
        {
            destW = p.window().width();
            destH = AdvPrintNint((double) destW * ((double) srcH / (double) srcW));
        }
    }

    double xRatio = (double) destW / (double) srcPage->width();
    double yRatio = (double) destH / (double) srcPage->height();
    int left      = (p.window().width()  - destW) / 2;
    int top       = (p.window().height() - destH) / 2;

    // FIXME: may not want to erase the background page
    p.eraseRect(left, top,
                AdvPrintNint((double) srcPage->width()  * xRatio),
                AdvPrintNint((double) srcPage->height() * yRatio));

    for (; current < photos.count(); ++current)
    {
        AdvPrintPhoto* const photo = photos.at(current);
        // crop
        QImage img;

        if (useThumbnails)
            img = photo->thumbnail().toImage();
        else
            img = photo->loadPhoto();

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
                xRatio = (double) photo->thumbnail().width() / (double) photo->width();

            if (photo->thumbnail().height() != 0)
                yRatio = (double) photo->thumbnail().height() / (double) photo->height();

            int x1 = AdvPrintNint((double) photo->m_cropRegion.left()   * xRatio);
            int y1 = AdvPrintNint((double) photo->m_cropRegion.top()    * yRatio);
            int w  = AdvPrintNint((double) photo->m_cropRegion.width()  * xRatio);
            int h  = AdvPrintNint((double) photo->m_cropRegion.height() * yRatio);
            img    = img.copy(QRect(x1, y1, w, h));
        }
        else if (!cropDisabled)
        {
            img = img.copy(photo->m_cropRegion);
        }

        int x1 = AdvPrintNint((double) layout->left() * xRatio);
        int y1 = AdvPrintNint((double) layout->top()  * yRatio);
        int w  = AdvPrintNint((double) layout->width() * xRatio);
        int h  = AdvPrintNint((double) layout->height() * yRatio);

        QRect rectViewPort    = p.viewport();
        QRect newRectViewPort = QRect(x1 + left, y1 + top, w, h);
        QSize imageSize       = img.size();

        //     qCDebug(DIGIKAM_GENERAL_LOG) << "Image         " << photo->filename << " size " << imageSize;
        //     qCDebug(DIGIKAM_GENERAL_LOG) << "viewport size " << newRectViewPort.size();

        QPoint point;

        if (cropDisabled)
        {
            imageSize.scale(newRectViewPort.size(), Qt::KeepAspectRatio);
            int spaceLeft = (newRectViewPort.width() - imageSize.width()) / 2;
            int spaceTop  = (newRectViewPort.height() - imageSize.height()) / 2;
            p.setViewport(spaceLeft + newRectViewPort.x(), spaceTop + newRectViewPort.y(), imageSize.width(), imageSize.height());
            point         = QPoint(newRectViewPort.x() + spaceLeft + imageSize.width(), newRectViewPort.y() + spaceTop + imageSize.height());
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
            photo->m_pAdvPrintCaptionInfo->m_captionType != AdvPrintCaptionInfo::NoCaptions)
        {
            p.save();
            QString caption;
            caption = d->captionPage->captionFormatter(photo);
            qCDebug(DIGIKAM_GENERAL_LOG) << "Caption " << caption ;

            // draw the text at (0,0), but we will translate and rotate the world
            // before drawing so the text will be in the correct location
            // next, do we rotate?
            int captionW        = w - 2;
            double ratio        = photo->m_pAdvPrintCaptionInfo->m_captionSize * 0.01;
            int captionH        = (int)(qMin(w, h) * ratio);
            int exifOrientation = DMetadata::ORIENTATION_NORMAL;
            int orientatation   = photo->m_rotation;

            exifOrientation     = photo->metaIface().getImageOrientation();

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
                                         << orientatation ;
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
        layout = it == layouts.end() ? 0 : static_cast<QRect*>(*it);

        if (layout == 0)
        {
            current++;
            break;
        }
    }

    // did we print the last photo?
    return (current < photos.count());
}

void AdvPrintWizard::updateCropFrame(AdvPrintPhoto* const photo, int photoIndex)
{
    AdvPrintPhotoSize* const s = d->settings->photosizes.at(d->photoPage->ui()->ListPhotoSizes->currentRow());
    d->cropPage->ui()->cropFrame->init(photo,
                                       getLayout(photoIndex)->width(),
                                       getLayout(photoIndex)->height(),
                                       s->autoRotate);
    d->cropPage->ui()->LblCropPhoto->setText(i18n("Photo %1 of %2",
                                             photoIndex + 1,
                                             QString::number(d->settings->photos.count())));
}

void AdvPrintWizard::previewPhotos()
{
    if (d->settings->photosizes.isEmpty())
        return;

    //Change cursor to waitCursor during transition
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

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
        photosPerPage = s->layouts.count() - 1;
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
    QList<AdvPrintPhoto*>::iterator it;

    for (it = d->settings->photos.begin() ;
         it != d->settings->photos.end() ; ++it)
    {
        AdvPrintPhoto* const photo = static_cast<AdvPrintPhoto*>(*it);

        if (page == d->settings->currentPreviewPage)
        {
            photo->m_cropRegion.setRect(-1, -1, -1, -1);
            photo->m_rotation = 0;
            int w             = s->layouts.at(count + 1)->width();
            int h             = s->layouts.at(count + 1)->height();
            d->cropPage->ui()->cropFrame->init(photo, w, h, s->autoRotate, false);
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
        QImage img(d->photoPage->ui()->BmpFirstPagePreview->size(),
                   QImage::Format_ARGB32_Premultiplied);
        QPainter p(&img);
        p.setCompositionMode(QPainter::CompositionMode_Clear);
        p.fillRect(img.rect(), Qt::color0);
        p.setCompositionMode(QPainter::CompositionMode_SourceOver);
        paintOnePage(p, d->settings->photos, s->layouts, current, d->cropPage->ui()->m_disableCrop->isChecked(), true);
        p.end();

        d->photoPage->ui()->BmpFirstPagePreview->clear();
        d->photoPage->ui()->BmpFirstPagePreview->setPixmap(QPixmap::fromImage(img));
        d->photoPage->ui()->LblPreview->setText(i18n("Page %1 of %2",
                                                     d->settings->currentPreviewPage + 1,
                                                     d->photoPage->getPageCount()));
    }
    else
    {
        d->photoPage->ui()->BmpFirstPagePreview->clear();
        d->photoPage->ui()->LblPreview->clear();
//       d->photoPage->ui()->BmpFirstPagePreview->setPixmap ( QPixmap() );
        d->photoPage->ui()->LblPreview->setText(i18n("Page %1 of %2", 0, 0));
    }

    manageBtnPreviewPage();
    d->photoPage->update();
    QApplication::restoreOverrideCursor();
}

void AdvPrintWizard::manageBtnPreviewPage()
{
    if (d->settings->photos.empty())
    {
        d->photoPage->ui()->BtnPreviewPageDown->setEnabled(false);
        d->photoPage->ui()->BtnPreviewPageUp->setEnabled(false);
    }
    else
    {
        d->photoPage->ui()->BtnPreviewPageDown->setEnabled(true);
        d->photoPage->ui()->BtnPreviewPageUp->setEnabled(true);

        if (d->settings->currentPreviewPage == 0)
        {
            d->photoPage->ui()->BtnPreviewPageDown->setEnabled(false);
        }

        if ((d->settings->currentPreviewPage + 1) == d->photoPage->getPageCount())
        {
            d->photoPage->ui()->BtnPreviewPageUp->setEnabled(false);
        }
    }
}

void AdvPrintWizard::slotPageChanged(int curr)
{
    QWizardPage* const current = page(curr);

    if (!current) return;

    QWizardPage* const before = visitedPages().isEmpty() ? 0 : page(visitedPages().last());

    //Change cursor to waitCursor during transition
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    if (before)
    {
        saveSettings(before->title());
        qCDebug(DIGIKAM_GENERAL_LOG) << "Previous Page: " << before->title();
    }

    qCDebug(DIGIKAM_GENERAL_LOG) << "Current Page:" << current->title();

    if (current->title() == i18n(INTRO_PAGE_NAME))
    {
        // readSettings only the first time
        if (!before)
            readSettings(current->title());
    }
    else if (current->title() == i18n(PHOTO_PAGE_NAME))
    {
        // readSettings only the first time
        if (!before)
            readSettings(current->title());

        // set to first photo
        d->photoPage->imagesList()->listView()->clear();
        QList<QUrl> list;

        qCDebug(DIGIKAM_GENERAL_LOG) << "Items: " << d->settings->photos.count();

        for (int i = 0 ; i < d->settings->photos.count() ; ++i)
        {
            AdvPrintPhoto* const pCurrentPhoto = d->settings->photos.at(i);

            if (pCurrentPhoto)
            {
                list.push_back(pCurrentPhoto->m_url);
            }
        }

        d->photoPage->imagesList()->blockSignals(true);
        d->photoPage->imagesList()->slotAddImages(list);
        d->photoPage->imagesList()->listView()->setCurrentItem(d->photoPage->imagesList()->listView()->itemAt(0, 0));
        d->photoPage->imagesList()->blockSignals(false);
        d->photoPage->ui()->LblPhotoCount->setText(QString::number(d->settings->photos.count()));

        // PhotoPage
        initPhotoSizes(d->photoPage->printer()->paperSize(QPrinter::Millimeter));
        // restore photoSize

        if (before && d->settings->savedPhotoSize == i18n(CUSTOM_PAGE_LAYOUT_NAME))
        {
            d->photoPage->ui()->ListPhotoSizes->setCurrentRow(0);
        }
        else
        {
            QList<QListWidgetItem*> list = d->photoPage->ui()->ListPhotoSizes->findItems(d->settings->savedPhotoSize, Qt::MatchExactly);

            if (list.count())
                d->photoPage->ui()->ListPhotoSizes->setCurrentItem(list[0]);
            else
                d->photoPage->ui()->ListPhotoSizes->setCurrentRow(0);
        }

        // reset preview page number
        d->settings->currentPreviewPage = 0;

        // create our photo sizes list
        previewPhotos();
    }
    else if (current->title() == i18n(CAPTION_PAGE_NAME))
    {
        // readSettings only the first time

        if (!before)
            readSettings(current->title());

        // update captions only the first time to avoid missing old changes when
        // back to this page

        if (!before)
            d->captionPage->slotUpdateCaptions();
    }
    else if (current->title() == i18n(CROP_PAGE_NAME))
    {
        readSettings(current->title());
        d->settings->currentCropPhoto = 0;

        if (d->settings->photos.size())
        {
            AdvPrintPhoto* const photo = d->settings->photos[d->settings->currentCropPhoto];
            d->cropPage->setBtnCropEnabled();
            this->update();
            updateCropFrame(photo, d->settings->currentCropPhoto);
        }
        else
        {
            // NOTE it should not pass here
            qCDebug(DIGIKAM_GENERAL_LOG) << "Not any photos selected cropping is disabled";
        }
    }

    QApplication::restoreOverrideCursor();
}

void AdvPrintWizard::saveSettings(const QString& pageName)
{
    qCDebug(DIGIKAM_GENERAL_LOG) << pageName;

    // Save the current settings
    KConfig config;
    KConfigGroup group = config.group(QLatin1String("PrintCreator"));

    if (pageName == i18n(INTRO_PAGE_NAME))
    {
        group.writeEntry("SelMode", (int)d->settings->selMode);
    }
    else if (pageName == i18n(PHOTO_PAGE_NAME))
    {
        group.writeEntry(QLatin1String("Printer"),
                         d->photoPage->ui()->m_printer_choice->currentText());

        if (d->photoPage->ui()->ListPhotoSizes->currentItem())
        {
            d->settings->savedPhotoSize = d->photoPage->ui()->ListPhotoSizes->currentItem()->text();
        }

        group.writeEntry(QLatin1String("PhotoSize"),
                         d->settings->savedPhotoSize);
        group.writeEntry(QLatin1String("IconSize"),
                         d->photoPage->ui()->ListPhotoSizes->iconSize());
    }
}

void AdvPrintWizard::readSettings(const QString& pageName)
{
    KConfig config;
    KConfigGroup group = config.group(QLatin1String("PrintCreator"));

    qCDebug(DIGIKAM_GENERAL_LOG) << pageName;

    if (pageName == i18n(INTRO_PAGE_NAME))
    {
        d->settings->selMode = (AdvPrintSettings::Selection)group.readEntry("SelMode",
                               (int)AdvPrintSettings::IMAGES);
    }
    else if (pageName == i18n(PHOTO_PAGE_NAME))
    {
        // InfoPage

        int gid = d->photoPage->ui()->m_printer_choice->findText(i18n("Print with Gimp"));

        if (d->introPage->gimpPath().isEmpty())
        {
            // Gimp is not available : we disable the option.
            d->photoPage->ui()->m_printer_choice->setItemData(gid, false, Qt::UserRole-1);
        }

        QString printerName = group.readEntry("Printer", i18n("Print to PDF"));
        int index           = d->photoPage->ui()->m_printer_choice->findText(printerName);

        if (index != -1)
        {
            d->photoPage->ui()->m_printer_choice->setCurrentIndex(index);
        }

        // init QPrinter
        d->photoPage->slotOutputChanged(d->photoPage->ui()->m_printer_choice->currentText());

        QSize iconSize = group.readEntry("IconSize", QSize(24, 24));
        d->photoPage->ui()->ListPhotoSizes->setIconSize(iconSize);

        // photo size
        d->settings->savedPhotoSize = group.readEntry("PhotoSize");
        initPhotoSizes(d->photoPage->printer()->paperSize(QPrinter::Millimeter));
    }
    else if (pageName == i18n(CAPTION_PAGE_NAME))
    {
        //caption
        d->captionPage->readCaptionSettings();

        //enable right caption stuff
        d->captionPage->slotCaptionChanged(d->captionPage->ui()->m_captions->currentText());
    }
    else if (pageName == i18n(CROP_PAGE_NAME))
    {
        // CropPage
        if (d->photoPage->ui()->m_printer_choice->currentText() == i18n("Print to JPG"))
        {
            d->cropPage->ui()->m_fileSaveBox->setEnabled(true);
        }
        else
        {
            d->cropPage->ui()->m_fileSaveBox->setEnabled(false);
        }
    }
}

void AdvPrintWizard::printPhotos(const QList<AdvPrintPhoto*>& photos,
                                 const QList<QRect*>& layouts,
                                 QPrinter& printer)
{
    d->cancelPrinting = false;
    QProgressDialog pbar(this);
    pbar.setRange(0, photos.count());
    QApplication::processEvents();

    QPainter p;
    p.begin(&printer);

    int current   = 0;
    bool printing = true;

    while (printing)
    {
        printing = paintOnePage(p,
                                photos,
                                layouts,
                                current,
                                d->cropPage->ui()->m_disableCrop->isChecked());

        if (printing)
            printer.newPage();

        pbar.setValue(current);
        QApplication::processEvents();

        if (d->cancelPrinting)
        {
            printer.abort();
            return;
        }
    }

    p.end();
}

QStringList AdvPrintWizard::printPhotosToFile(const QList<AdvPrintPhoto*>& photos,
                                              const QString& baseFilename,
                                              AdvPrintPhotoSize* const layouts)
{
    Q_ASSERT(layouts->layouts.count() > 1);

    d->cancelPrinting = false;
    QProgressDialog pbar(this);
    pbar.setRange(0, photos.count());

    QApplication::processEvents();

    int current   = 0;
    int pageCount = 1;
    bool printing = true;
    QStringList files;

    QRect* const srcPage = layouts->layouts.at(0);

    while (printing)
    {
        // make a pixmap to save to file.  Make it just big enough to show the
        // highest-dpi image on the page without losing data.
        double dpi = layouts->dpi;

        if (dpi == 0.0)
            dpi = getMaxDPI(photos, layouts->layouts, current) * 1.1;

        //int w = AdvPrintNint(srcPage->width() / 1000.0 * dpi);
        //int h = AdvPrintNint(srcPage->height()  / 1000.0 * dpi);
        int w = AdvPrintNint(srcPage->width());
        int h = AdvPrintNint(srcPage->height());

        QPixmap pixmap(w, h);
        QPainter painter;
        painter.begin(&pixmap);

        // save this page out to file
        QFileInfo fi(baseFilename);
        QString ext  = fi.completeSuffix();  // ext = ".jpeg"
        if (ext.isEmpty()) ext = QLatin1String(".jpeg");
        QString name = fi.baseName();
        QString path = fi.absolutePath();

        QString filename = path + QLatin1String("/") + name + QLatin1String("_") + QString::number(pageCount) + QLatin1String(".") + ext;
        bool saveFile    = true;

        if (QFile::exists(filename))
        {
            int result = QMessageBox::question(this, i18n("Overwrite File"),
                                               i18n("The following file will be overwritten. Are you sure you want to overwrite it?") +
                                               QLatin1String("\n\n") + filename,
                                               QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel),
                                               QMessageBox::No);

            if (result == QMessageBox::No)
            {
                saveFile = false;
            }
            else if (result == QMessageBox::Cancel)
            {
                break;
            }
        }

        printing = paintOnePage(painter,
                                photos,
                                layouts->layouts,
                                current,
                                d->cropPage->ui()->m_disableCrop->isChecked());
        painter.end();

        if (saveFile)
        {
            files.append(filename);

            if (!pixmap.save(filename, 0, 100))
            {
                QMessageBox::information(this,
                                         QString(),
                                         i18n("Could not save file, please check your output entry."));
                break;
            }
        }

        pageCount++;
        pbar.setValue(current);
        QApplication::processEvents();

        if (d->cancelPrinting)
            break;
    }

    return files;
}

void AdvPrintWizard::removeGimpFiles()
{
    for (QStringList::ConstIterator it = d->settings->gimpFiles.constBegin() ;
         it != d->settings->gimpFiles.constEnd() ; ++it)
    {
        if (QFile::exists(*it))
        {
            if (QFile::remove(*it) == false)
            {
                QMessageBox::information(this,
                                         QString(),
                                         i18n("Could not remove the GIMP's temporary files."));
                break;
            }
        }
    }
}

// this is called when Cancel is clicked.
void AdvPrintWizard::reject()
{
    d->cancelPrinting = true;

    if (d->settings->gimpFiles.count() > 0)
        removeGimpFiles();

    QDialog::reject();
}

void AdvPrintWizard::accept()
{
    if (d->settings->photos.empty())
    {
        DWizardDlg::reject();
        return;
    }

    // set the default crop regions if not already set
    AdvPrintPhotoSize* const s = d->settings->photosizes.at(d->photoPage->ui()->ListPhotoSizes->currentRow());
    QList<AdvPrintPhoto*>::iterator it;
    int i                      = 0;

    for (it = d->settings->photos.begin() ; it != d->settings->photos.end() ; ++it)
    {
        AdvPrintPhoto* const photo = static_cast<AdvPrintPhoto* >(*it);

        if (photo && photo->m_cropRegion == QRect(-1, -1, -1, -1))
        {
            d->cropPage->ui()->cropFrame->init(photo,
                                         getLayout(i)->width(),
                                         getLayout(i)->height(),
                                         s->autoRotate);
        }

        i++;
    }

    if (d->photoPage->ui()->m_printer_choice->currentText() != i18n("Print to JPG") &&
        d->photoPage->ui()->m_printer_choice->currentText() != i18n("Print with Gimp"))
    {
        // tell him again!
        d->photoPage->printer()->setFullPage(true);

        qreal left, top, right, bottom;
        d->photoPage->printer()->getPageMargins(&left, &top, &right, &bottom, QPrinter::Millimeter);

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

        QPrinter::PaperSize paperSize =  d->photoPage->printer()->paperSize();

        QPrintDialog* const dialog    = new QPrintDialog(d->photoPage->printer(), this);
        dialog->setWindowTitle(i18n("Print Creator"));

        qCDebug(DIGIKAM_GENERAL_LOG) << "(2) paper page "
                                     << dialog->printer()->paperSize()
                                     << " size "
                                     << dialog->printer()->paperSize(QPrinter::Millimeter);

        bool wantToPrint = (dialog->exec() == QDialog::Accepted);

        if (!wantToPrint)
        {
            DWizardDlg::accept();
            return;
        }

        qCDebug(DIGIKAM_GENERAL_LOG) << "(3) paper page "
                                     << dialog->printer()->paperSize()
                                     << " size "
                                     << dialog->printer()->paperSize(QPrinter::Millimeter);

        // Why paperSize changes if printer properties is not pressed?
        if (paperSize !=  d->photoPage->printer()->paperSize())
            d->photoPage->printer()->setPaperSize(paperSize);

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

        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        printPhotos(d->settings->photos, s->layouts, *d->photoPage->printer());
        QApplication::restoreOverrideCursor();
    }
    else if (d->photoPage->ui()->m_printer_choice->currentText() == i18n("Print with Gimp"))
    {
        // now output the items
        QString path = d->settings->tempPath;

        if (!AdvPrintCheckTempPath(this, path))
            return;

        path = path + QLatin1String("PrintCreator_tmp_");

        if (d->settings->gimpFiles.count() > 0)
            removeGimpFiles();

        d->settings->gimpFiles = printPhotosToFile(d->settings->photos, path, s);
        QStringList args;
        QString prog = d->introPage->gimpPath();

        for (QStringList::ConstIterator it = d->settings->gimpFiles.constBegin() ;
             it != d->settings->gimpFiles.constEnd() ; ++it)
        {
            args << (*it);
        }

        if (!AdvPrintLaunchExternalApp(prog, args))
        {
            QMessageBox::information(this, QString(),
                                     i18n("There was an error launching the external Gimp "
                                     "program. Please make sure it is properly installed."));
            return;
        }
    }
    else if (d->photoPage->ui()->m_printer_choice->currentText() == i18n("Print to JPG"))
    {
        // now output the items
        QString path = d->cropPage->outputPath();

        if (path.isEmpty())
        {
            QMessageBox::information(this, QString(), i18n("Empty output path."));
            return;
        }

        qCDebug(DIGIKAM_GENERAL_LOG) << path;
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        printPhotosToFile(d->settings->photos, path, s);
        QApplication::restoreOverrideCursor();
    }

    saveSettings(currentPage()->title());
    DWizardDlg::accept();
}

} // namespace Digikam
