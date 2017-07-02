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
#include <QFileDialog>
#include <QPainter>
#include <QPalette>
#include <QtGlobal>
#include <QPrinter>
#include <QPrintDialog>
#include <QPageSetupDialog>
#include <QPrinterInfo>
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

// KDE includes

#include <kconfigdialogmanager.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdesktopfile.h>
#include <klocalizedstring.h>

// Local includes

#include "dwizardpage.h"
#include "dimageslist.h"
#include "dinfointerface.h"
#include "advprintphoto.h"
#include "advprintutils.h"
#include "advprintintropage.h"
#include "templateicon.h"
#include "advprintcustomdlg.h"
#include "ui_advprintcroppage.h"
#include "ui_advprintphotopage.h"
#include "digikam_debug.h"

namespace Digikam
{

const float FONT_HEIGHT_RATIO = 0.8F;

template <class Ui_Class>

class WizardUI : public QWidget, public Ui_Class
{
public:

    WizardUI(QWidget* const parent)
        : QWidget(parent)
    {
        this->setupUi(this);
        layout()->setContentsMargins(QMargins());
    }
};

typedef WizardUI<Ui_AdvPrintPhotoPage> PhotoUI;
typedef WizardUI<Ui_AdvPrintCropPage>  CropUI;

class WizardPage : public DWizardPage
{
public:

    WizardPage(DWizardDlg* const dialog, const QString& title, QWidget* const ui)
        : DWizardPage(dialog, title)
    {
        setPageWidget(ui);
        setShowLeftView(false);
    }
};

typedef WizardPage PhotoPage;
typedef WizardPage CropPage;

// ---------------------------------------------------------------------------

// some title name definitions (managed by translators)
const char* photoPageName        = I18N_NOOP("Select page layout");
const char* cropPageName         = I18N_NOOP("Crop photos");
// custom page layout
const char* customPageLayoutName = I18N_NOOP("Custom");

// ---------------------------------------------------------------------------

class AdvPrintWizard::Private
{
public:

    Private()
    {
        m_introPage            = 0;
        m_photoPage            = 0;
        m_cropPage             = 0;
        m_photoUi              = 0;
        m_cropUi               = 0;
        m_infopageCurrentPhoto = 0;
        m_currentPreviewPage   = 0;
        m_currentCropPhoto     = 0;
        m_cancelPrinting       = false;
        m_pDlg                 = 0;
        m_printer              = 0;
        m_imagesFilesListBox   = 0;
        m_iface                = 0;
    }

    AdvPrintIntroPage*        m_introPage;
    PhotoPage*                m_photoPage;
    CropPage*                 m_cropPage;

    PhotoUI*                  m_photoUi;
    CropUI*                   m_cropUi;

    // Page Size in mm
    QSizeF                    m_pageSize;

    QList<AdvPrintPhoto*>     m_photos;
    QList<AdvPrintPhotoSize*> m_photoSizes;
    int                       m_infopageCurrentPhoto;
    int                       m_currentPreviewPage;
    int                       m_currentCropPhoto;
    bool                      m_cancelPrinting;
    QString                   m_tempPath;
    QStringList               m_gimpFiles;
    QString                   m_savedPhotoSize;

    QPageSetupDialog*         m_pDlg;
    QPrinter*                 m_printer;
    QList<QPrinterInfo>       m_printerList;
    DImagesList*              m_imagesFilesListBox;
    DInfoInterface*           m_iface;
};

AdvPrintWizard::AdvPrintWizard(QWidget* const parent, DInfoInterface* const iface)
    : DWizardDlg(parent, QLatin1String("PrintCreatorDialog")),
      d(new Private)
{
    setWindowTitle(i18n("Print Creator"));

    d->m_iface       = iface;
    d->m_introPage   = new AdvPrintIntroPage(this, i18n("Welcome to Print Creator"));

    // -----------------------------------

    d->m_photoUi     = new PhotoUI(this);
    d->m_photoPage   = new PhotoPage(this, i18n(photoPageName), d->m_photoUi);

    d->m_photoUi->BtnPreviewPageUp->setIcon(QIcon::fromTheme(QLatin1String("go-next"))
                                            .pixmap(16, 16));
    d->m_photoUi->BtnPreviewPageDown->setIcon(QIcon::fromTheme(QLatin1String("go-previous"))
                                              .pixmap(16, 16));

    QList<QPrinterInfo>::iterator it;
    d->m_printerList = QPrinterInfo::availablePrinters();

    qCDebug(DIGIKAM_GENERAL_LOG) << " printers: " << d->m_printerList.count();

    for (it = d->m_printerList.begin() ; it != d->m_printerList.end() ; ++it)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << " printer: " << it->printerName();
        d->m_photoUi->m_printer_choice->addItem(it->printerName());
    }

    connect(d->m_photoUi->m_captions, SIGNAL(activated(QString)),
            this, SLOT(captionChanged(QString)));

    connect(d->m_photoUi->m_FreeCaptionFormat , SIGNAL(editingFinished()),
            this, SLOT(infopage_updateCaptions()));

    connect(d->m_photoUi->m_sameCaption , SIGNAL(stateChanged(int)),
            this, SLOT(infopage_updateCaptions()));

    connect(d->m_photoUi->m_font_name , SIGNAL(currentFontChanged(QFont)),
            this, SLOT(infopage_updateCaptions()));

    connect(d->m_photoUi->m_font_size , SIGNAL(valueChanged(int)),
            this, SLOT(infopage_updateCaptions()));

    connect(d->m_photoUi->m_font_color , SIGNAL(signalColorSelected(QColor)),
            this, SLOT(infopage_updateCaptions()));

    connect(d->m_photoUi->m_setDefault , SIGNAL(clicked()),
            this, SLOT(saveCaptionSettings()));

    connect(d->m_photoUi->m_printer_choice, SIGNAL(activated(QString)),
            this, SLOT(outputChanged(QString)));

    connect(d->m_photoUi->BtnPreviewPageUp, SIGNAL(clicked()),
            this, SLOT(BtnPreviewPageUp_clicked()));

    connect(d->m_photoUi->BtnPreviewPageDown, SIGNAL(clicked()),
            this, SLOT(BtnPreviewPageDown_clicked()));

    connect(d->m_photoUi->ListPhotoSizes, SIGNAL(currentRowChanged(int)),
            this, SLOT(ListPhotoSizes_selected()));

    connect(d->m_photoUi->m_pagesetup, SIGNAL(clicked()),
            this, SLOT(pagesetupclicked()));

    if (d->m_photoUi->mPrintList->layout())
    {
        delete d->m_photoUi->mPrintList->layout();
    }

    // -----------------------------------

    QVBoxLayout* printListLayout = new QVBoxLayout;
    printListLayout->setContentsMargins(QMargins());
    printListLayout->setSpacing(0);

    d->m_imagesFilesListBox = new DImagesList(d->m_photoUi->mPrintList, 32);
    d->m_imagesFilesListBox->setAllowDuplicate(true);
    d->m_imagesFilesListBox->setControlButtons(DImagesList::Add      |
                                               DImagesList::Remove   |
                                               DImagesList::MoveUp   |
                                               DImagesList::MoveDown |
                                               DImagesList::Clear    |
                                               DImagesList::Save     |
                                               DImagesList::Load);
    d->m_imagesFilesListBox->setControlButtonsPlacement(DImagesList::ControlButtonsAbove);
    d->m_imagesFilesListBox->enableDragAndDrop(false);

    printListLayout->addWidget(d->m_imagesFilesListBox);
    d->m_photoUi->mPrintList->setLayout(printListLayout);

    d->m_photoUi->BmpFirstPagePreview->setAlignment(Qt::AlignHCenter);

