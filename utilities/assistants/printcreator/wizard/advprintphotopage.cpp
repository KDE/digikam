/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-05-25
 * Description : a tool to print images
 *
 * Copyright (C) 2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "advprintphotopage.h"

// Qt includes

#include <QIcon>
#include <QPrinter>
#include <QPrinterInfo>
#include <QWidget>
#include <QApplication>
#include <QStyle>
#include <QMenu>
#include <QAction>
#include <QPageSetupDialog>
#include <QXmlStreamWriter>
#include <QXmlStreamAttributes>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "advprintwizard.h"
#include "advprintcustomdlg.h"
#include "templateicon.h"

namespace Digikam
{

class AdvPrintPhotoPage::Private
{
public:

    template <class Ui_Class>

    class WizardUI : public QWidget, public Ui_Class
    {
    public:

        WizardUI(QWidget* const parent)
            : QWidget(parent)
        {
            this->setupUi(this);
        }
    };

    typedef WizardUI<Ui_AdvPrintPhotoPage> PhotoUI;

public:

    Private(QWizard* const dialog)
      : pageSetupDlg(0),
        printer(0),
        wizard(0),
        settings(0),
        iface(0)
    {
        photoUi = new PhotoUI(dialog);
        wizard  = dynamic_cast<AdvPrintWizard*>(dialog);

        if (wizard)
        {
            settings = wizard->settings();
            iface    = wizard->iface();
        }
    }

    PhotoUI*            photoUi;
    QPageSetupDialog*   pageSetupDlg;
    QPrinter*           printer;
    QList<QPrinterInfo> printerList;

    AdvPrintWizard*     wizard;
    AdvPrintSettings*   settings;
    DInfoInterface*     iface;
};

AdvPrintPhotoPage::AdvPrintPhotoPage(QWizard* const wizard, const QString& title)
    : DWizardPage(wizard, title),
      d(new Private(wizard))
{
    d->photoUi->BtnPreviewPageUp->setIcon(QIcon::fromTheme(QLatin1String("go-next"))
                                            .pixmap(16, 16));
    d->photoUi->BtnPreviewPageDown->setIcon(QIcon::fromTheme(QLatin1String("go-previous"))
                                              .pixmap(16, 16));

    d->printerList = QPrinterInfo::availablePrinters();

    qCDebug(DIGIKAM_GENERAL_LOG) << " printers: " << d->printerList.count();

    for (QList<QPrinterInfo>::iterator it = d->printerList.begin() ;
         it != d->printerList.end() ; ++it)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << " printer: " << it->printerName();
        d->photoUi->m_printer_choice->addItem(it->printerName());
    }

    connect(d->photoUi->m_printer_choice, SIGNAL(activated(QString)),
            this, SLOT(slotOutputChanged(QString)));

    connect(d->photoUi->BtnPreviewPageUp, SIGNAL(clicked()),
            this, SLOT(slotBtnPreviewPageUpClicked()));

    connect(d->photoUi->BtnPreviewPageDown, SIGNAL(clicked()),
            this, SLOT(slotBtnPreviewPageDownClicked()));

    connect(d->photoUi->ListPhotoSizes, SIGNAL(currentRowChanged(int)),
            this, SLOT(slotListPhotoSizesSelected()));

    connect(d->photoUi->m_pagesetup, SIGNAL(clicked()),
            this, SLOT(slotPageSetup()));

    if (d->photoUi->mPrintList->layout())
    {
        delete d->photoUi->mPrintList->layout();
    }

    // -----------------------------------

    d->photoUi->mPrintList->setIface(d->iface);
    d->photoUi->mPrintList->setAllowDuplicate(true);
    d->photoUi->mPrintList->setControlButtons(DImagesList::Add      |
                                    DImagesList::Remove   |
                                    DImagesList::MoveUp   |
                                    DImagesList::MoveDown |
                                    DImagesList::Clear    |
                                    DImagesList::Save     |
                                    DImagesList::Load);
    d->photoUi->mPrintList->setControlButtonsPlacement(DImagesList::ControlButtonsAbove);
    d->photoUi->mPrintList->enableDragAndDrop(false);

    d->photoUi->BmpFirstPagePreview->setAlignment(Qt::AlignHCenter);

    connect(d->photoUi->mPrintList, SIGNAL(signalMoveDownItem()),
            this, SLOT(slotBtnPrintOrderDownClicked()));

    connect(d->photoUi->mPrintList, SIGNAL(signalMoveUpItem()),
            this, SLOT(slotBtnPrintOrderUpClicked()));

