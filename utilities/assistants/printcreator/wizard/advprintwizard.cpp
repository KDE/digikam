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
#include <QPageSetupDialog>
#include <QProgressDialog>
#include <QDomDocument>
#include <QContextMenuEvent>
#include <QXmlStreamWriter>
#include <QXmlStreamAttributes>
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
#include "advprintcustomdlg.h"
#include "advprintintropage.h"
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

const char* const PHOTO_PAGE_NAME         = I18N_NOOP("Select page layout");
const char* const CAPTION_PAGE_NAME       = I18N_NOOP("Caption settings");
const char* const CROP_PAGE_NAME          = I18N_NOOP("Crop and rotate photos");
const char* const CUSTOM_PAGE_LAYOUT_NAME = I18N_NOOP("Custom");

class AdvPrintWizard::Private
{
public:

    Private()
      : FONT_HEIGHT_RATIO(0.8F)
    {
        introPage            = 0;
        photoPage            = 0;
        captionPage          = 0;
        cropPage             = 0;
        infopageCurrentPhoto = 0;
        currentPreviewPage   = 0;
        currentCropPhoto     = 0;
        cancelPrinting       = false;
        pageSetupDlg         = 0;
        iface                = 0;
    }

    const float               FONT_HEIGHT_RATIO;

    AdvPrintIntroPage*        introPage;
    AdvPrintPhotoPage*        photoPage;
    AdvPrintCaptionPage*      captionPage;
    AdvPrintCropPage*         cropPage;

    int                       infopageCurrentPhoto;
    int                       currentPreviewPage;
    int                       currentCropPhoto;
    bool                      cancelPrinting;

    AdvPrintSettings*         settings;
    QPageSetupDialog*         pageSetupDlg;
    DInfoInterface*           iface;
};

AdvPrintWizard::AdvPrintWizard(QWidget* const parent, DInfoInterface* const iface)
    : DWizardDlg(parent, QLatin1String("PrintCreatorDialog")),
      d(new Private)
{
    setWindowTitle(i18n("Print Creator"));

    d->iface       = iface;
    d->settings    = new AdvPrintSettings;
    d->introPage   = new AdvPrintIntroPage(this, i18n("Welcome to Print Creator"));
    d->photoPage   = new AdvPrintPhotoPage(this, i18n(PHOTO_PAGE_NAME));
    d->captionPage = new AdvPrintCaptionPage(this, i18n(CAPTION_PAGE_NAME));
    d->cropPage    = new AdvPrintCropPage(this, i18n(CROP_PAGE_NAME));

    // -----------------------------------

    connect(this, SIGNAL(currentIdChanged(int)),
            this, SLOT(slotPageChanged(int)));

    connect(button(QWizard::CancelButton), SIGNAL(clicked()),
            this, SLOT(reject()));

    connect(d->photoPage->imagesList(), SIGNAL(signalImageListChanged()),
            d->captionPage, SLOT(slotUpdateImagesList()));

    connect(d->captionPage->imagesList(), SIGNAL(signalImageListChanged()),
            this, SLOT(slotInfoPageUpdateCaptions()));

    if (d->iface)
        setItemsList();
}