    connect(d->m_imagesFilesListBox, SIGNAL(signalMoveDownItem()),
            this, SLOT(BtnPrintOrderDown_clicked()));

    connect(d->m_imagesFilesListBox, SIGNAL(signalMoveUpItem()),
            this, SLOT(BtnPrintOrderUp_clicked()));

    connect(d->m_imagesFilesListBox, SIGNAL(signalAddItems(QList<QUrl>)),
            this, SLOT(slotAddItems(QList<QUrl>)));

    connect(d->m_imagesFilesListBox, SIGNAL(signalRemovingItem(int)),
            this, SLOT(slotRemovingItem(int)));

    connect(d->m_imagesFilesListBox, SIGNAL(signalItemClicked(QTreeWidgetItem*)),
            this, SLOT(imageSelected(QTreeWidgetItem*)));

    connect(d->m_imagesFilesListBox, SIGNAL(signalContextMenuRequested()),
            this, SLOT(slotContextMenuRequested()));

    // Save item list => we catch the signal to add our PA attributes and elements Image children
    connect(d->m_imagesFilesListBox, SIGNAL(signalXMLSaveItem(QXmlStreamWriter&, int)),
            this, SLOT(slotXMLSaveItem(QXmlStreamWriter&, int)));

    // Save item list => we catch the signal to add our PA elements (not per image)
    connect(d->m_imagesFilesListBox, SIGNAL(signalXMLCustomElements(QXmlStreamWriter&)),
            this, SLOT(slotXMLCustomElement(QXmlStreamWriter&)));

    connect(d->m_imagesFilesListBox, SIGNAL(signalXMLLoadImageElement(QXmlStreamReader&)),
            this, SLOT(slotXMLLoadElement(QXmlStreamReader&)));

    connect(d->m_imagesFilesListBox, SIGNAL(signalXMLCustomElements(QXmlStreamReader&)),
            this, SLOT(slotXMLCustomElement(QXmlStreamReader&)));

    // -----------------------------------

    d->m_cropUi   = new CropUI(this);
    d->m_cropPage = new CropPage(this, i18n(cropPageName), d->m_cropUi) ;

    d->m_cropUi->BtnCropRotateRight->setIcon(QIcon::fromTheme(QLatin1String("object-rotate-right"))
                                             .pixmap(16, 16));
    d->m_cropUi->BtnCropRotateLeft->setIcon(QIcon::fromTheme(QLatin1String("object-rotate-left"))
                                            .pixmap(16, 16));

    connect(d->m_cropUi->BtnCropPrev, SIGNAL(clicked()),
            this, SLOT(BtnCropPrev_clicked()));

    connect(d->m_cropUi->BtnCropNext, SIGNAL(clicked()),
            this, SLOT(BtnCropNext_clicked()));

    connect(d->m_cropUi->BtnCropRotateRight, SIGNAL(clicked()),
            this, SLOT(BtnCropRotateRight_clicked()));

    connect(d->m_cropUi->BtnCropRotateLeft, SIGNAL(clicked()),
            this, SLOT(BtnCropRotateLeft_clicked()));

    connect(d->m_cropUi->m_disableCrop, SIGNAL(stateChanged(int)),
            this, SLOT(crop_selection(int)));

    connect(d->m_cropUi->BtnSaveAs, SIGNAL (clicked()),
            this, SLOT (BtnSaveAs_clicked()));

    // -----------------------------------

    // select a different page to force a refresh in initPhotoSizes.
    d->m_pageSize    = QSizeF(-1, -1);

    connect(this, SIGNAL(currentIdChanged(int)),
            this, SLOT(pageChanged(int)));

    connect(button(QWizard::CancelButton), SIGNAL(clicked()),
            this, SLOT(reject()));

    connect(this, SIGNAL(pageRemoved(int)),
            this, SLOT(slotPageRemoved(int)));
}

AdvPrintWizard::~AdvPrintWizard()
{
    // TODO private object could be deleted inside private destructor
    delete d->m_pDlg;
    delete d->m_printer;

    for (int i = 0 ; i < d->m_photos.count() ; ++i)
        delete d->m_photos.at(i);

    d->m_photos.clear();
    delete d;
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

void AdvPrintWizard::print(const QList<QUrl>& fileList, const QString& tempPath)
{
    for (int i = 0 ; i < d->m_photos.count() ; ++i)
        delete d->m_photos.at(i);

    d->m_photos.clear();
    //d->m_photoPage->m_PictureInfo->setRowCount(fileList.count());

    for (int i = 0; i < fileList.count(); ++i)
    {
        AdvPrintPhoto* const photo = new AdvPrintPhoto(150, d->m_iface);
        photo->m_filename          = fileList[i];
        photo->m_first             = true;
        d->m_photos.append(photo);
    }

    d->m_tempPath = tempPath;
    d->m_cropUi->BtnCropPrev->setEnabled(false);

    if (d->m_photos.count() == 1)
        d->m_cropUi->BtnCropNext->setEnabled(false);

    emit currentIdChanged(d->m_photoPage->id());
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
                            d->m_photoSizes.append(p);
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
                                 << d->m_pageSize;

    // don't refresh anything if we haven't changed page sizes.
    if (pageSize == d->m_pageSize)
        return;

    d->m_pageSize = pageSize;

    // cleaning m_pageSize memory before invoking clear()
    for (int i = 0; i < d->m_photoSizes.count(); ++i)
        delete d->m_photoSizes.at(i);

    d->m_photoSizes.clear();

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

    qCDebug(DIGIKAM_GENERAL_LOG) << "d->m_photoSizes.count()="   << d->m_photoSizes.count();
    qCDebug(DIGIKAM_GENERAL_LOG) << "d->m_photoSizes.isEmpty()=" << d->m_photoSizes.isEmpty();

    if (d->m_photoSizes.isEmpty())
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
        d->m_photoSizes.append(p);
    }

    // load the photo sizes into the listbox
    d->m_photoUi->ListPhotoSizes->blockSignals(true);
    d->m_photoUi->ListPhotoSizes->clear();
    QList<AdvPrintPhotoSize*>::iterator it;

    for (it = d->m_photoSizes.begin() ; it != d->m_photoSizes.end() ; ++it)
    {
        AdvPrintPhotoSize* const s = static_cast<AdvPrintPhotoSize*>(*it);

        if (s)
        {
            QListWidgetItem* const pWItem = new QListWidgetItem(s->label);
            pWItem->setIcon(s->icon);
            d->m_photoUi->ListPhotoSizes->addItem(pWItem);
        }
    }

    // Adding custom choice
    QListWidgetItem* const pWItem = new QListWidgetItem(i18n(customPageLayoutName));

    //TODO FREE STYLE ICON
    TemplateIcon ti(80, pageSize.toSize());
    ti.begin();
    QPainter& painter = ti.getPainter();
    painter.setPen(Qt::color1);
    painter.drawText(painter.viewport(), Qt::AlignCenter, i18n("Custom layout"));
    ti.end();

    pWItem->setIcon(ti.getIcon());
    d->m_photoUi->ListPhotoSizes->addItem(pWItem);
    d->m_photoUi->ListPhotoSizes->blockSignals(false);
    d->m_photoUi->ListPhotoSizes->setCurrentRow(0, QItemSelectionModel::Select);
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
    AdvPrintPhotoSize* const s = d->m_photoSizes.at(d->m_photoUi->ListPhotoSizes->currentRow());

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
    int photoCount  =  d->m_photos.count();

    if (photoCount > 0)
    {
        // get the selected layout
        AdvPrintPhotoSize* const s = d->m_photoSizes.at(d->m_photoUi->ListPhotoSizes->currentRow());

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

    QFont font(photo->m_pAdvPrintCaptionInfo->m_caption_font);
    font.setStyleHint(QFont::SansSerif);
    font.setPixelSize((int)(captionH * FONT_HEIGHT_RATIO));
    font.setWeight(QFont::Normal);

    QFontMetrics fm(font);
    int pixelsHigh = fm.height();

    p.setFont(font);
    p.setPen(photo->m_pAdvPrintCaptionInfo->m_caption_color);

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

    switch (photo->m_pAdvPrintCaptionInfo->m_caption_type)
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
        case AdvPrintCaptionInfo::Free:
            format =  photo->m_pAdvPrintCaptionInfo->m_caption_text;
            break;
        default:
            qCWarning(DIGIKAM_GENERAL_LOG) << "UNKNOWN caption type "
                                           << photo->m_pAdvPrintCaptionInfo->m_caption_type;
            break;
    }

    QFileInfo fi(photo->m_filename.toLocalFile());
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

    DItemInfo info(d->m_iface->itemInfo(photo->m_filename));
    format.replace(QString::fromUtf8("%r"), resolution);
    format.replace(QString::fromUtf8("%f"), fi.fileName());
    format.replace(QString::fromUtf8("%c"), info.comment());
    format.replace(QString::fromUtf8("%d"), QLocale().toString(info.dateTime(), 
                                            QLocale::ShortFormat));

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
        else if (!cropDisabled)       //d->m_cropUi->m_disableCrop->isChecked() )
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

        if (cropDisabled)  //->m_cropPage->m_disableCrop->isChecked() )
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
            photo->m_pAdvPrintCaptionInfo->m_caption_type != AdvPrintCaptionInfo::NoCaptions)
        {
            p.save();
            QString caption;
            caption = captionFormatter(photo);
            qCDebug(DIGIKAM_GENERAL_LOG) << "Caption " << caption ;

            // draw the text at (0,0), but we will translate and rotate the world
            // before drawing so the text will be in the correct location
            // next, do we rotate?
            int captionW        = w - 2;
            double ratio        = photo->m_pAdvPrintCaptionInfo->m_caption_size * 0.01;
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
    AdvPrintPhotoSize* const s = d->m_photoSizes.at(d->m_photoUi->ListPhotoSizes->currentRow());
    d->m_cropUi->cropFrame->init(photo,
                                 getLayout(photoIndex)->width(),
                                 getLayout(photoIndex)->height(),
                                 s->autoRotate);
    d->m_cropUi->LblCropPhoto->setText(i18n("Photo %1 of %2",
                                            photoIndex + 1,
                                            QString::number(d->m_photos.count())));
}