    connect(d->photoUi->mPrintList, SIGNAL(signalAddItems(QList<QUrl>)),
            this, SLOT(slotAddItems(QList<QUrl>)));

    connect(d->photoUi->mPrintList, SIGNAL(signalRemovingItem(int)),
            this, SLOT(slotRemovingItem(int)));

    connect(d->photoUi->mPrintList, SIGNAL(signalContextMenuRequested()),
            this, SLOT(slotContextMenuRequested()));

    connect(d->photoUi->mPrintList, SIGNAL(signalXMLSaveItem(QXmlStreamWriter&, int)),
            this, SLOT(slotXMLSaveItem(QXmlStreamWriter&, int)));

    connect(d->photoUi->mPrintList, SIGNAL(signalXMLCustomElements(QXmlStreamWriter&)),
            this, SLOT(slotXMLCustomElement(QXmlStreamWriter&)));

    connect(d->photoUi->mPrintList, SIGNAL(signalXMLLoadImageElement(QXmlStreamReader&)),
            this, SLOT(slotXMLLoadElement(QXmlStreamReader&)));

    connect(d->photoUi->mPrintList, SIGNAL(signalXMLCustomElements(QXmlStreamReader&)),
            this, SLOT(slotXMLCustomElement(QXmlStreamReader&)));

    // -----------------------------------

    setPageWidget(d->photoUi);
    setLeftBottomPix(QIcon::fromTheme(QLatin1String("image-stack")));

    slotOutputChanged(d->photoUi->m_printer_choice->currentText());
}

AdvPrintPhotoPage::~AdvPrintPhotoPage()
{
    delete d->printer;
    delete d->pageSetupDlg;
    delete d;
}

void AdvPrintPhotoPage::initializePage()
{
    d->photoUi->mPrintList->listView()->clear();

    if (d->wizard->settings()->selMode == AdvPrintSettings::IMAGES)
    {
        d->photoUi->mPrintList->loadImagesFromCurrentSelection();
    }
    else
    {
        d->wizard->setItemsList(d->wizard->settings()->inputImages);
    }
}

QPrinter* AdvPrintPhotoPage::printer() const
{
    return d->printer;
}

DImagesList* AdvPrintPhotoPage::imagesList() const
{
    return d->photoUi->mPrintList;
}

Ui_AdvPrintPhotoPage* AdvPrintPhotoPage::ui() const
{
    return d->photoUi;
}

void AdvPrintPhotoPage::slotOutputChanged(const QString& text)
{
    if (text == i18n("Print to PDF") ||
        text == i18n("Print to JPG") ||
        text == i18n("Print with Gimp"))
    {
        delete d->printer;

        d->printer = new QPrinter();
        d->printer->setOutputFormat(QPrinter::PdfFormat);
    }
    else // real printer
    {
        for (QList<QPrinterInfo>::iterator it = d->printerList.begin() ;
             it != d->printerList.end() ; ++it)
        {
            if (it->printerName() == text)
            {
                qCDebug(DIGIKAM_GENERAL_LOG) << "Chosen printer: " << it->printerName();
                delete d->printer;
                d->printer = new QPrinter(*it);
            }
        }

        d->printer->setOutputFormat(QPrinter::NativeFormat);
    }

    // default no margins

    d->printer->setFullPage(true);
    d->printer->setPageMargins(0, 0, 0, 0, QPrinter::Millimeter);
}

bool AdvPrintPhotoPage::isComplete() const
{
    return (!d->photoUi->mPrintList->imageUrls().empty());
}

void AdvPrintPhotoPage::slotXMLLoadElement(QXmlStreamReader& xmlReader)
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
                QXmlStreamAttributes attrs     = xmlReader.attributes();
                // get value of each attribute from QXmlStreamAttributes
                QStringRef attr                = attrs.value(QLatin1String("type"));
                bool ok;

                if (!attr.isEmpty())
                {
                    qCDebug(DIGIKAM_GENERAL_LOG) << " found " << attr.toString();
                    pPhoto->m_pAdvPrintCaptionInfo->m_captionType =
                        (AdvPrintCaptionInfo::AvailableCaptions)attr.toString().toInt(&ok);
                }

                attr = attrs.value(QLatin1String("font"));

                if (!attr.isEmpty())
                {
                    qCDebug(DIGIKAM_GENERAL_LOG) << " found " << attr.toString();
                    pPhoto->m_pAdvPrintCaptionInfo->m_captionFont.fromString(attr.toString());
                }

                attr = attrs.value(QLatin1String("color"));

                if (!attr.isEmpty())
                {
                    qCDebug(DIGIKAM_GENERAL_LOG) << " found " << attr.toString();
                    pPhoto->m_pAdvPrintCaptionInfo->m_captionColor.setNamedColor(attr.toString());
                }

                attr = attrs.value(QLatin1String("size"));

                if (!attr.isEmpty())
                {
                    qCDebug(DIGIKAM_GENERAL_LOG) << " found " << attr.toString();
                    pPhoto->m_pAdvPrintCaptionInfo->m_captionSize = attr.toString().toInt(&ok);
                }

                attr = attrs.value(QLatin1String("text"));

                if (!attr.isEmpty())
                {
                    qCDebug(DIGIKAM_GENERAL_LOG) << " found " << attr.toString();
                    pPhoto->m_pAdvPrintCaptionInfo->m_captionText = attr.toString();
                }
            }
        }
    }
}

