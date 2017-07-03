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
#include <QLabel>
#include <QPrinter>
#include <QPrinterInfo>
#include <QWidget>
#include <QApplication>
#include <QStyle>
#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>

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

    Private(QWidget* const parent)
      : printer(0),
        imagesFilesListBox(0)
    {
        photoUi = new PhotoUI(parent);
    }

    PhotoUI*            photoUi;
    QPrinter*           printer;
    QList<QPrinterInfo> printerList;
    DImagesList*        imagesFilesListBox;
};

AdvPrintPhotoPage::AdvPrintPhotoPage(QWizard* const wizard, const QString& title)
    : DWizardPage(wizard, title),
      d(new Private(this))
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

    // -----------------

    connect(d->photoUi->m_captions, SIGNAL(activated(QString)),
            wizard, SLOT(captionChanged(QString)));

    connect(d->photoUi->m_FreeCaptionFormat , SIGNAL(editingFinished()),
            wizard, SLOT(infopage_updateCaptions()));

    connect(d->photoUi->m_sameCaption , SIGNAL(stateChanged(int)),
            wizard, SLOT(infopage_updateCaptions()));

    connect(d->photoUi->m_font_name , SIGNAL(currentFontChanged(QFont)),
            wizard, SLOT(infopage_updateCaptions()));

    connect(d->photoUi->m_font_size , SIGNAL(valueChanged(int)),
            wizard, SLOT(infopage_updateCaptions()));

    connect(d->photoUi->m_font_color , SIGNAL(signalColorSelected(QColor)),
            wizard, SLOT(infopage_updateCaptions()));

    connect(d->photoUi->m_setDefault , SIGNAL(clicked()),
            wizard, SLOT(saveCaptionSettings()));

    connect(d->photoUi->BtnPreviewPageUp, SIGNAL(clicked()),
            wizard, SLOT(BtnPreviewPageUp_clicked()));

    connect(d->photoUi->BtnPreviewPageDown, SIGNAL(clicked()),
            wizard, SLOT(BtnPreviewPageDown_clicked()));

    connect(d->photoUi->ListPhotoSizes, SIGNAL(currentRowChanged(int)),
            wizard, SLOT(ListPhotoSizes_selected()));

    connect(d->photoUi->m_pagesetup, SIGNAL(clicked()),
            wizard, SLOT(pagesetupclicked()));

    if (d->photoUi->mPrintList->layout())
    {
        delete d->photoUi->mPrintList->layout();
    }

    // -----------------------------------

    QVBoxLayout* printListLayout = new QVBoxLayout;
    printListLayout->setContentsMargins(QMargins());
    printListLayout->setSpacing(0);

    d->imagesFilesListBox = new DImagesList(d->photoUi->mPrintList, 32);
    d->imagesFilesListBox->setAllowDuplicate(true);
    d->imagesFilesListBox->setControlButtons(DImagesList::Add      |
                                             DImagesList::Remove   |
                                             DImagesList::MoveUp   |
                                             DImagesList::MoveDown |
                                             DImagesList::Clear    |
                                             DImagesList::Save     |
                                             DImagesList::Load);
    d->imagesFilesListBox->setControlButtonsPlacement(DImagesList::ControlButtonsAbove);
    d->imagesFilesListBox->enableDragAndDrop(false);

    printListLayout->addWidget(d->imagesFilesListBox);
    d->photoUi->mPrintList->setLayout(printListLayout);

    d->photoUi->BmpFirstPagePreview->setAlignment(Qt::AlignHCenter);

    connect(d->imagesFilesListBox, SIGNAL(signalMoveDownItem()),
            wizard, SLOT(BtnPrintOrderDown_clicked()));

    connect(d->imagesFilesListBox, SIGNAL(signalMoveUpItem()),
            wizard, SLOT(BtnPrintOrderUp_clicked()));

    connect(d->imagesFilesListBox, SIGNAL(signalAddItems(QList<QUrl>)),
            wizard, SLOT(slotAddItems(QList<QUrl>)));

    connect(d->imagesFilesListBox, SIGNAL(signalRemovingItem(int)),
            wizard, SLOT(slotRemovingItem(int)));

    connect(d->imagesFilesListBox, SIGNAL(signalItemClicked(QTreeWidgetItem*)),
            wizard, SLOT(imageSelected(QTreeWidgetItem*)));

    connect(d->imagesFilesListBox, SIGNAL(signalContextMenuRequested()),
            wizard, SLOT(slotContextMenuRequested()));

    // Save item list => we catch the signal to add our PA attributes and elements Image children
    connect(d->imagesFilesListBox, SIGNAL(signalXMLSaveItem(QXmlStreamWriter&, int)),
            wizard, SLOT(slotXMLSaveItem(QXmlStreamWriter&, int)));

    // Save item list => we catch the signal to add our PA elements (not per image)
    connect(d->imagesFilesListBox, SIGNAL(signalXMLCustomElements(QXmlStreamWriter&)),
            wizard, SLOT(slotXMLCustomElement(QXmlStreamWriter&)));

    connect(d->imagesFilesListBox, SIGNAL(signalXMLLoadImageElement(QXmlStreamReader&)),
            wizard, SLOT(slotXMLLoadElement(QXmlStreamReader&)));

    connect(d->imagesFilesListBox, SIGNAL(signalXMLCustomElements(QXmlStreamReader&)),
            wizard, SLOT(slotXMLCustomElement(QXmlStreamReader&)));

    // -----------------------------------

    setPageWidget(d->photoUi);
    setLeftBottomPix(QIcon::fromTheme(QLatin1String("image-stack")));
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
    return d->imagesFilesListBox;
}

Ui_AdvPrintPhotoPage* AdvPrintPhotoPage::ui() const
{
    return d->photoUi;
}

void AdvPrintPhotoPage::slotOutputChanged(const QString& text)
{
    if (text == i18n("Print to PDF") ||
        text == i18n("Print to JPG") ||
        text == i18n("Print to gimp"))
    {
        delete d->printer;

        d->printer = new QPrinter();
        d->printer->setOutputFormat(QPrinter::PdfFormat);
    }
    else // real printer
    {
        QList<QPrinterInfo>::iterator it;
        //d->printerList = QPrinterInfo::availablePrinters();

        for (it = d->printerList.begin(); it != d->printerList.end(); ++it)
        {
            if (it->printerName() == text)
            {
                qCDebug(DIGIKAM_GENERAL_LOG) << "Chosen printer: " << it->printerName();
                delete d->printer;
                d->printer = new QPrinter(*it);
            }
        }

        //d->printer->setPrinterName(text);
        d->printer->setOutputFormat(QPrinter::NativeFormat);
    }

    //default no margins
    d->printer->setFullPage(true);
    d->printer->setPageMargins(0, 0, 0, 0, QPrinter::Millimeter);
}

void AdvPrintPhotoPage::updateUi()
{
    d->photoUi->update();
}

} // namespace Digikam