// update the pages to be printed and preview first/last pages
void AdvPrintWizard::previewPhotos()
{
    //Change cursor to waitCursor during transition
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    // get the selected layout
    int curr                   = d->m_photoUi->ListPhotoSizes->currentRow();
    AdvPrintPhotoSize* const s = d->m_photoSizes.at(curr);
    int photoCount             =  d->m_photos.count();
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

    d->m_photoUi->LblPhotoCount->setText(QString::number(photoCount));
    d->m_photoUi->LblSheetsPrinted->setText(QString::number(pageCount));
    d->m_photoUi->LblEmptySlots->setText(QString::number(emptySlots));

    // photo previews
    // preview the first page.
    // find the first page of photos
    int count   = 0;
    int page    = 0;
    int current = 0;
    QList<AdvPrintPhoto*>::iterator it;

    for (it = d->m_photos.begin() ; it != d->m_photos.end() ; ++it)
    {
        AdvPrintPhoto* const photo = static_cast<AdvPrintPhoto*>(*it);

        if (page == d->m_currentPreviewPage)
        {
            photo->m_cropRegion.setRect(-1, -1, -1, -1);
            photo->m_rotation = 0;
            int w             = s->layouts.at(count + 1)->width();
            int h             = s->layouts.at(count + 1)->height();
            d->m_cropUi->cropFrame->init(photo, w, h, s->autoRotate, false);
        }

        count++;

        if (count >= photosPerPage)
        {
            if (page == d->m_currentPreviewPage)
                break;

            page++;
            current += photosPerPage;
            count    = 0;
        }
    }

    // send this photo list to the painter
    if (photoCount > 0)
    {
        QImage img(d->m_photoUi->BmpFirstPagePreview->size(),
                   QImage::Format_ARGB32_Premultiplied);
        QPainter p(&img);
        p.setCompositionMode(QPainter::CompositionMode_Clear);
        //p.setCompositionMode(QPainter::CompositionMode_Destination );
        p.fillRect(img.rect(), Qt::color0); //Qt::transparent );
        p.setCompositionMode(QPainter::CompositionMode_SourceOver);
        paintOnePage(p, d->m_photos, s->layouts, current, d->m_cropUi->m_disableCrop->isChecked(), true);
        p.end();

        d->m_photoUi->BmpFirstPagePreview->clear();
        d->m_photoUi->BmpFirstPagePreview->setPixmap(QPixmap::fromImage(img));
        d->m_photoUi->LblPreview->setText(i18n("Page %1 of %2", d->m_currentPreviewPage + 1, getPageCount()));
    }
    else
    {
        d->m_photoUi->BmpFirstPagePreview->clear();
        d->m_photoUi->LblPreview->clear();
//       d->m_photoUi->BmpFirstPagePreview->setPixmap ( QPixmap() );
        d->m_photoUi->LblPreview->setText(i18n("Page %1 of %2", 0, 0));
    }

    manageBtnPreviewPage();
    d->m_photoUi->update();
    QApplication::restoreOverrideCursor();
}

void AdvPrintWizard::manageBtnPreviewPage()
{
    if (d->m_photos.empty())
    {
        d->m_photoUi->BtnPreviewPageDown->setEnabled(false);
        d->m_photoUi->BtnPreviewPageUp->setEnabled(false);
    }
    else
    {
        d->m_photoUi->BtnPreviewPageDown->setEnabled(true);
        d->m_photoUi->BtnPreviewPageUp->setEnabled(true);

        if (d->m_currentPreviewPage == 0)
        {
            d->m_photoUi->BtnPreviewPageDown->setEnabled(false);
        }

        if ((d->m_currentPreviewPage + 1) == getPageCount())
        {
            d->m_photoUi->BtnPreviewPageUp->setEnabled(false);
        }
    }
}

void AdvPrintWizard::infopage_setCaptionButtons()
{
    if (d->m_photos.size())
    {
        AdvPrintPhoto* const pPhoto = d->m_photos.at(d->m_infopageCurrentPhoto);

        if (pPhoto && !d->m_photoUi->m_sameCaption->isChecked())
        {
            infopage_blockCaptionButtons();

            if (pPhoto->m_pAdvPrintCaptionInfo)
            {
                d->m_photoUi->m_font_color->setColor(pPhoto->m_pAdvPrintCaptionInfo->m_caption_color);
                d->m_photoUi->m_font_size->setValue(pPhoto->m_pAdvPrintCaptionInfo->m_caption_size);
                d->m_photoUi->m_font_name->setCurrentFont(pPhoto->m_pAdvPrintCaptionInfo->m_caption_font);
                d->m_photoUi->m_captions->setCurrentIndex(int(pPhoto->m_pAdvPrintCaptionInfo->m_caption_type));
                d->m_photoUi->m_FreeCaptionFormat->setText(pPhoto->m_pAdvPrintCaptionInfo->m_caption_text);
                enableCaptionGroup(d->m_photoUi->m_captions->currentText());
            }
            else
            {
                infopage_readCaptionSettings();
                captionChanged(d->m_photoUi->m_captions->currentText());
            }

            infopage_blockCaptionButtons(false);
        }
    }
}