void AdvPrintPhotoPage::slotXMLSaveItem(QXmlStreamWriter& xmlWriter, int itemIndex)
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

void AdvPrintPhotoPage::slotXMLCustomElement(QXmlStreamWriter& xmlWriter)
{
    xmlWriter.writeStartElement(QLatin1String("pa_layout"));
    xmlWriter.writeAttribute(QLatin1String("Printer"),   d->photoUi->m_printer_choice->currentText());
    xmlWriter.writeAttribute(QLatin1String("PageSize"),  QString::fromUtf8("%1").arg(d->printer->paperSize()));
    xmlWriter.writeAttribute(QLatin1String("PhotoSize"), d->photoUi->ListPhotoSizes->currentItem()->text());
    xmlWriter.writeEndElement(); // pa_layout
}

void AdvPrintPhotoPage::slotContextMenuRequested()
{
    if (d->settings->photos.size())
    {
        int itemIndex         = d->photoUi->mPrintList->listView()->currentIndex().row();
        d->photoUi->mPrintList->listView()->blockSignals(true);
        QMenu menu(d->photoUi->mPrintList->listView());
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
        d->photoUi->mPrintList->listView()->blockSignals(false);
    }
}

void AdvPrintPhotoPage::slotIncreaseCopies()
{
    if (d->settings->photos.size())
    {
        QList<QUrl> list;
        DImagesListViewItem* const item = dynamic_cast<DImagesListViewItem*>(d->photoUi->mPrintList->listView()->currentItem());

        if (!item)
            return;

        list.append(item->url());
        qCDebug(DIGIKAM_GENERAL_LOG) << " Adding a copy of " << item->url();
        d->photoUi->mPrintList->slotAddImages(list);
    }
}

void AdvPrintPhotoPage::slotDecreaseCopies()
{
    if (d->settings->photos.size())
    {
        DImagesListViewItem* const item = dynamic_cast<DImagesListViewItem*>
            (d->photoUi->mPrintList->listView()->currentItem());

        if (!item)
            return;

        qCDebug(DIGIKAM_GENERAL_LOG) << " Removing a copy of " << item->url();
        d->photoUi->mPrintList->slotRemoveItems();
    }
}

