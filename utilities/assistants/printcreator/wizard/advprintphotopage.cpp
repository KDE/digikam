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

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "advprintwizard.h"

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
      : printer(0),
        wizard(0),
        settings(0)
    {
        photoUi = new PhotoUI(dialog);
        wizard  = dynamic_cast<AdvPrintWizard*>(dialog);

        if (wizard)
        {
            settings = wizard->settings();
        }
    }

    PhotoUI*            photoUi;
    QPrinter*           printer;
    QList<QPrinterInfo> printerList;

    AdvPrintWizard*     wizard;
    AdvPrintSettings*   settings;
};

AdvPrintPhotoPage::AdvPrintPhotoPage(QWizard* const wizard, const QString& title)
    : DWizardPage(wizard, title),
      d(new Private(wizard))
{
    d->photoUi->BtnPreviewPageUp->setIcon(QIcon::fromTheme(QLatin1String("go-next"))
                                            .pixmap(16, 16));
    d->photoUi->BtnPreviewPageDown->setIcon(QIcon::fromTheme(QLatin1String("go-previous"))
                                              .pixmap(16, 16));

    QList<QPrinterInfo>::iterator it;
    d->printerList = QPrinterInfo::availablePrinters();

    qCDebug(DIGIKAM_GENERAL_LOG) << " printers: " << d->printerList.count();

    for (it = d->printerList.begin() ; it != d->printerList.end() ; ++it)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << " printer: " << it->printerName();
        d->photoUi->m_printer_choice->addItem(it->printerName());
    }

    connect(d->photoUi->m_printer_choice, SIGNAL(activated(QString)),
            this, SLOT(slotOutputChanged(QString)));

    connect(d->photoUi->BtnPreviewPageUp, SIGNAL(clicked()),
            wizard, SLOT(slotBtnPreviewPageUpClicked()));

    connect(d->photoUi->BtnPreviewPageDown, SIGNAL(clicked()),
            wizard, SLOT(slotBtnPreviewPageDownClicked()));

    connect(d->photoUi->ListPhotoSizes, SIGNAL(currentRowChanged(int)),
            wizard, SLOT(slotListPhotoSizesSelected()));

    connect(d->photoUi->m_pagesetup, SIGNAL(clicked()),
            wizard, SLOT(slotPageSetup()));

    if (d->photoUi->mPrintList->layout())
    {
        delete d->photoUi->mPrintList->layout();
    }

    // -----------------------------------

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
            wizard, SLOT(slotBtnPrintOrderDownClicked()));

    connect(d->photoUi->mPrintList, SIGNAL(signalMoveUpItem()),
            wizard, SLOT(slotBtnPrintOrderUpClicked()));

    connect(d->photoUi->mPrintList, SIGNAL(signalAddItems(QList<QUrl>)),
            wizard, SLOT(slotAddItems(QList<QUrl>)));

    connect(d->photoUi->mPrintList, SIGNAL(signalRemovingItem(int)),
            wizard, SLOT(slotRemovingItem(int)));

    connect(d->photoUi->mPrintList, SIGNAL(signalContextMenuRequested()),
            wizard, SLOT(slotContextMenuRequested()));

    // Save item list => we catch the signal to add our PA attributes and elements Image children
    connect(d->photoUi->mPrintList, SIGNAL(signalXMLSaveItem(QXmlStreamWriter&, int)),
            this, SLOT(slotXMLSaveItem(QXmlStreamWriter&, int)));

    // Save item list => we catch the signal to add our PA elements (not per image)
    connect(d->photoUi->mPrintList, SIGNAL(signalXMLCustomElements(QXmlStreamWriter&)),
            this, SLOT(slotXMLCustomElement(QXmlStreamWriter&)));

    connect(d->photoUi->mPrintList, SIGNAL(signalXMLLoadImageElement(QXmlStreamReader&)),
            this, SLOT(slotXMLLoadElement(QXmlStreamReader&)));

    connect(d->photoUi->mPrintList, SIGNAL(signalXMLCustomElements(QXmlStreamReader&)),
            wizard, SLOT(slotXMLCustomElement(QXmlStreamReader&)));

    // -----------------------------------

    setPageWidget(d->photoUi);
    setLeftBottomPix(QIcon::fromTheme(QLatin1String("image-stack")));

    slotOutputChanged(d->photoUi->m_printer_choice->currentText());
}

AdvPrintPhotoPage::~AdvPrintPhotoPage()
{
    delete d->printer;
    delete d;
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

void AdvPrintPhotoPage::updateUi()
{
    d->photoUi->update();
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

} // namespace Digikam