void AdvPrintWizard::slotXMLCustomElement(QXmlStreamWriter& xmlWriter)
{
    xmlWriter.writeStartElement(QLatin1String("pa_layout"));
    xmlWriter.writeAttribute(QLatin1String("Printer"),   d->m_photoUi->m_printer_choice->currentText());
    xmlWriter.writeAttribute(QLatin1String("PageSize"),  QString::fromUtf8("%1").arg(d->m_printer->paperSize()));
    xmlWriter.writeAttribute(QLatin1String("PhotoSize"), d->m_photoUi->ListPhotoSizes->currentItem()->text());
    xmlWriter.writeEndElement(); // pa_layout
}

void AdvPrintWizard::slotXMLSaveItem(QXmlStreamWriter& xmlWriter, int itemIndex)
{
    if (d->m_photos.size())
    {
        AdvPrintPhoto* const pPhoto = d->m_photos[itemIndex];
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
                                     QString::fromUtf8("%1").arg(pPhoto->m_pAdvPrintCaptionInfo->m_caption_type));
            xmlWriter.writeAttribute(QLatin1String("font"),
                                     pPhoto->m_pAdvPrintCaptionInfo->m_caption_font.toString());
            xmlWriter.writeAttribute(QLatin1String("size"),
                                     QString::fromUtf8("%1").arg(pPhoto->m_pAdvPrintCaptionInfo->m_caption_size));
            xmlWriter.writeAttribute(QLatin1String("color"),
                                     pPhoto->m_pAdvPrintCaptionInfo->m_caption_color.name());
            xmlWriter.writeAttribute(QLatin1String("text"),
                                     pPhoto->m_pAdvPrintCaptionInfo->m_caption_text);
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
                int index = d->m_photoUi->m_printer_choice->findText(attr.toString());

                if (index != -1)
                {
                    d->m_photoUi->m_printer_choice->setCurrentIndex(index);
                }

                outputChanged(d->m_photoUi->m_printer_choice->currentText());
            }

            attr = attrs.value(QLatin1String("PageSize"));

            if (!attr.isEmpty())
            {
                qCDebug(DIGIKAM_GENERAL_LOG) <<  " found " << attr.toString();
                QPrinter::PaperSize paperSize = (QPrinter::PaperSize)attr.toString().toInt(&ok);
                d->m_printer->setPaperSize(paperSize);
            }

            attr = attrs.value(QLatin1String("PhotoSize"));

            if (!attr.isEmpty())
            {
                qCDebug(DIGIKAM_GENERAL_LOG) <<  " found " << attr.toString();
                d->m_savedPhotoSize = attr.toString();
            }
        }

        xmlReader.readNext();
    }

    // reset preview page number
    d->m_currentPreviewPage      = 0;
    initPhotoSizes(d->m_printer->paperSize(QPrinter::Millimeter));
    QList<QListWidgetItem*> list = d->m_photoUi->ListPhotoSizes->findItems(d->m_savedPhotoSize, Qt::MatchExactly);

    if (list.count())
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << " PhotoSize " << list[0]->text();
        d->m_photoUi->ListPhotoSizes->setCurrentItem(list[0]);
    }
    else
    {
        d->m_photoUi->ListPhotoSizes->setCurrentRow(0);
    }

    previewPhotos();
}

void AdvPrintWizard::slotXMLLoadElement(QXmlStreamReader& xmlReader)
{
    if (d->m_photos.size())
    {
        // read image is the last.
        AdvPrintPhoto* const pPhoto = d->m_photos[d->m_photos.size()-1];
        qCDebug(DIGIKAM_GENERAL_LOG) << " invoked " << xmlReader.name();

        while (xmlReader.readNextStartElement())
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << pPhoto->m_filename << " " << xmlReader.name();

            if (xmlReader.name() == QLatin1String("pa_caption"))
            {
                d->m_photoUi->m_sameCaption->blockSignals(true);
                d->m_photoUi->m_sameCaption->setCheckState( Qt::Unchecked );
                d->m_photoUi->m_sameCaption->blockSignals(false);

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
                    pPhoto->m_pAdvPrintCaptionInfo->m_caption_type =
                        (AdvPrintCaptionInfo::AvailableCaptions)attr.toString().toInt(&ok);
                }

                attr = attrs.value(QLatin1String("font"));

                if (!attr.isEmpty())
                {
                    qCDebug(DIGIKAM_GENERAL_LOG) <<  " found " << attr.toString();
                    pPhoto->m_pAdvPrintCaptionInfo->m_caption_font.fromString(attr.toString());
                }

                attr = attrs.value(QLatin1String("color"));

                if (!attr.isEmpty())
                {
                    qCDebug(DIGIKAM_GENERAL_LOG) <<  " found " << attr.toString();
                    pPhoto->m_pAdvPrintCaptionInfo->m_caption_color.setNamedColor(attr.toString());
                }

                attr = attrs.value(QLatin1String("size"));

                if (!attr.isEmpty())
                {
                    qCDebug(DIGIKAM_GENERAL_LOG) <<  " found " << attr.toString();
                    pPhoto->m_pAdvPrintCaptionInfo->m_caption_size = attr.toString().toInt(&ok);
                }

                attr = attrs.value(QLatin1String("text"));

                if (!attr.isEmpty())
                {
                    qCDebug(DIGIKAM_GENERAL_LOG) <<  " found " << attr.toString();
                    pPhoto->m_pAdvPrintCaptionInfo->m_caption_text = attr.toString();
                }

                infopage_setCaptionButtons();
            }
        }
    }
}

void AdvPrintWizard::slotContextMenuRequested()
{
    if (d->m_photos.size())
    {
        int itemIndex         = d->m_imagesFilesListBox->listView()->currentIndex().row();
        d->m_imagesFilesListBox->listView()->blockSignals(true);
        QMenu menu(d->m_imagesFilesListBox->listView());
        QAction* const action = menu.addAction(i18n("Add again"));

        connect(action, SIGNAL(triggered()),
                this , SLOT(increaseCopies()));

        AdvPrintPhoto* const pPhoto  = d->m_photos[itemIndex];

        qCDebug(DIGIKAM_GENERAL_LOG) << " copies "
                                     << pPhoto->m_copies
                                     << " first "
                                     << pPhoto->m_first;

        if (pPhoto->m_copies > 1 || !pPhoto->m_first)
        {
            QAction* const actionr = menu.addAction(i18n("Remove"));

            connect(actionr, SIGNAL(triggered()),
                    this, SLOT(decreaseCopies()));
        }

        menu.exec(QCursor::pos());
        d->m_imagesFilesListBox->listView()->blockSignals(false);
    }
}

void AdvPrintWizard::imageSelected(QTreeWidgetItem* item)
{
    DImagesListViewItem* const l_item = dynamic_cast<DImagesListViewItem*>(item);

    if (!l_item)
        return;

    int itemIndex = d->m_imagesFilesListBox->listView()->indexFromItem(l_item).row();

    qCDebug(DIGIKAM_GENERAL_LOG) << " current row now is " << itemIndex;
    d->m_infopageCurrentPhoto = itemIndex;

    infopage_setCaptionButtons();
}

void AdvPrintWizard::decreaseCopies()
{
    if (d->m_photos.size())
    {
        DImagesListViewItem* const item = dynamic_cast<DImagesListViewItem*>
            (d->m_imagesFilesListBox->listView()->currentItem());

        if (!item)
            return;

        qCDebug(DIGIKAM_GENERAL_LOG) << " Removing a copy of " << item->url();
        d->m_imagesFilesListBox->slotRemoveItems();
    }
}