AdvPrintWizard::~AdvPrintWizard()
{
    delete d->pageSetupDlg;
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

void AdvPrintWizard::createPhotoGrid(AdvPrintPhotoSize* const p,
                                     int pageWidth,
                                     int pageHeight,
                                     int rows,
                                     int columns,
                                     TemplateIcon* const iconpreview)
{
    int MARGIN      = (int)(((double)pageWidth + (double)pageHeight) / 2.0 * 0.04 + 0.5);
    int GAP         = MARGIN / 4;
    int photoWidth  = (pageWidth  - (MARGIN * 2) - ((columns - 1) * GAP)) / columns;
    int photoHeight = (pageHeight - (MARGIN * 2) - ((rows - 1)    * GAP)) / rows;
    int row         = 0;

    for (int y = MARGIN ; row < rows && y < pageHeight - MARGIN ; y += photoHeight + GAP)
    {
        int col = 0;

        for (int x = MARGIN ; col < columns && x < pageWidth - MARGIN ; x += photoWidth + GAP)
        {
            p->layouts.append(new QRect(x, y, photoWidth, photoHeight));
            iconpreview->fillRect(x, y, photoWidth, photoHeight, Qt::color1);
            col++;
        }

        row++;
    }
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

    for (int i = 0; i < list.count(); ++i)
    {
        AdvPrintPhoto* const photo = new AdvPrintPhoto(150, d->iface);
        photo->m_url          = list[i];
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
                    qCDebug(DIGIKAM_GENERAL_LOG) << "skipping size " << size << " page size " << pageSize;
                    // skipping layout it can't fit
                    n = n.nextSibling();
                    continue;
                }

                // Next templates are good
                qCDebug(DIGIKAM_GENERAL_LOG) << "layout size " << size << " page size " << pageSize;
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
                            else if (unit == QLatin1String("inches") || unit == QLatin1String("inch"))
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

                            qCDebug(DIGIKAM_GENERAL_LOG) << "Template desktop files list: " << list;

                            QStringList::ConstIterator it  = list.constBegin();
                            QStringList::ConstIterator end = list.constEnd();

                            if (it != end)
                            {
                                p->label = KDesktopFile(dir.absolutePath() + QLatin1String("/") + *it).readName();
                            }
                            else
                            {
                                p->label = ep.attribute(QLatin1String("name"), QLatin1String("XXX"));
                                qCWarning(DIGIKAM_GENERAL_LOG) << "missed template translation " << desktopFileName;
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
                                            createPhotoGrid(p, pageWidth, pageHeight, rows, columns, &iconpreview);
                                        }
                                        else
                                        {
                                            qCWarning(DIGIKAM_GENERAL_LOG) << " Wrong grid configuration, rows " << rows << ", columns " << columns;
                                        }
                                    }
                                    else
                                    {
                                        qCDebug(DIGIKAM_GENERAL_LOG) << "    " <<  et.tagName();
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
                                                         << ep.attribute(QLatin1String("name"), QLatin1String("??"));
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
    for (int i = 0; i < d->settings->photosizes.count(); ++i)
        delete d->settings->photosizes.at(i);

    d->settings->photosizes.clear();

    // get template-files and parse them

    QDir dir(QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                    QLatin1String("digikam/templates"),
                                    QStandardPaths::LocateDirectory));
    const QStringList list = dir.entryList(QStringList() << QLatin1String("*.xml"));

    qCDebug(DIGIKAM_GENERAL_LOG) << "Template XML files list: " << list;

    foreach(const QString& fn, list)
    {
        parseTemplateFile(dir.absolutePath() + QLatin1String("/") + fn, pageSize);
    }

    qCDebug(DIGIKAM_GENERAL_LOG) << "d->settings->photosizes.count()="   << d->settings->photosizes.count();
    qCDebug(DIGIKAM_GENERAL_LOG) << "d->settings->photosizes.isEmpty()=" << d->settings->photosizes.isEmpty();

    if (d->settings->photosizes.isEmpty())
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Empty photoSize-list, create default size\n";
        // There is no valid page size yet.  Create a default page (B10) to prevent crashes.
        AdvPrintPhotoSize* const p = new AdvPrintPhotoSize;
        p->dpi              = 0;
        p->autoRotate       = false;
        p->label            = i18n("Unsupported Paper Size");
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

    for (it = d->settings->photosizes.begin() ; it != d->settings->photosizes.end() ; ++it)
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

    //TODO FREE STYLE ICON
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

int AdvPrintWizard::getPageCount() const
{
    int pageCount   = 0;
    int photoCount  =  d->settings->photos.count();

    if (photoCount > 0)
    {
        // get the selected layout
        AdvPrintPhotoSize* const s = d->settings->photosizes.at(d->photoPage->ui()->ListPhotoSizes->currentRow());

        // how many pages?  Recall that the first layout item is the paper size
        int photosPerPage   = s->layouts.count() - 1;
        int remainder       = photoCount % photosPerPage;
        int emptySlots      = 0;

        if (remainder > 0)
            emptySlots = photosPerPage - remainder;

        pageCount = photoCount / photosPerPage;

        if (emptySlots > 0)
            pageCount++;
    }

    return pageCount;
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

    qCDebug(DIGIKAM_GENERAL_LOG) << "Number of lines " << (int) captionByLines.count() ;

    // Now draw the caption
    // TODO allow printing captions  per photo and on top, bottom and vertically
    for (int lineNumber = 0 ; lineNumber < (int) captionByLines.count() ; ++lineNumber)
    {
        if (lineNumber > 0)
            p.translate(0, - (int)(pixelsHigh));

        QRect r(0, 0, captionW, captionH);
        //TODO check if ok
        p.drawText(r, Qt::AlignLeft, captionByLines[lineNumber], &r);
    }
}

QString AdvPrintWizard::captionFormatter(AdvPrintPhoto* const photo) const
{
    if (!photo->m_pAdvPrintCaptionInfo)
        return QString();

    QString format;

    switch (photo->m_pAdvPrintCaptionInfo->m_captionType)
    {
        case AdvPrintCaptionInfo::FileNames:
            format = QLatin1String("%f");
            break;
        case AdvPrintCaptionInfo::ExifDateTime:
            format = QLatin1String("%d");
            break;
        case AdvPrintCaptionInfo::Comment:
            format = QLatin1String("%c");
            break;
        case AdvPrintCaptionInfo::Custom:
            format =  photo->m_pAdvPrintCaptionInfo->m_captionText;
            break;
        default:
            qCWarning(DIGIKAM_GENERAL_LOG) << "UNKNOWN caption type "
                                           << photo->m_pAdvPrintCaptionInfo->m_captionType;
            break;
    }

    QFileInfo fi(photo->m_url.toLocalFile());
    QString resolution;
    QSize imageSize;
    DMetadata meta = photo->metaIface();

    imageSize = meta.getImageDimensions();

    if (imageSize.isValid())
    {
        resolution = QString::fromUtf8("%1x%2").arg(imageSize.width()).arg(imageSize.height());
    }

    format.replace(QLatin1String("\\n"), QLatin1String("\n"));

    // %f filename
    // %c comment
    // %d date-time
    // %t exposure time
    // %i iso
    // %r resolution
    // %a aperture
    // %l focal length

    format.replace(QString::fromUtf8("%r"), resolution);
    format.replace(QString::fromUtf8("%f"), fi.fileName());

    if (d->iface)
    {
        DItemInfo info(d->iface->itemInfo(photo->m_url));
        format.replace(QString::fromUtf8("%c"), info.comment());
        format.replace(QString::fromUtf8("%d"), QLocale().toString(info.dateTime(),
                                                QLocale::ShortFormat));
    }
    else
    {
        format.replace(QString::fromUtf8("%c"),
            meta.getImageComments()[QLatin1String("x-default")].caption);
        format.replace(QString::fromUtf8("%d"),
            QLocale().toString(meta.getImageDateTime(), QLocale::ShortFormat));
    }

    format.replace(QString::fromUtf8("%t"),
        meta.getExifTagString("Exif.Photo.ExposureTime"));
    format.replace(QString::fromUtf8("%i"),
        meta.getExifTagString("Exif.Photo.ISOSpeedRatings"));
    format.replace(QString::fromUtf8("%a"),
        meta.getExifTagString("Exif.Photo.FNumber"));
    format.replace(QString::fromUtf8("%l"),
        meta.getExifTagString("Exif.Photo.FocalLength"));

    return format;
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
            caption = captionFormatter(photo);
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

// update the pages to be printed and preview first/last pages
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

    for (it = d->settings->photos.begin() ; it != d->settings->photos.end() ; ++it)
    {
        AdvPrintPhoto* const photo = static_cast<AdvPrintPhoto*>(*it);

        if (page == d->currentPreviewPage)
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
            if (page == d->currentPreviewPage)
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
        //p.setCompositionMode(QPainter::CompositionMode_Destination );
        p.fillRect(img.rect(), Qt::color0); //Qt::transparent );
        p.setCompositionMode(QPainter::CompositionMode_SourceOver);
        paintOnePage(p, d->settings->photos, s->layouts, current, d->cropPage->ui()->m_disableCrop->isChecked(), true);
        p.end();

        d->photoPage->ui()->BmpFirstPagePreview->clear();
        d->photoPage->ui()->BmpFirstPagePreview->setPixmap(QPixmap::fromImage(img));
        d->photoPage->ui()->LblPreview->setText(i18n("Page %1 of %2", d->currentPreviewPage + 1, getPageCount()));
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

        if (d->currentPreviewPage == 0)
        {
            d->photoPage->ui()->BtnPreviewPageDown->setEnabled(false);
        }

        if ((d->currentPreviewPage + 1) == getPageCount())
        {
            d->photoPage->ui()->BtnPreviewPageUp->setEnabled(false);
        }
    }
}

void AdvPrintWizard::setCaptionButtons()
{
    if (d->settings->photos.size())
    {
        d->captionPage->setCaptionButtons(d->settings->photos.at(d->infopageCurrentPhoto));
    }
}

void AdvPrintWizard::slotXMLCustomElement(QXmlStreamWriter& xmlWriter)
{
    xmlWriter.writeStartElement(QLatin1String("pa_layout"));
    xmlWriter.writeAttribute(QLatin1String("Printer"),   d->photoPage->ui()->m_printer_choice->currentText());
    xmlWriter.writeAttribute(QLatin1String("PageSize"),  QString::fromUtf8("%1").arg(d->photoPage->printer()->paperSize()));
    xmlWriter.writeAttribute(QLatin1String("PhotoSize"), d->photoPage->ui()->ListPhotoSizes->currentItem()->text());
    xmlWriter.writeEndElement(); // pa_layout
}

void AdvPrintWizard::slotXMLSaveItem(QXmlStreamWriter& xmlWriter, int itemIndex)
{
    if (d->settings->photos.size())
    {
        AdvPrintPhoto* const pPhoto = d->settings->photos[itemIndex];
        // TODO: first and copies could be removed since they are not useful any more
        xmlWriter.writeAttribute(QLatin1String("first"),
                                 QString::fromUtf8("%1")
                                 .arg(pPhoto->m_first));

        xmlWriter.writeAttribute(QLatin1String("copies"),
                                 QString::fromUtf8("%1")
                                 .arg(pPhoto->m_first ? pPhoto->m_copies : 0));

        // additional info (caption... etc)
        if (pPhoto->m_pAdvPrintCaptionInfo)
        {
            xmlWriter.writeStartElement(QLatin1String("pa_caption"));
            xmlWriter.writeAttribute(QLatin1String("type"),
                                     QString::fromUtf8("%1").arg(pPhoto->m_pAdvPrintCaptionInfo->m_captionType));
            xmlWriter.writeAttribute(QLatin1String("font"),
                                     pPhoto->m_pAdvPrintCaptionInfo->m_captionFont.toString());
            xmlWriter.writeAttribute(QLatin1String("size"),
                                     QString::fromUtf8("%1").arg(pPhoto->m_pAdvPrintCaptionInfo->m_captionSize));
            xmlWriter.writeAttribute(QLatin1String("color"),
                                     pPhoto->m_pAdvPrintCaptionInfo->m_captionColor.name());
            xmlWriter.writeAttribute(QLatin1String("text"),
                                     pPhoto->m_pAdvPrintCaptionInfo->m_captionText);
            xmlWriter.writeEndElement(); // pa_caption
        }
    }
}

void AdvPrintWizard::slotXMLCustomElement(QXmlStreamReader& xmlReader)
{
    qCDebug(DIGIKAM_GENERAL_LOG) << " invoked " << xmlReader.name();

    while (!xmlReader.atEnd())
    {
        if (xmlReader.isStartElement() && xmlReader.name() == QLatin1String("pa_layout"))
        {
            bool ok;
            QXmlStreamAttributes attrs = xmlReader.attributes();
            // get value of each attribute from QXmlStreamAttributes
            QStringRef attr            = attrs.value(QLatin1String("Printer"));

            if (!attr.isEmpty())
            {
                qCDebug(DIGIKAM_GENERAL_LOG) <<  " found " << attr.toString();
                int index = d->photoPage->ui()->m_printer_choice->findText(attr.toString());

                if (index != -1)
                {
                    d->photoPage->ui()->m_printer_choice->setCurrentIndex(index);
                }

                d->photoPage->slotOutputChanged(d->photoPage->ui()->m_printer_choice->currentText());
            }

            attr = attrs.value(QLatin1String("PageSize"));

            if (!attr.isEmpty())
            {
                qCDebug(DIGIKAM_GENERAL_LOG) <<  " found " << attr.toString();
                QPrinter::PaperSize paperSize = (QPrinter::PaperSize)attr.toString().toInt(&ok);
                d->photoPage->printer()->setPaperSize(paperSize);
            }

            attr = attrs.value(QLatin1String("PhotoSize"));

            if (!attr.isEmpty())
            {
                qCDebug(DIGIKAM_GENERAL_LOG) <<  " found " << attr.toString();
                d->settings->savedPhotoSize = attr.toString();
            }
        }

        xmlReader.readNext();
    }

    // reset preview page number
    d->currentPreviewPage      = 0;
    initPhotoSizes(d->photoPage->printer()->paperSize(QPrinter::Millimeter));
    QList<QListWidgetItem*> list = d->photoPage->ui()->ListPhotoSizes->findItems(d->settings->savedPhotoSize, Qt::MatchExactly);

    if (list.count())
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << " PhotoSize " << list[0]->text();
        d->photoPage->ui()->ListPhotoSizes->setCurrentItem(list[0]);
    }
    else
    {
        d->photoPage->ui()->ListPhotoSizes->setCurrentRow(0);
    }

    previewPhotos();
}

