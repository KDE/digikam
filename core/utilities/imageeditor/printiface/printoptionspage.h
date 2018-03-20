/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-06
 * Description : image editor printing interface.
 *
 * Copyright (C) 2009 by Angelo Naselli <anaselli at linux dot it>
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

#ifndef PRINT_OPTIONS_PAGE_H
#define PRINT_OPTIONS_PAGE_H

// Qt includes

#include <QWidget>

namespace Digikam
{

class IccProfile;

class PrintOptionsPage : public QWidget
{
    Q_OBJECT

public:

    enum ScaleMode
    {
        NoScale,
        ScaleToPage,
        ScaleToCustomSize
    };

    /** Order should match the content of the unit combbox in the ui file
     */
    enum Unit
    {
        Millimeters,
        Centimeters,
        Inches
    };

public:

    PrintOptionsPage(QWidget* const parent, const QSize& imageSize);
    ~PrintOptionsPage();

    void loadConfig();
    void saveConfig();

    bool          enlargeSmallerImages() const;
    bool          colorManaged()         const;
    bool          autoRotation()         const;
    Qt::Alignment alignment()            const;
    IccProfile    outputProfile()        const;

    ScaleMode     scaleMode()            const;
    Unit          scaleUnit()            const;
    double        scaleWidth()           const;
    double        scaleHeight()          const;

private Q_SLOTS:

    void adjustWidthToRatio();
    void adjustHeightToRatio();
    void slotAlertSettings(bool);
    void slotSetupDlg();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // PRINT_OPTIONS_PAGE_H