void AdvPrintWizard::slotRemovingItem(int itemIndex)
{
    if (d->m_photos.size() && itemIndex >= 0)
    {
        /// Debug data: found and copies
        bool found = false;
        int copies = 0;

        d->m_imagesFilesListBox->blockSignals(true);
        AdvPrintPhoto* const pPhotoToRemove = d->m_photos.at(itemIndex);

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
                for (int i = 0 ; i < d->m_photos.count() && !found ; ++i)
                {
                    AdvPrintPhoto* const pCurrentPhoto = d->m_photos.at(i);

                    if (pCurrentPhoto && pCurrentPhoto->m_filename == pPhotoToRemove->m_filename)
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
            for (int i = 0 ; i < d->m_photos.count() && !found ; ++i)
            {
                AdvPrintPhoto* const pCurrentPhoto = d->m_photos.at(i);

                if (pCurrentPhoto &&
                    pCurrentPhoto->m_filename == pPhotoToRemove->m_filename &&
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
                                            << pPhotoToRemove->m_filename.fileName()
                                            << " copy number "
                                            << copies;
        }

        d->m_photos.removeAt(itemIndex);
        delete pPhotoToRemove;

        d->m_imagesFilesListBox->blockSignals(false);
        previewPhotos();
    }

    if (d->m_photos.empty())
    {
        // No photos => disabling next button (e.g. crop page)
        d->m_photoPage->setComplete(false);
    }
}

void AdvPrintWizard::slotAddItems(const QList<QUrl>& list)
{
    if (list.count() == 0)
    {
        return;
    }

    QList<QUrl> urls;
    d->m_imagesFilesListBox->blockSignals(true);

    for (QList<QUrl>::ConstIterator it = list.constBegin() ; it != list.constEnd() ; ++it)
    {
        QUrl imageUrl = *it;

        // Check if the new item already exist in the list.
        bool found    = false;

        for (int i = 0 ; i < d->m_photos.count() && !found ; ++i)
        {
            AdvPrintPhoto* const pCurrentPhoto = d->m_photos.at(i);

            if (pCurrentPhoto &&
                pCurrentPhoto->m_filename == imageUrl &&
                pCurrentPhoto->m_first)
            {
                pCurrentPhoto->m_copies++;
                found                       = true;
                AdvPrintPhoto* const pPhoto = new AdvPrintPhoto(*pCurrentPhoto);
                pPhoto->m_first             = false;
                d->m_photos.append(pPhoto);
                qCDebug(DIGIKAM_GENERAL_LOG) << "Added fileName: "
                                             << pPhoto->m_filename.fileName()
                                             << " copy number "
                                             << pCurrentPhoto->m_copies;
            }
        }

        if (!found)
        {
            AdvPrintPhoto* const pPhoto = new AdvPrintPhoto(150, d->m_iface);
            pPhoto->m_filename          = *it;
            pPhoto->m_first             = true;
            d->m_photos.append(pPhoto);
            qCDebug(DIGIKAM_GENERAL_LOG) << "Added new fileName: "
                                         << pPhoto->m_filename.fileName();
        }
    }

    d->m_imagesFilesListBox->blockSignals(false);
    infopage_updateCaptions();
    //previewPhotos();

    if (d->m_photos.size())
    {
        d->m_photoPage->setComplete(true);
    }
}

void AdvPrintWizard::increaseCopies()
{
    if (d->m_photos.size())
    {
        QList<QUrl> list;
        DImagesListViewItem* const item = dynamic_cast<DImagesListViewItem*>(d->m_imagesFilesListBox->listView()->currentItem());

        if (!item)
            return;

        list.append(item->url());
        qCDebug(DIGIKAM_GENERAL_LOG) << " Adding a copy of " << item->url();
        d->m_imagesFilesListBox->slotAddImages(list);
    }
}

void AdvPrintWizard::pageChanged(int curr)
{
    QWizardPage* const current = page(curr);

    if (!current) return;

    QWizardPage* const before  = visitedPages().isEmpty() ? 0 : page(visitedPages().last());

    //Change cursor to waitCursor during transition
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    if (before)
    {
        saveSettings(before->title());
        qCDebug(DIGIKAM_GENERAL_LOG) << " before " << before->title();
    }

    qCDebug(DIGIKAM_GENERAL_LOG) << " current " << current->title();

    if (current->title() == i18n(photoPageName))
    {
        // readSettings only the first time
        if (!before)
            readSettings(current->title());

        // set to first photo
        d->m_infopageCurrentPhoto = 0;
        d->m_imagesFilesListBox->listView()->clear();
        QList<QUrl> list;

        for (int i = 0; i < d->m_photos.count();++i)
        {
            AdvPrintPhoto* const pCurrentPhoto = d->m_photos.at(i);

            if (pCurrentPhoto)
            {
                list.push_back(pCurrentPhoto->m_filename);
            }
        }

        d->m_imagesFilesListBox->blockSignals(true);
        d->m_imagesFilesListBox->slotAddImages(list);
        d->m_imagesFilesListBox->listView()->setCurrentItem(d->m_imagesFilesListBox->listView()->itemAt(0, 0));
        d->m_imagesFilesListBox->blockSignals(false);
        d->m_photoUi->LblPhotoCount->setText(QString::number(d->m_photos.count()));

        // PhotoPage
        initPhotoSizes(d->m_printer->paperSize(QPrinter::Millimeter));
        // restore photoSize

        if (before && d->m_savedPhotoSize == i18n(customPageLayoutName))
        {
            d->m_photoUi->ListPhotoSizes->setCurrentRow(0);
        }
        else
        {
            QList<QListWidgetItem*> list = d->m_photoUi->ListPhotoSizes->findItems(d->m_savedPhotoSize, Qt::MatchExactly);

            if (list.count())
                d->m_photoUi->ListPhotoSizes->setCurrentItem(list[0]);
            else
                d->m_photoUi->ListPhotoSizes->setCurrentRow(0);
        }

        // update captions only the first time to avoid missing old changes when
        // back to this page
        if (!before)
            infopage_updateCaptions();

        // reset preview page number
        d->m_currentPreviewPage = 0;
        // create our photo sizes list
        previewPhotos();
    }
    else if (current->title() == i18n(cropPageName))
    {
        readSettings(current->title());
        d->m_currentCropPhoto = 0;

        if (d->m_photos.size())
        {
            AdvPrintPhoto* const photo = d->m_photos[d->m_currentCropPhoto];
            setBtnCropEnabled();
            this->update();
            updateCropFrame(photo, d->m_currentCropPhoto);
        }
        else
        {
            // NOTE it should not pass here
            qCDebug(DIGIKAM_GENERAL_LOG) << "Not any photos selected cropping is disabled";
        }
    }

    QApplication::restoreOverrideCursor();
}

void AdvPrintWizard::outputChanged(const QString& text)
{
    if (text == i18n("Print to PDF") ||
        text == i18n("Print to JPG") ||
        text == i18n("Print to gimp"))
    {
        delete d->m_printer;

        d->m_printer = new QPrinter();
        d->m_printer->setOutputFormat(QPrinter::PdfFormat);
    }
    else // real printer
    {
        QList<QPrinterInfo>::iterator it;
        //m_printerList = QPrinterInfo::availablePrinters();

        for (it = d->m_printerList.begin(); it != d->m_printerList.end(); ++it)
        {
            if (it->printerName() == text)
            {
                qCDebug(DIGIKAM_GENERAL_LOG) << "Chosen printer: " << it->printerName();
                delete d->m_printer;
                d->m_printer = new QPrinter(*it);
            }
        }

        //d->m_printer->setPrinterName(text);
        d->m_printer->setOutputFormat(QPrinter::NativeFormat);
    }

    //default no margins
    d->m_printer->setFullPage(true);
    d->m_printer->setPageMargins(0, 0, 0, 0, QPrinter::Millimeter);
}

void AdvPrintWizard::updateCaption(AdvPrintPhoto* pPhoto)
{
    if (pPhoto)
    {
        if (!pPhoto->m_pAdvPrintCaptionInfo &&
            d->m_photoUi->m_captions->currentIndex() != AdvPrintCaptionInfo::NoCaptions)
        {
            pPhoto->m_pAdvPrintCaptionInfo = new AdvPrintCaptionInfo();
        }
        else if (pPhoto->m_pAdvPrintCaptionInfo &&
            d->m_photoUi->m_captions->currentIndex() == AdvPrintCaptionInfo::NoCaptions)
        {
            delete pPhoto->m_pAdvPrintCaptionInfo;
            pPhoto->m_pAdvPrintCaptionInfo = NULL;
        }

        if (pPhoto->m_pAdvPrintCaptionInfo)
        {
            pPhoto->m_pAdvPrintCaptionInfo->m_caption_color = d->m_photoUi->m_font_color->color();
            pPhoto->m_pAdvPrintCaptionInfo->m_caption_size  = d->m_photoUi->m_font_size->value();
            pPhoto->m_pAdvPrintCaptionInfo->m_caption_font  = d->m_photoUi->m_font_name->currentFont();
            pPhoto->m_pAdvPrintCaptionInfo->m_caption_type  = (AdvPrintCaptionInfo::AvailableCaptions)d->m_photoUi->m_captions->currentIndex();
            pPhoto->m_pAdvPrintCaptionInfo->m_caption_text  = d->m_photoUi->m_FreeCaptionFormat->text();
        }
    }
}

void AdvPrintWizard::infopage_updateCaptions()
{
    if (d->m_photos.size())
    {
        if (d->m_photoUi->m_sameCaption->isChecked())
        {
            QList<AdvPrintPhoto*>::iterator it;

            for (it = d->m_photos.begin() ; it != d->m_photos.end() ; ++it)
            {
                AdvPrintPhoto* const pPhoto = static_cast<AdvPrintPhoto*>(*it);
                updateCaption(pPhoto);
            }
        }
        else
        {
            QList <QTreeWidgetItem*> list = d->m_imagesFilesListBox->listView()->selectedItems();

            foreach(QTreeWidgetItem* const item, list)
            {
                DImagesListViewItem* const lvItem = dynamic_cast<DImagesListViewItem*>(item);

                if (item)
                {
                    int itemIndex               = d->m_imagesFilesListBox->listView()->indexFromItem(lvItem).row();
                    AdvPrintPhoto* const pPhoto = d->m_photos.at(itemIndex);
                    updateCaption(pPhoto);
                }
            }
        }
    }

    // create our photo sizes list
    previewPhotos();
}

void AdvPrintWizard::enableCaptionGroup(const QString& text)
{
    bool fontSettingsEnabled;

    if (text == i18n("No captions"))
    {
        fontSettingsEnabled = false;
        d->m_photoUi->m_FreeCaptionFormat->setEnabled(false);
        d->m_photoUi->m_free_label->setEnabled(false);
    }
    else if (text == i18n("Free"))
    {
        fontSettingsEnabled = true;
        d->m_photoUi->m_FreeCaptionFormat->setEnabled(true);
        d->m_photoUi->m_free_label->setEnabled(true);
    }
    else
    {
        fontSettingsEnabled = true;
        d->m_photoUi->m_FreeCaptionFormat->setEnabled(false);
        d->m_photoUi->m_free_label->setEnabled(false);
    }

    d->m_photoUi->m_font_name->setEnabled(fontSettingsEnabled);
    d->m_photoUi->m_font_size->setEnabled(fontSettingsEnabled);
    d->m_photoUi->m_font_color->setEnabled(fontSettingsEnabled);
}

void AdvPrintWizard::captionChanged(const QString& text)
{
    enableCaptionGroup(text);
    infopage_updateCaptions();
}

void AdvPrintWizard::BtnCropRotateLeft_clicked()
{
    // by definition, the cropRegion should be set by now,
    // which means that after our rotation it will become invalid,
    // so we will initialize it to -2 in an awful hack (this
    // tells the cropFrame to reset the crop region, but don't
    // automagically rotate the image to fit.
    AdvPrintPhoto* const photo = d->m_photos[d->m_currentCropPhoto];
    photo->m_cropRegion        = QRect(-2, -2, -2, -2);
    photo->m_rotation          = (photo->m_rotation - 90) % 360;

    updateCropFrame(photo, d->m_currentCropPhoto);
}

void AdvPrintWizard::BtnCropRotateRight_clicked()
{
    // by definition, the cropRegion should be set by now,
    // which means that after our rotation it will become invalid,
    // so we will initialize it to -2 in an awful hack (this
    // tells the cropFrame to reset the crop region, but don't
    // automagically rotate the image to fit.
    AdvPrintPhoto* const photo = d->m_photos[d->m_currentCropPhoto];
    photo->m_cropRegion        = QRect(-2, -2, -2, -2);
    photo->m_rotation          = (photo->m_rotation + 90) % 360;

    updateCropFrame(photo, d->m_currentCropPhoto);
}

void AdvPrintWizard::setBtnCropEnabled()
{
    if (d->m_currentCropPhoto == 0)
        d->m_cropUi->BtnCropPrev->setEnabled(false);
    else
        d->m_cropUi->BtnCropPrev->setEnabled(true);

    if (d->m_currentCropPhoto == (int) d->m_photos.count() - 1)
        d->m_cropUi->BtnCropNext->setEnabled(false);
    else
        d->m_cropUi->BtnCropNext->setEnabled(true);
}

void AdvPrintWizard::BtnCropNext_clicked()
{
    AdvPrintPhoto* const photo = d->m_photos[++d->m_currentCropPhoto];
    setBtnCropEnabled();

    if (!photo)
    {
        d->m_currentCropPhoto = d->m_photos.count() - 1;
        return;
    }

    updateCropFrame(photo, d->m_currentCropPhoto);
}

void AdvPrintWizard::BtnCropPrev_clicked()
{
    AdvPrintPhoto* const photo = d->m_photos[--d->m_currentCropPhoto];

    setBtnCropEnabled();

    if (!photo)
    {
        d->m_currentCropPhoto = 0;
        return;
    }

    updateCropFrame(photo, d->m_currentCropPhoto);
}

void AdvPrintWizard::BtnPrintOrderUp_clicked()
{
    d->m_imagesFilesListBox->blockSignals(true);
    int currentIndex = d->m_imagesFilesListBox->listView()->currentIndex().row();

    qCDebug(DIGIKAM_GENERAL_LOG) << "Moved photo " << currentIndex << " to  " << currentIndex + 1;

    d->m_photos.swap(currentIndex, currentIndex + 1);
    d->m_imagesFilesListBox->blockSignals(false);
    previewPhotos();
}

void AdvPrintWizard::ListPhotoSizes_selected()
{
    AdvPrintPhotoSize* s = NULL;
    QSizeF size, sizeManaged;

    // TODO FREE STYLE
    // check if layout is managed by templates or free one
    // get the selected layout
    int curr              = d->m_photoUi->ListPhotoSizes->currentRow();
    QListWidgetItem* item = d->m_photoUi->ListPhotoSizes->item(curr);

    // if custom page layout we launch a dialog to choose what kind
    if (item->text() == i18n(customPageLayoutName))
    {
        // check if a custom layout has already been added
        if (curr >= 0 && curr < d->m_photoSizes.size())
        {
            s = d->m_photoSizes.at(curr);
            d->m_photoSizes.removeAt(curr);
            delete s;
            s = NULL;
        }

        AdvPrintCustomLayoutDlg custDlg(this);
        custDlg.readSettings();
        custDlg.exec();
        custDlg.saveSettings();

        // get parameters from dialog
        size           = d->m_pageSize;
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
            d->m_photoSizes.append(s);
        }
    }
    else
    {
        s = d->m_photoSizes.at(curr);
    }

    if (!s)
    {
        // change position to top
        d->m_photoUi->ListPhotoSizes->blockSignals(true);
        d->m_photoUi->ListPhotoSizes->setCurrentRow(0, QItemSelectionModel::Select);
        d->m_photoUi->ListPhotoSizes->blockSignals(false);
    }

    // reset preview page number
    d->m_currentPreviewPage = 0;
    previewPhotos();
}

void AdvPrintWizard::BtnPrintOrderDown_clicked()
{
    d->m_imagesFilesListBox->blockSignals(true);
    int currentIndex = d->m_imagesFilesListBox->listView()->currentIndex().row();

    qCDebug(DIGIKAM_GENERAL_LOG) << "Moved photo "
                                 << currentIndex - 1
                                 << " to  "
                                 << currentIndex;

    d->m_photos.swap(currentIndex, currentIndex - 1);
    d->m_imagesFilesListBox->blockSignals(false);
    previewPhotos();
}

void AdvPrintWizard::BtnPreviewPageDown_clicked()
{
    if (d->m_currentPreviewPage == 0)
        return;

    d->m_currentPreviewPage--;
    previewPhotos();
}

void AdvPrintWizard::BtnPreviewPageUp_clicked()
{
    if (d->m_currentPreviewPage == getPageCount() - 1)
        return;

    d->m_currentPreviewPage++;
    previewPhotos();
}

void AdvPrintWizard::BtnSaveAs_clicked()
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "Save As Clicked";
    KConfig config;
    KConfigGroup group = config.group( QLatin1String( "PrintCreator" ) );
    QUrl outputPath; // force to get current directory as default
    outputPath         = QUrl(group.readPathEntry( "OutputPath", outputPath.url() ));
    QString filename   = QFileDialog::getSaveFileName(qApp->activeWindow(),
                                                      i18n("Output Path"),
                                                      QLatin1String(".jpeg") );
    d->m_cropUi->m_fileName->setText(filename);
}