void AdvPrintWizard::slotXMLLoadElement(QXmlStreamReader& xmlReader)
{
    if (d->settings->photos.size())
    {
        // read image is the last.
        AdvPrintPhoto* const pPhoto = d->settings->photos[d->settings->photos.size()-1];
        qCDebug(DIGIKAM_GENERAL_LOG) << " invoked " << xmlReader.name();

        while (xmlReader.readNextStartElement())
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << pPhoto->m_url << " " << xmlReader.name();

            if (xmlReader.name() == QLatin1String("pa_caption"))
            {
                //useless this item has been added now
                if (pPhoto->m_pAdvPrintCaptionInfo)
                    delete pPhoto->m_pAdvPrintCaptionInfo;

                pPhoto->m_pAdvPrintCaptionInfo = new AdvPrintCaptionInfo();
                // get all attributes and its value of a tag in attrs variable.
                QXmlStreamAttributes attrs = xmlReader.attributes();
                // get value of each attribute from QXmlStreamAttributes
                QStringRef attr      = attrs.value(QLatin1String("type"));
                bool ok;

                if (!attr.isEmpty())
                {
                    qCDebug(DIGIKAM_GENERAL_LOG) <<  " found " << attr.toString();
                    pPhoto->m_pAdvPrintCaptionInfo->m_captionType =
                        (AdvPrintCaptionInfo::AvailableCaptions)attr.toString().toInt(&ok);
                }

                attr = attrs.value(QLatin1String("font"));

                if (!attr.isEmpty())
                {
                    qCDebug(DIGIKAM_GENERAL_LOG) <<  " found " << attr.toString();
                    pPhoto->m_pAdvPrintCaptionInfo->m_captionFont.fromString(attr.toString());
                }

                attr = attrs.value(QLatin1String("color"));

                if (!attr.isEmpty())
                {
                    qCDebug(DIGIKAM_GENERAL_LOG) <<  " found " << attr.toString();
                    pPhoto->m_pAdvPrintCaptionInfo->m_captionColor.setNamedColor(attr.toString());
                }

                attr = attrs.value(QLatin1String("size"));

                if (!attr.isEmpty())
                {
                    qCDebug(DIGIKAM_GENERAL_LOG) <<  " found " << attr.toString();
                    pPhoto->m_pAdvPrintCaptionInfo->m_captionSize = attr.toString().toInt(&ok);
                }

                attr = attrs.value(QLatin1String("text"));

                if (!attr.isEmpty())
                {
                    qCDebug(DIGIKAM_GENERAL_LOG) <<  " found " << attr.toString();
                    pPhoto->m_pAdvPrintCaptionInfo->m_captionText = attr.toString();
                }

                setCaptionButtons();
            }
        }
    }
}

