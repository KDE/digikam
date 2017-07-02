/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-08
 * Description : a tool to print images
 *
 * Copyright (C) 2009-2012 by Angelo Naselli <anaselli at linux dot it>
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

#ifndef ADV_PRINT_OPTIONS_PAGE_H
#define ADV_PRINT_OPTIONS_PAGE_H

// Qt includes

#include <QWidget>
#include <QPrinter>

// Local includes

#include "advprintphoto.h"

namespace Digikam
{

class AdvPrintOptionsPage : public QWidget
{
      Q_OBJECT

public:

    enum ScaleMode
    {
        NoScale,
        ScaleToPage,
        ScaleToCustomSize
    };

    // Order should match the content of the unit combbox in the ui file
    enum Unit
    {
        Millimeters,
        Centimeters,
        Inches
    };

public:

    explicit AdvPrintOptionsPage(QWidget* const parent,
                                 QList<AdvPrintPhoto*>* const photoList);
    ~AdvPrintOptionsPage();

    int  photoXPage()             const;
    int  mp_horPages()            const;
    int  mp_verPages()            const;
    bool printUsingAtkinsLayout() const;

    void loadConfig();

    static double unitToInches(AdvPrintOptionsPage::Unit unit);

private Q_SLOTS:

    void adjustWidthToRatio();
    void adjustHeightToRatio();
    void manageQPrintDialogChanges(QPrinter* printer);
    void selectNext();
    void selectPrev();
    void photoXpageChanged(int i);
    void horizontalPagesChanged(int i);
    void verticalPagesChanged(int i);
    void saveConfig();
    void scaleOption();
    void autoRotate(bool);

    /// Slot to fix per photo position
    void positionChosen(int);

private:

    void imagePreview();
    void enableButtons();
    void setAdvPrintAdditionalInfo();
    void showAdvPrintAdditionalInfo();

    // TODO fix remove what is not needed
    Qt::Alignment alignment()   const;
    Unit          scaleUnit()   const;
    double        scaleWidth()  const;
    double        scaleHeight() const;

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // ADV_PRINT_OPTIONS_PAGE_H