void AdvPrintWizard::saveSettings(const QString& pageName)
{
    qCDebug(DIGIKAM_GENERAL_LOG) << pageName;

    // Save the current settings
    KConfig config;
    KConfigGroup group = config.group(QLatin1String("PrintCreator"));

    if (pageName == i18n(photoPageName))
    {
        group.writeEntry(QLatin1String("Printer"),
                         d->m_photoUi->m_printer_choice->currentText());
        // PhotoPage
        // photo size
        d->m_savedPhotoSize = d->m_photoUi->ListPhotoSizes->currentItem()->text();
        group.writeEntry(QLatin1String("PhotoSize"),
                         d->m_savedPhotoSize);
        group.writeEntry(QLatin1String("IconSize"),
                         d->m_photoUi->ListPhotoSizes->iconSize());
    }
    else if (pageName == i18n(cropPageName))
    {
        // CropPage
        if (d->m_photoUi->m_printer_choice->currentText() == i18n("Print to JPG"))
        {
            // output path
            QString outputPath = d->m_cropUi->m_fileName->text();
            group.writePathEntry(QLatin1String("OutputPath"), outputPath);
        }
    }
}

void AdvPrintWizard::infopage_readCaptionSettings()
{
    KConfig config;
    KConfigGroup group = config.group(QLatin1String("PrintCreator"));

    // image captions
    d->m_photoUi->m_captions->setCurrentIndex(group.readEntry(QLatin1String("Captions"), 0));
    // caption color
    QColor defColor(Qt::yellow);
    QColor color = group.readEntry(QLatin1String("CaptionColor"), defColor);
    d->m_photoUi->m_font_color->setColor(color);
    // caption font
    QFont defFont(QLatin1String("Sans Serif"));
    QFont font = group.readEntry(QLatin1String("CaptionFont"), defFont);
    d->m_photoUi->m_font_name->setCurrentFont(font.family());
    // caption size
    int fontSize = group.readEntry(QLatin1String("CaptionSize"), 4);
    d->m_photoUi->m_font_size->setValue(fontSize);
    // free caption
    QString captionTxt = group.readEntry(QLatin1String("FreeCaption"));
    d->m_photoUi->m_FreeCaptionFormat->setText(captionTxt);
}