void AdvPrintPhotoPage::slotAddItems(const QList<QUrl>& list)
{
    if (list.count() == 0)
    {
        return;
    }

    QList<QUrl> urls;
    d->photoUi->mPrintList->blockSignals(true);

    for (QList<QUrl>::ConstIterator it = list.constBegin() ;
         it != list.constEnd() ; ++it)
    {
        QUrl imageUrl = *it;

        // Check if the new item already exist in the list.
        bool found    = false;

        for (int i = 0 ; i < d->settings->photos.count() && !found ; ++i)
        {
            AdvPrintPhoto* const pCurrentPhoto = d->settings->photos.at(i);

            if (pCurrentPhoto &&
                (pCurrentPhoto->m_url == imageUrl) &&
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
            pPhoto->m_url               = *it;
            pPhoto->m_first             = true;
            d->settings->photos.append(pPhoto);

            qCDebug(DIGIKAM_GENERAL_LOG) << "Added new fileName: "
                                         << pPhoto->m_url.fileName();
        }
    }

    d->photoUi->mPrintList->blockSignals(false);

    if (d->settings->photos.size())
    {
        setComplete(true);
    }
}

void AdvPrintPhotoPage::slotRemovingItem(int itemIndex)
{
    if (d->settings->photos.size() && itemIndex >= 0)
    {
        /// Debug data: found and copies
        bool found = false;
        int copies = 0;

        d->photoUi->mPrintList->blockSignals(true);
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

        d->photoUi->mPrintList->blockSignals(false);
        d->wizard->previewPhotos();
    }

    if (d->settings->photos.empty())
    {
        // No photos => disabling next button (e.g. crop page)
        setComplete(false);
    }
}

void AdvPrintPhotoPage::slotBtnPrintOrderDownClicked()
{
    d->photoUi->mPrintList->blockSignals(true);
    int currentIndex = d->photoUi->mPrintList->listView()->currentIndex().row();

    qCDebug(DIGIKAM_GENERAL_LOG) << "Moved photo "
                                 << currentIndex - 1
                                 << " to  "
                                 << currentIndex;

    d->settings->photos.swap(currentIndex, currentIndex - 1);
    d->photoUi->mPrintList->blockSignals(false);
    d->wizard->previewPhotos();
}

void AdvPrintPhotoPage::slotBtnPrintOrderUpClicked()
{
    d->photoUi->mPrintList->blockSignals(true);
    int currentIndex = d->photoUi->mPrintList->listView()->currentIndex().row();

    qCDebug(DIGIKAM_GENERAL_LOG) << "Moved photo "
                                 << currentIndex
                                 << " to  "
                                 << currentIndex + 1;

    d->settings->photos.swap(currentIndex, currentIndex + 1);
    d->photoUi->mPrintList->blockSignals(false);
    d->wizard->previewPhotos();
}

void AdvPrintPhotoPage::slotXMLCustomElement(QXmlStreamReader& xmlReader)
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
                qCDebug(DIGIKAM_GENERAL_LOG) << " found " << attr.toString();
                int index = d->photoUi->m_printer_choice->findText(attr.toString());

                if (index != -1)
                {
                    d->photoUi->m_printer_choice->setCurrentIndex(index);
                }

                slotOutputChanged(d->photoUi->m_printer_choice->currentText());
            }

            attr = attrs.value(QLatin1String("PageSize"));

            if (!attr.isEmpty())
            {
                qCDebug(DIGIKAM_GENERAL_LOG) << " found " << attr.toString();
                QPrinter::PaperSize paperSize = (QPrinter::PaperSize)attr.toString().toInt(&ok);
                d->printer->setPaperSize(paperSize);
            }

            attr = attrs.value(QLatin1String("PhotoSize"));

            if (!attr.isEmpty())
            {
                qCDebug(DIGIKAM_GENERAL_LOG) << " found " << attr.toString();
                d->settings->savedPhotoSize = attr.toString();
            }
        }

        xmlReader.readNext();
    }

    // reset preview page number
    d->settings->currentPreviewPage = 0;

    d->wizard->initPhotoSizes(d->printer->paperSize(QPrinter::Millimeter));

    QList<QListWidgetItem*> list    = d->photoUi->ListPhotoSizes->findItems(d->settings->savedPhotoSize,
                                                                            Qt::MatchExactly);

    if (list.count())
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << " PhotoSize " << list[0]->text();
        d->photoUi->ListPhotoSizes->setCurrentItem(list[0]);
    }
    else
    {
        d->photoUi->ListPhotoSizes->setCurrentRow(0);
    }

    d->wizard->previewPhotos();
}

void AdvPrintPhotoPage::slotBtnPreviewPageDownClicked()
{
    if (d->settings->currentPreviewPage == 0)
        return;

    d->settings->currentPreviewPage--;
    d->wizard->previewPhotos();
}

void AdvPrintPhotoPage::slotBtnPreviewPageUpClicked()
{
    if (d->settings->currentPreviewPage == getPageCount() - 1)
        return;

    d->settings->currentPreviewPage++;
    d->wizard->previewPhotos();
}