void AdvPrintWizard::slotContextMenuRequested()
{
    if (d->settings->photos.size())
    {
        int itemIndex         = d->photoPage->imagesList()->listView()->currentIndex().row();
        d->photoPage->imagesList()->listView()->blockSignals(true);
        QMenu menu(d->photoPage->imagesList()->listView());
        QAction* const action = menu.addAction(i18n("Add again"));

        connect(action, SIGNAL(triggered()),
                this , SLOT(slotIncreaseCopies()));

        AdvPrintPhoto* const pPhoto  = d->settings->photos[itemIndex];

        qCDebug(DIGIKAM_GENERAL_LOG) << " copies "
                                     << pPhoto->m_copies
                                     << " first "
                                     << pPhoto->m_first;

        if (pPhoto->m_copies > 1 || !pPhoto->m_first)
        {
            QAction* const actionr = menu.addAction(i18n("Remove"));

            connect(actionr, SIGNAL(triggered()),
                    this, SLOT(slotDecreaseCopies()));
        }

        menu.exec(QCursor::pos());
        d->photoPage->imagesList()->listView()->blockSignals(false);
    }
}

void AdvPrintWizard::slotImageSelected(QTreeWidgetItem* item)
{
    DImagesListViewItem* const l_item = dynamic_cast<DImagesListViewItem*>(item);

    if (!l_item)
        return;

    int itemIndex = d->photoPage->imagesList()->listView()->indexFromItem(l_item).row();

    qCDebug(DIGIKAM_GENERAL_LOG) << " current row now is " << itemIndex;
    d->infopageCurrentPhoto = itemIndex;

    setCaptionButtons();
}