void AdvPrintWizard::readSettings(const QString& pageName)
{
    KConfig config;
    KConfigGroup group = config.group(QLatin1String("PrintCreator"));

    qCDebug(DIGIKAM_GENERAL_LOG) << pageName;

    if (pageName == i18n(photoPageName))
    {
        // InfoPage
        QString printerName = group.readEntry("Printer", i18n("Print to PDF"));
        int index           = d->m_photoUi->m_printer_choice->findText(printerName);

        if (index != -1)
        {
            d->m_photoUi->m_printer_choice->setCurrentIndex(index);
        }

        // init QPrinter
        outputChanged(d->m_photoUi->m_printer_choice->currentText());

        QSize iconSize = group.readEntry("IconSize", QSize(24, 24));
        d->m_photoUi->ListPhotoSizes->setIconSize(iconSize);

        // photo size
        d->m_savedPhotoSize = group.readEntry("PhotoSize");
        //caption
        initPhotoSizes(d->m_printer->paperSize(QPrinter::Millimeter));
        infopage_readCaptionSettings();

        bool same_to_all = group.readEntry("SameCaptionToAll", 0) == 1;
        d->m_photoUi->m_sameCaption->setChecked(same_to_all);
        //enable right caption stuff
        captionChanged(d->m_photoUi->m_captions->currentText());
    }
    else if (pageName == i18n(cropPageName))
    {
        // CropPage
        if (d->m_photoUi->m_printer_choice->currentText() == i18n("Print to JPG"))
        {
            // set the last output path
            QUrl outputPath; // force to get current directory as default
            outputPath = QUrl(group.readPathEntry("OutputPath", outputPath.url()));

            d->m_cropUi->m_fileName->setVisible(true);
            d->m_cropUi->m_fileName->setEnabled(true);
            d->m_cropUi->m_fileName->setText(outputPath.path());
            d->m_cropUi->BtnSaveAs->setVisible(true);
        }
        else
        {
            d->m_cropUi->m_fileName->setVisible(false);
            d->m_cropUi->BtnSaveAs->setVisible(false);

        }
    }
}

void AdvPrintWizard::printPhotos(const QList<AdvPrintPhoto*>& photos,
                                 const QList<QRect*>& layouts,
                                 QPrinter& printer)
{
    d->m_cancelPrinting = false;
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
                                d->m_cropUi->m_disableCrop->isChecked());

        if (printing)
            printer.newPage();

        pbar.setValue(current);
        QApplication::processEvents();

        if (d->m_cancelPrinting)
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

    d->m_cancelPrinting = false;
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
                                d->m_cropUi->m_disableCrop->isChecked());
        painter.end();

        if (saveFile)
        {
            files.append(filename);

            if (!pixmap.save(filename,0,100))
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

        if (d->m_cancelPrinting)
            break;
    }

    return files;
}