int AdvPrintPhotoPage::getPageCount() const
{
    int pageCount   = 0;
    int photoCount  =  d->settings->photos.count();

    if (photoCount > 0)
    {
        // get the selected layout
        AdvPrintPhotoSize* const s = d->settings->photosizes.at(d->photoUi->ListPhotoSizes->currentRow());

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

void AdvPrintPhotoPage::createPhotoGrid(AdvPrintPhotoSize* const p,
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

void AdvPrintPhotoPage::slotListPhotoSizesSelected()
{
    AdvPrintPhotoSize* s = 0;
    QSizeF size, sizeManaged;

    // TODO FREE STYLE
    // check if layout is managed by templates or free one
    // get the selected layout
    int curr              = d->photoUi->ListPhotoSizes->currentRow();
    QListWidgetItem* item = d->photoUi->ListPhotoSizes->item(curr);

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

            s->layouts.append(new QRect(0, 0,
                                        (int)sizeManaged.width(),
                                        (int)sizeManaged.height()));
            s->autoRotate  = custDlg.m_autorotate->isChecked();
            s->label       = item->text();
            s->dpi         = 0;

            int pageWidth  = (int)(size.width())  * scaleValue;
            int pageHeight = (int)(size.height()) * scaleValue;
            createPhotoGrid(s, pageWidth, pageHeight,
                            rows, columns, &iconpreview);
        }
        else if (custDlg.m_fitAsManyCheck->isChecked())
        {
            int width  = custDlg.m_photoWidth->value();
            int height = custDlg.m_photoHeight->value();

            //photo size must be less than page size
            static const float round_value = 0.01F;

            if ((height > (size.height() + round_value) ||
                 width  > (size.width()  + round_value)))
            {
                qCDebug(DIGIKAM_GENERAL_LOG) << "photo size "
                                             << QSize(width, height)
                                             << "> page size "
                                             << size;
                delete s;
                s = NULL;
            }
            else
            {
                // fit as many photos of given size as possible
                s->layouts.append(new QRect(0, 0, (int)sizeManaged.width(),
                                            (int)sizeManaged.height()));
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

                            qCDebug(DIGIKAM_GENERAL_LOG) << "photo at P("
                                                         << photoX
                                                         << ", "
                                                         << photoY
                                                         << ") size("
                                                         << width
                                                         << ", "
                                                         << height;

                            s->layouts.append(new QRect(photoX, photoY,
                                                        width, height));
                            iconpreview.fillRect(photoX, photoY,
                                                 width, height, Qt::color1);
                        }
                    }
                }
                else
                {
                    qCDebug(DIGIKAM_GENERAL_LOG) << "I can't go on, rows "
                                                 << nRows
                                                 << "> columns "
                                                 << nColumns;
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
        d->photoUi->ListPhotoSizes->blockSignals(true);
        d->photoUi->ListPhotoSizes->setCurrentRow(0, QItemSelectionModel::Select);
        d->photoUi->ListPhotoSizes->blockSignals(false);
    }

    // reset preview page number
    d->settings->currentPreviewPage = 0;
    d->wizard->previewPhotos();
}

void AdvPrintPhotoPage::slotPageSetup()
{
    delete d->pageSetupDlg;
    d->pageSetupDlg = new QPageSetupDialog(d->printer, this);
    int ret         = d->pageSetupDlg->exec();

    if (ret == QDialog::Accepted)
    {
        QPrinter* const printer = d->pageSetupDlg->printer();

        qCDebug(DIGIKAM_GENERAL_LOG) << "Dialog exit, new size "
                                     << printer->paperSize(QPrinter::Millimeter)
                                     << " internal size "
                                     << d->printer->paperSize(QPrinter::Millimeter);

        qreal left, top, right, bottom;
        d->printer->getPageMargins(&left, &top,
                                   &right, &bottom,
                                   QPrinter::Millimeter);

        qCDebug(DIGIKAM_GENERAL_LOG) << "Dialog exit, new margins: left "
                                     << left
                                     << " right "
                                     << right
                                     << " top "
                                     << top
                                     << " bottom "
                                     << bottom;

        // next should be useless invoke once changing wizard page
        //d->wizard->initPhotoSizes(d->printer.paperSize(QPrinter::Millimeter));

        //d->settings->pageSize = d->printer.paperSize(QPrinter::Millimeter);

#ifdef DEBUG
        qCDebug(DIGIKAM_GENERAL_LOG) << " dialog exited num of copies: "
                                     << printer->numCopies()
                                     << " inside:   "
                                     << d->printer->numCopies();

        qCDebug(DIGIKAM_GENERAL_LOG) << " dialog exited from : "
                                     << printer->fromPage()
                                     << " to:   "
                                     << d->printer->toPage();
#endif
    }

    // Fix the page size dialog and preview PhotoPage
    d->wizard->initPhotoSizes(d->printer->paperSize(QPrinter::Millimeter));

    // restore photoSize
    if (d->settings->savedPhotoSize == i18n(CUSTOM_PAGE_LAYOUT_NAME))
    {
        d->photoUi->ListPhotoSizes->setCurrentRow(0);
    }
    else
    {
        QList<QListWidgetItem*> list =
            d->photoUi->ListPhotoSizes->findItems(d->settings->savedPhotoSize,
                                                  Qt::MatchExactly);

        if (list.count())
            d->photoUi->ListPhotoSizes->setCurrentItem(list[0]);
        else
            d->photoUi->ListPhotoSizes->setCurrentRow(0);
    }

    // create our photo sizes list
    d->wizard->previewPhotos();
}

} // namespace Digikam
