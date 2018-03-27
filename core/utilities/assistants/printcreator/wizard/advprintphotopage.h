/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-05-25
 * Description : a tool to print images
 *
 * Copyright (C) 2017-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef ADV_PRINT_PHOTO_PAGE_H
#define ADV_PRINT_PHOTO_PAGE_H

// Qt includes

#include <QString>
#include <QPrinter>
#include <QList>
#include <QUrl>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

// Local includes

#include "dwizardpage.h"
#include "dimageslist.h"
#include "advprintphoto.h"
#include "ui_advprintphotopage.h"

namespace Digikam
{

class DImagesList;
class TemplateIcon;

class AdvPrintPhotoPage : public DWizardPage
{
    Q_OBJECT

public:

    explicit AdvPrintPhotoPage(QWizard* const wizard, const QString& title);
    ~AdvPrintPhotoPage();

    QPrinter*             printer()           const;
    DImagesList*          imagesList()        const;
    Ui_AdvPrintPhotoPage* ui()                const;
    bool                  isComplete()        const;
    int                   getPageCount()      const;

    void initializePage();
    bool validatePage();

    /** Create a MxN grid of photos, fitting on the page.
     */
    void createPhotoGrid(AdvPrintPhotoSize* const p,
                         int pageWidth,
                         int pageHeight,
                         int rows,
                         int columns,
                         TemplateIcon* const iconpreview);

    void manageBtnPreviewPage();

    /** Initialize page layout to the given pageSize in mm.
     */
    void initPhotoSizes(const QSizeF& pageSize);

private:

    /** To parse template file with 'fn' as filename, and 'pageSize' in mm.
     */
    void parseTemplateFile(const QString& fn,
                           const QSizeF& pageSize);

public Q_SLOTS:

    void slotOutputChanged(const QString&);

private Q_SLOTS:

    void slotXMLLoadElement(QXmlStreamReader&);

    /** Save item list => we catch the signal to add
     *  our PA attributes and elements Image children
     */
    void slotXMLSaveItem(QXmlStreamWriter&, int);

    /** Save item list => we catch the signal to add
     *  our PA elements (not per image)
     */
    void slotXMLCustomElement(QXmlStreamWriter&);

    void slotXMLCustomElement(QXmlStreamReader&);
    void slotContextMenuRequested();
    void slotIncreaseCopies();
    void slotDecreaseCopies();
    void slotAddItems(const QList<QUrl>&);
    void slotRemovingItems(const QList<int>&);
    void slotBtnPrintOrderDownClicked();
    void slotBtnPrintOrderUpClicked();
    void slotBtnPreviewPageDownClicked();
    void slotBtnPreviewPageUpClicked();
    void slotListPhotoSizesSelected();
    void slotPageSetup();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // ADV_PRINT_PHOTO_PAGE_H