void AdvPrintWizard::removeGimpFiles()
{
    for (QStringList::ConstIterator it = d->m_gimpFiles.constBegin(); it != d->m_gimpFiles.constEnd(); ++it)
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

//TODO not needed at the moment maybe we can remove it
void AdvPrintWizard::slotPageRemoved(int id)
{
    qCDebug(DIGIKAM_GENERAL_LOG) << page(id)->title();
}

void AdvPrintWizard::crop_selection(int)
{
    d->m_cropUi->cropFrame->drawCropRectangle(!d->m_cropUi->m_disableCrop->isChecked());
    update();
}

// this is called when Cancel is clicked.
void AdvPrintWizard::reject()
{
    d->m_cancelPrinting = true;

    if (d->m_gimpFiles.count() > 0)
        removeGimpFiles();

    QDialog::reject();
}

void AdvPrintWizard::accept()
{
    if (d->m_photos.empty())
    {
        DWizardDlg::reject();
        return;
    }

    // set the default crop regions if not already set
    AdvPrintPhotoSize* const s = d->m_photoSizes.at(d->m_photoUi->ListPhotoSizes->currentRow());
    QList<AdvPrintPhoto*>::iterator it;
    int i                      = 0;

    for (it = d->m_photos.begin() ; it != d->m_photos.end() ; ++it)
    {
        AdvPrintPhoto* const photo = static_cast<AdvPrintPhoto* >(*it);

        if (photo && photo->m_cropRegion == QRect(-1, -1, -1, -1))
        {
            d->m_cropUi->cropFrame->init(photo,
                                         getLayout(i)->width(),
                                         getLayout(i)->height(),
                                         s->autoRotate);
        }

        i++;
    }

    if (d->m_photoUi->m_printer_choice->currentText() != i18n("Print to JPG") &&
        d->m_photoUi->m_printer_choice->currentText() != i18n("Print to gimp"))
    {
        // tell him again!
        d->m_printer->setFullPage(true);

        qreal left, top, right, bottom;
        d->m_printer->getPageMargins(&left, &top, &right, &bottom, QPrinter::Millimeter);

        qCDebug(DIGIKAM_GENERAL_LOG) << "Margins before print dialog: left "
                                     << left
                                     << " right "
                                     << right
                                     << " top "
                                     << top
                                     << " bottom "
                                     << bottom;

        qCDebug(DIGIKAM_GENERAL_LOG) << "(1) paper page "
                                     << d->m_printer->paperSize()
                                     << " size "
                                     << d->m_printer->paperSize(QPrinter::Millimeter);

        QPrinter::PaperSize paperSize =  d->m_printer->paperSize();

        QPrintDialog* const dialog    = new QPrintDialog(d->m_printer, this);
        dialog->setWindowTitle(i18n("Print assistant"));

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
        if (paperSize !=  d->m_printer->paperSize())
            d->m_printer->setPaperSize(paperSize);

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
        printPhotos(d->m_photos, s->layouts, *d->m_printer);
        QApplication::restoreOverrideCursor();
    }
    else if (d->m_photoUi->m_printer_choice->currentText() == i18n("Print to gimp"))
    {
        // now output the items
        QString path = d->m_tempPath;

        if (!AdvPrintCheckTempPath(this, path))
            return;

        path = path + QLatin1String("kipi_tmp_");

        if (d->m_gimpFiles.count() > 0)
            removeGimpFiles();

        d->m_gimpFiles = printPhotosToFile(d->m_photos, path, s);
        QStringList args;
        QString prog = QLatin1String("gimp-remote");

        for (QStringList::ConstIterator it = d->m_gimpFiles.constBegin() ;
             it != d->m_gimpFiles.constEnd() ; ++it)
        {
            args << (*it);
        }

        if (!AdvPrintLaunchExternalApp(prog, args))
        {
            QMessageBox::information(this, QString(),
                                     i18n("There was an error launching the GIMP. Please make sure it is properly installed."));
            return;
        }
    }
    else if (d->m_photoUi->m_printer_choice->currentText() == i18n("Print to JPG"))
    {
        // now output the items
        //TODO manage URL
        QString path = d->m_cropUi->m_fileName->text();

        if (path.isEmpty())
        {
            QMessageBox::information(this, QString(), i18n("Empty output path."));
            return;
        }

        qCDebug(DIGIKAM_GENERAL_LOG) << path;
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        printPhotosToFile(d->m_photos, path, s);
        QApplication::restoreOverrideCursor();
    }

    saveSettings(currentPage()->title());
    DWizardDlg::accept();
}

void AdvPrintWizard::pagesetupdialogexit()
{
    QPrinter* const printer = d->m_pDlg->printer();

    qCDebug(DIGIKAM_GENERAL_LOG) << "Dialog exit, new size "
                                 << printer->paperSize(QPrinter::Millimeter)
                                 << " internal size "
                                 << d->m_printer->paperSize(QPrinter::Millimeter);

    qreal left, top, right, bottom;
    d->m_printer->getPageMargins(&left, &top, &right, &bottom, QPrinter::Millimeter);

    qCDebug(DIGIKAM_GENERAL_LOG) << "Dialog exit, new margins: left "
                                 << left
                                 << " right "
                                 << right
                                 << " top "
                                 << top
                                 << " bottom "
                                 << bottom;

    // next should be useless invoke once changing wizard page
    //initPhotoSizes ( d->m_printer.paperSize(QPrinter::Millimeter));

    //     d->m_pageSize = d->m_printer.paperSize(QPrinter::Millimeter);
#ifdef NOT_YET
    qCDebug(DIGIKAM_GENERAL_LOG) << " dialog exited num of copies: "
                                 << printer->numCopies()
                                 << " inside:   "
                                 << d->m_printer->numCopies();

    qCDebug(DIGIKAM_GENERAL_LOG) << " dialog exited from : "
                                 << printer->fromPage()
                                 << " to:   "
                                 << d->m_printer->toPage();
#endif
}

void AdvPrintWizard::pagesetupclicked()
{
    delete d->m_pDlg;
    d->m_pDlg = new QPageSetupDialog(d->m_printer, this);
    // TODO next line should work but it doesn't because of a QT bug
    //d->m_pDlg->open(this, SLOT(pagesetupdialogexit()));
    int ret   = d->m_pDlg->exec();

    if (ret == QDialog::Accepted)
    {
        pagesetupdialogexit();
    }

    // FIX page size dialog and preview PhotoPage
    initPhotoSizes(d->m_printer->paperSize(QPrinter::Millimeter));

    // restore photoSize
    if (d->m_savedPhotoSize == i18n(customPageLayoutName))
    {
        d->m_photoUi->ListPhotoSizes->setCurrentRow(0);
    }
    else
    {
        QList<QListWidgetItem*> list = d->m_photoUi->ListPhotoSizes->findItems(d->m_savedPhotoSize,
                                                                               Qt::MatchExactly);

        if (list.count())
            d->m_photoUi->ListPhotoSizes->setCurrentItem(list[0]);
        else
            d->m_photoUi->ListPhotoSizes->setCurrentRow(0);
    }

    // create our photo sizes list
    previewPhotos();
}

void AdvPrintWizard::infopage_blockCaptionButtons(bool block)
{
    d->m_photoUi->m_captions->blockSignals(block);
    d->m_photoUi->m_free_label->blockSignals(block);
    d->m_photoUi->m_sameCaption->blockSignals(block);
    d->m_photoUi->m_font_name->blockSignals(block);
    d->m_photoUi->m_font_size->blockSignals(block);
    d->m_photoUi->m_font_color->blockSignals(block);
}

void AdvPrintWizard::saveCaptionSettings()
{
    // Save the current settings
    KConfig config;
    KConfigGroup group = config.group(QLatin1String("PrintCreator"));
    // image captions
    group.writeEntry(QLatin1String("Captions"),         d->m_photoUi->m_captions->currentIndex());
    // caption color
    group.writeEntry(QLatin1String("CaptionColor"),     d->m_photoUi->m_font_color->color());
    // caption font
    group.writeEntry(QLatin1String("CaptionFont"),      QFont(d->m_photoUi->m_font_name->currentFont()));
    // caption size
    group.writeEntry(QLatin1String("CaptionSize"),      d->m_photoUi->m_font_size->value());
    // free caption
    group.writeEntry(QLatin1String("FreeCaption"),      d->m_photoUi->m_FreeCaptionFormat->text());
    // same to all
    group.writeEntry(QLatin1String("SameCaptionToAll"), (d->m_photoUi->m_sameCaption->isChecked() ? 1 : 0));
}

DInfoInterface* AdvPrintWizard::iface() const
{
    return d->m_iface;
}

} // namespace Digikam
