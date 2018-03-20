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

#ifndef ADV_PRINT_CROP_PAGE_H
#define ADV_PRINT_CROP_PAGE_H

// Qt includes

#include <QString>
#include <QUrl>

// Local includes

#include "dwizardpage.h"
#include "ui_advprintcroppage.h"

namespace Digikam
{

class DImagesList;

class AdvPrintCropPage : public DWizardPage
{
    Q_OBJECT

public:

    explicit AdvPrintCropPage(QWizard* const wizard, const QString& title);
    ~AdvPrintCropPage();

    Ui_AdvPrintCropPage* ui() const;

    void setBtnCropEnabled();

    void initializePage();
    bool validatePage();

private Q_SLOTS:

    void slotCropSelection(int);
    void slotBtnCropPrevClicked();
    void slotBtnCropNextClicked();
    void slotBtnCropRotateLeftClicked();
    void slotBtnCropRotateRightClicked();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // ADV_PRINT_CROP_PAGE_H