void AdvPrintWizard::slotDecreaseCopies()
{
    if (d->settings->photos.size())
    {
        DImagesListViewItem* const item = dynamic_cast<DImagesListViewItem*>
            (d->photoPage->imagesList()->listView()->currentItem());

        if (!item)
            return;

        qCDebug(DIGIKAM_GENERAL_LOG) << " Removing a copy of " << item->url();
        d->photoPage->imagesList()->slotRemoveItems();
    }
}

void AdvPrintWizard::slotRemovingItem(int itemIndex)
{
    if (d->settings->photos.size() && itemIndex >= 0)
    {
        /// Debug data: found and copies
        bool found = false;
        int copies = 0;

        d->photoPage->imagesList()->blockSignals(true);
        AdvPrintPhoto* const pPhotoToRemove = d->settings->photos.at(itemIndex);

        // photo to be removed could be:
        // 1) unique => just remove it
        // 2) first of n, =>
        //    search another with the same url
        //    and set it a first and with a count to n-1 then remove it
        // 3) one of n, search the first one and set count to n-1 then remove it
        if (pPhotoToRemove && pPhotoToRemove->m_first)
        {
            if (pPhotoToRemove->m_copies > 0)
            {
                for (int i = 0 ; i < d->settings->photos.count() && !found ; ++i)
                {
                    AdvPrintPhoto* const pCurrentPhoto = d->settings->photos.at(i);

                    if (pCurrentPhoto && pCurrentPhoto->m_url == pPhotoToRemove->m_url)
                    {
                        pCurrentPhoto->m_copies = pPhotoToRemove->m_copies - 1;
                        copies                  = pCurrentPhoto->m_copies;
                        pCurrentPhoto->m_first  = true;
                        found                   = true;
                    }
                }
            }
            // otherwise it's unique
        }
        else if (pPhotoToRemove)
        {
            for (int i = 0 ; i < d->settings->photos.count() && !found ; ++i)
            {
                AdvPrintPhoto* const pCurrentPhoto = d->settings->photos.at(i);

                if (pCurrentPhoto &&
                    pCurrentPhoto->m_url == pPhotoToRemove->m_url &&
                    pCurrentPhoto->m_first)
                {
                    pCurrentPhoto->m_copies--;
                    copies = pCurrentPhoto->m_copies;
                    found  = true;
                }
            }
        }
        else
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << " NULL AdvPrintPhoto object ";
            return;
        }

        if (pPhotoToRemove)
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Removed fileName: "
                                         << pPhotoToRemove->m_url.fileName()
                                         << " copy number "
                                         << copies;
        }

        d->settings->photos.removeAt(itemIndex);
        delete pPhotoToRemove;

        d->photoPage->imagesList()->blockSignals(false);
        previewPhotos();
    }

    if (d->settings->photos.empty())
    {
        // No photos => disabling next button (e.g. crop page)
        d->photoPage->setComplete(false);
    }
}

void AdvPrintWizard::slotAddItems(const QList<QUrl>& list)
{
    if (list.count() == 0)
    {
        return;
    }

    QList<QUrl> urls;
    d->photoPage->imagesList()->blockSignals(true);

    for (QList<QUrl>::ConstIterator it = list.constBegin() ; it != list.constEnd() ; ++it)
    {
        QUrl imageUrl = *it;

        // Check if the new item already exist in the list.
        bool found    = false;

        for (int i = 0 ; i < d->settings->photos.count() && !found ; ++i)
        {
            AdvPrintPhoto* const pCurrentPhoto = d->settings->photos.at(i);

            if (pCurrentPhoto &&
                pCurrentPhoto->m_url == imageUrl &&
                pCurrentPhoto->m_first)
            {
                pCurrentPhoto->m_copies++;
                found                       = true;
                AdvPrintPhoto* const pPhoto = new AdvPrintPhoto(*pCurrentPhoto);
                pPhoto->m_first             = false;
                d->settings->photos.append(pPhoto);
                qCDebug(DIGIKAM_GENERAL_LOG) << "Added fileName: "
                                             << pPhoto->m_url.fileName()
                                             << " copy number "
                                             << pCurrentPhoto->m_copies;
            }
        }

        if (!found)
        {
            AdvPrintPhoto* const pPhoto = new AdvPrintPhoto(150, d->iface);
            pPhoto->m_url          = *it;
            pPhoto->m_first             = true;
            d->settings->photos.append(pPhoto);
            qCDebug(DIGIKAM_GENERAL_LOG) << "Added new fileName: "
                                         << pPhoto->m_url.fileName();
        }
    }

    d->photoPage->imagesList()->blockSignals(false);

    if (d->settings->photos.size())
    {
        d->photoPage->setComplete(true);
    }
}

void AdvPrintWizard::slotIncreaseCopies()
{
    if (d->settings->photos.size())
    {
        QList<QUrl> list;
        DImagesListViewItem* const item = dynamic_cast<DImagesListViewItem*>(d->photoPage->imagesList()->listView()->currentItem());

        if (!item)
            return;

        list.append(item->url());
        qCDebug(DIGIKAM_GENERAL_LOG) << " Adding a copy of " << item->url();
        d->photoPage->imagesList()->slotAddImages(list);
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

    if (current->title() == i18n(PHOTO_PAGE_NAME))
    {
        // readSettings only the first time
        if (!before)
            readSettings(current->title());

        // set to first photo
        d->infopageCurrentPhoto = 0;
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
        d->currentPreviewPage = 0;

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
            slotInfoPageUpdateCaptions();
    }
    else if (current->title() == i18n(CROP_PAGE_NAME))
    {
        readSettings(current->title());
        d->currentCropPhoto = 0;

        if (d->settings->photos.size())
        {
            AdvPrintPhoto* const photo = d->settings->photos[d->currentCropPhoto];
            setBtnCropEnabled();
            this->update();
            updateCropFrame(photo, d->currentCropPhoto);
        }
        else
        {
            // NOTE it should not pass here
            qCDebug(DIGIKAM_GENERAL_LOG) << "Not any photos selected cropping is disabled";
        }
    }

    QApplication::restoreOverrideCursor();
}

void AdvPrintWizard::updateCaption(AdvPrintPhoto* pPhoto)
{
    if (pPhoto)
    {
        if (!pPhoto->m_pAdvPrintCaptionInfo &&
            d->captionPage->ui()->m_captions->currentIndex() != AdvPrintCaptionInfo::NoCaptions)
        {
            pPhoto->m_pAdvPrintCaptionInfo = new AdvPrintCaptionInfo();
        }
        else if (pPhoto->m_pAdvPrintCaptionInfo &&
                 d->captionPage->ui()->m_captions->currentIndex() == AdvPrintCaptionInfo::NoCaptions)
        {
            delete pPhoto->m_pAdvPrintCaptionInfo;
            pPhoto->m_pAdvPrintCaptionInfo = NULL;
        }

        if (pPhoto->m_pAdvPrintCaptionInfo)
        {
            pPhoto->m_pAdvPrintCaptionInfo->m_captionColor = d->captionPage->ui()->m_font_color->color();
            pPhoto->m_pAdvPrintCaptionInfo->m_captionSize  = d->captionPage->ui()->m_font_size->value();
            pPhoto->m_pAdvPrintCaptionInfo->m_captionFont  = d->captionPage->ui()->m_font_name->currentFont();
            pPhoto->m_pAdvPrintCaptionInfo->m_captionType  = (AdvPrintCaptionInfo::AvailableCaptions)d->captionPage->ui()->m_captions->currentIndex();
            pPhoto->m_pAdvPrintCaptionInfo->m_captionText  = d->captionPage->ui()->m_FreeCaptionFormat->text();

            qCDebug(DIGIKAM_GENERAL_LOG) << "Update caption properties for" << pPhoto->m_url;
        }
    }
}

void AdvPrintWizard::slotInfoPageUpdateCaptions()
{
    if (d->settings->photos.size())
    {
        foreach(AdvPrintPhoto* const pPhoto, d->settings->photos)
        {
            updateCaption(pPhoto);

            if (pPhoto && pPhoto->m_pAdvPrintCaptionInfo)
            {
                DImagesListViewItem* const lvItem = d->captionPage->imagesList()->listView()->findItem(pPhoto->m_url);

                if (lvItem)
                {
                    QString cap;

                    if (pPhoto->m_pAdvPrintCaptionInfo->m_captionType != AdvPrintCaptionInfo::NoCaptions)
                        cap = captionFormatter(pPhoto);

                    lvItem->setText(DImagesListView::User1, cap);
                }
            }
        }
    }

    // create our photo sizes list
    previewPhotos();
}

void AdvPrintWizard::slotBtnCropRotateLeftClicked()
{
    // by definition, the cropRegion should be set by now,
    // which means that after our rotation it will become invalid,
    // so we will initialize it to -2 in an awful hack (this
    // tells the cropFrame to reset the crop region, but don't
    // automagically rotate the image to fit.
    AdvPrintPhoto* const photo = d->settings->photos[d->currentCropPhoto];
    photo->m_cropRegion        = QRect(-2, -2, -2, -2);
    photo->m_rotation          = (photo->m_rotation - 90) % 360;

    updateCropFrame(photo, d->currentCropPhoto);
}

void AdvPrintWizard::slotBtnCropRotateRightClicked()
{
    // by definition, the cropRegion should be set by now,
    // which means that after our rotation it will become invalid,
    // so we will initialize it to -2 in an awful hack (this
    // tells the cropFrame to reset the crop region, but don't
    // automagically rotate the image to fit.
    AdvPrintPhoto* const photo = d->settings->photos[d->currentCropPhoto];
    photo->m_cropRegion        = QRect(-2, -2, -2, -2);
    photo->m_rotation          = (photo->m_rotation + 90) % 360;

    updateCropFrame(photo, d->currentCropPhoto);
}

void AdvPrintWizard::setBtnCropEnabled()
{
    if (d->currentCropPhoto == 0)
        d->cropPage->ui()->BtnCropPrev->setEnabled(false);
    else
        d->cropPage->ui()->BtnCropPrev->setEnabled(true);

    if (d->currentCropPhoto == (int) d->settings->photos.count() - 1)
        d->cropPage->ui()->BtnCropNext->setEnabled(false);
    else
        d->cropPage->ui()->BtnCropNext->setEnabled(true);
}

void AdvPrintWizard::slotBtnCropNextClicked()
{
    AdvPrintPhoto* const photo = d->settings->photos[++d->currentCropPhoto];
    setBtnCropEnabled();

    if (!photo)
    {
        d->currentCropPhoto = d->settings->photos.count() - 1;
        return;
    }

    updateCropFrame(photo, d->currentCropPhoto);
}

void AdvPrintWizard::slotBtnCropPrevClicked()
{
    AdvPrintPhoto* const photo = d->settings->photos[--d->currentCropPhoto];

    setBtnCropEnabled();

    if (!photo)
    {
        d->currentCropPhoto = 0;
        return;
    }

    updateCropFrame(photo, d->currentCropPhoto);
}

void AdvPrintWizard::slotBtnPrintOrderUpClicked()
{
    d->photoPage->imagesList()->blockSignals(true);
    int currentIndex = d->photoPage->imagesList()->listView()->currentIndex().row();

    qCDebug(DIGIKAM_GENERAL_LOG) << "Moved photo "
                                 << currentIndex
                                 << " to  "
                                 << currentIndex + 1;

    d->settings->photos.swap(currentIndex, currentIndex + 1);
    d->photoPage->imagesList()->blockSignals(false);
    previewPhotos();
}

void AdvPrintWizard::slotListPhotoSizesSelected()
{
    AdvPrintPhotoSize* s = NULL;
    QSizeF size, sizeManaged;

    // TODO FREE STYLE
    // check if layout is managed by templates or free one
    // get the selected layout
    int curr              = d->photoPage->ui()->ListPhotoSizes->currentRow();
    QListWidgetItem* item = d->photoPage->ui()->ListPhotoSizes->item(curr);

    // if custom page layout we launch a dialog to choose what kind
    if (item->text() == i18n(CUSTOM_PAGE_LAYOUT_NAME))
    {
        // check if a custom layout has already been added
        if (curr >= 0 && curr < d->settings->photosizes.size())
        {
            s = d->settings->photosizes.at(curr);
            d->settings->photosizes.removeAt(curr);
            delete s;
            s = NULL;
        }

        AdvPrintCustomLayoutDlg custDlg(this);
        custDlg.readSettings();
        custDlg.exec();
        custDlg.saveSettings();

        // get parameters from dialog
        size           = d->settings->pageSize;
        int scaleValue = 10; // 0.1 mm

        // convert to mm
        if (custDlg.m_photoUnits->currentText() == i18n("inches"))
        {
            size       /= 25.4;
            scaleValue  = 1000;
        }
        else if (custDlg.m_photoUnits->currentText() == i18n("cm"))
        {
            size       /= 10;
            scaleValue  = 100;
        }

        sizeManaged = size * scaleValue;

        s = new AdvPrintPhotoSize;
        TemplateIcon iconpreview(80, sizeManaged.toSize());
        iconpreview.begin();

        if (custDlg.m_photoGridCheck->isChecked())
        {
            // custom photo grid
            int rows       = custDlg.m_gridRows->value();
            int columns    = custDlg.m_gridColumns->value();

            s->layouts.append(new QRect(0, 0, (int)sizeManaged.width(), (int)sizeManaged.height()));
            s->autoRotate  = custDlg.m_autorotate->isChecked();
            s->label       = item->text();
            s->dpi         = 0;

            int pageWidth  = (int)(size.width()) * scaleValue;
            int pageHeight = (int)(size.height()) * scaleValue;
            createPhotoGrid(s, pageWidth, pageHeight, rows, columns, &iconpreview);
        }
        else if (custDlg.m_fitAsManyCheck->isChecked())
        {
            int width  = custDlg.m_photoWidth->value();
            int height = custDlg.m_photoHeight->value();

            //photo size must be less than page size
            static const float round_value = 0.01F;

            if ((height > (size.height() + round_value) || width  > (size.width() + round_value)))
            {
                qCDebug(DIGIKAM_GENERAL_LOG) << "photo size " << QSize(width, height) << "> page size " << size;
                delete s;
                s = NULL;
            }
            else
            {
                // fit as many photos of given size as possible
                s->layouts.append(new QRect(0, 0, (int)sizeManaged.width(), (int)sizeManaged.height()));
                s->autoRotate  = custDlg.m_autorotate->isChecked();
                s->label       = item->text();
                s->dpi         = 0;
                int nColumns   = int(size.width()  / width);
                int nRows      = int(size.height() / height);
                int spareWidth = int(size.width())  % width;

                // check if there's no room left to separate photos
                if (nColumns > 1 &&  spareWidth == 0)
                {
                    nColumns  -= 1;
                    spareWidth = width;
                }

                int spareHeight = int(size.height()) % height;

                // check if there's no room left to separate photos
                if (nRows > 1 && spareHeight == 0)
                {
                    nRows      -= 1;
                    spareHeight = height;
                }

                if (nRows > 0 && nColumns > 0)
                {
                    // n photos => dx1, photo1, dx2, photo2,... photoN, dxN+1
                    int dx      = spareWidth  * scaleValue / (nColumns + 1);
                    int dy      = spareHeight * scaleValue / (nRows + 1);
                    int photoX  = 0;
                    int photoY  = 0;
                    width      *= scaleValue;
                    height     *= scaleValue;

                    for (int row = 0 ; row < nRows ; ++row)
                    {
                        photoY = dy * (row + 1) + (row * height);

                        for (int col = 0 ; col < nColumns ; ++col)
                        {
                            photoX = dx * (col + 1) + (col * width);
                            qCDebug(DIGIKAM_GENERAL_LOG) << "photo at P(" << photoX << ", " << photoY << ") size(" << width << ", " << height;

                            s->layouts.append(new QRect(photoX, photoY,
                                                        width, height));
                            iconpreview.fillRect(photoX, photoY, width, height, Qt::color1);
                        }
                    }
                }
                else
                {
                    qCDebug(DIGIKAM_GENERAL_LOG) << "I can't go on, rows " << nRows << "> columns " << nColumns;
                    delete s;
                    s = NULL;
                }
            }
        }
        else
        {
            // Atckin's layout
        }

        // TODO not for Atckin's layout
        iconpreview.end();

        if (s)
        {
            s->icon = iconpreview.getIcon();
            d->settings->photosizes.append(s);
        }
    }
    else
    {
        s = d->settings->photosizes.at(curr);
    }

    if (!s)
    {
        // change position to top
        d->photoPage->ui()->ListPhotoSizes->blockSignals(true);
        d->photoPage->ui()->ListPhotoSizes->setCurrentRow(0, QItemSelectionModel::Select);
        d->photoPage->ui()->ListPhotoSizes->blockSignals(false);
    }

    // reset preview page number
    d->currentPreviewPage = 0;
    previewPhotos();
}

void AdvPrintWizard::slotBtnPrintOrderDownClicked()
{
    d->photoPage->imagesList()->blockSignals(true);
    int currentIndex = d->photoPage->imagesList()->listView()->currentIndex().row();

    qCDebug(DIGIKAM_GENERAL_LOG) << "Moved photo "
                                 << currentIndex - 1
                                 << " to  "
                                 << currentIndex;

    d->settings->photos.swap(currentIndex, currentIndex - 1);
    d->photoPage->imagesList()->blockSignals(false);
    previewPhotos();
}

void AdvPrintWizard::slotBtnPreviewPageDownClicked()
{
    if (d->currentPreviewPage == 0)
        return;

    d->currentPreviewPage--;
    previewPhotos();
}

void AdvPrintWizard::slotBtnPreviewPageUpClicked()
{
    if (d->currentPreviewPage == getPageCount() - 1)
        return;

    d->currentPreviewPage++;
    previewPhotos();
}

void AdvPrintWizard::saveSettings(const QString& pageName)
{
    qCDebug(DIGIKAM_GENERAL_LOG) << pageName;

    // Save the current settings
    KConfig config;
    KConfigGroup group = config.group(QLatin1String("PrintCreator"));

    if (pageName == i18n(PHOTO_PAGE_NAME))
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

    if (pageName == i18n(PHOTO_PAGE_NAME))
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

void AdvPrintWizard::slotPageSetupDialogExit()
{
    QPrinter* const printer = d->pageSetupDlg->printer();

    qCDebug(DIGIKAM_GENERAL_LOG) << "Dialog exit, new size "
                                 << printer->paperSize(QPrinter::Millimeter)
                                 << " internal size "
                                 << d->photoPage->printer()->paperSize(QPrinter::Millimeter);

    qreal left, top, right, bottom;
    d->photoPage->printer()->getPageMargins(&left, &top, &right, &bottom, QPrinter::Millimeter);

    qCDebug(DIGIKAM_GENERAL_LOG) << "Dialog exit, new margins: left "
                                 << left
                                 << " right "
                                 << right
                                 << " top "
                                 << top
                                 << " bottom "
                                 << bottom;

    // next should be useless invoke once changing wizard page
    //initPhotoSizes ( d->photoPage->printer().paperSize(QPrinter::Millimeter));

    //     d->settings->pageSize = d->photoPage->printer().paperSize(QPrinter::Millimeter);
#ifdef DEBUG
    qCDebug(DIGIKAM_GENERAL_LOG) << " dialog exited num of copies: "
                                 << printer->numCopies()
                                 << " inside:   "
                                 << d->photoPage->printer()->numCopies();

    qCDebug(DIGIKAM_GENERAL_LOG) << " dialog exited from : "
                                 << printer->fromPage()
                                 << " to:   "
                                 << d->photoPage->printer()->toPage();
#endif
}

void AdvPrintWizard::slotPagesetupclicked()
{
    delete d->pageSetupDlg;
    d->pageSetupDlg = new QPageSetupDialog(d->photoPage->printer(), this);

    // TODO next line should work but it doesn't because of a QT bug
    //d->pageSetupDlg->open(this, SLOT(slotPageSetupDialogExit()));
    int ret   = d->pageSetupDlg->exec();

    if (ret == QDialog::Accepted)
    {
        slotPageSetupDialogExit();
    }

    // Fix the page size dialog and preview PhotoPage
    initPhotoSizes(d->photoPage->printer()->paperSize(QPrinter::Millimeter));

    // restore photoSize
    if (d->settings->savedPhotoSize == i18n(CUSTOM_PAGE_LAYOUT_NAME))
    {
        d->photoPage->ui()->ListPhotoSizes->setCurrentRow(0);
    }
    else
    {
        QList<QListWidgetItem*> list =
            d->photoPage->ui()->ListPhotoSizes->findItems(d->settings->savedPhotoSize,
                                                          Qt::MatchExactly);

        if (list.count())
            d->photoPage->ui()->ListPhotoSizes->setCurrentItem(list[0]);
        else
            d->photoPage->ui()->ListPhotoSizes->setCurrentRow(0);
    }

    // create our photo sizes list
    previewPhotos();
}

} // namespace Digikam
