/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-06
 * Description : image editor printing interface.
 *               inspired from  Gwenview code (Aurelien Gateau).
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

#ifndef PRINTOPTIONSPAGE_H
#define PRINTOPTIONSPAGE_H

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

    // Order should match the content of the unit combbox in the ui file
    enum Unit
    {
        Millimeters,
        Centimeters,
        Inches
    };

    PrintOptionsPage(QWidget* parent, const QSize& imageSize );
    ~PrintOptionsPage();

    Qt::Alignment alignment() const;
    ScaleMode scaleMode() const;
    bool enlargeSmallerImages() const;
    Unit scaleUnit() const;
    double scaleWidth() const;
    double scaleHeight() const;
    bool colorManaged() const;
    bool autoRotation() const;
    IccProfile outputProfile() const;

    void loadConfig();
    void saveConfig();

private Q_SLOTS:

    void adjustWidthToRatio();
    void adjustHeightToRatio();
    void slotAlertSettings(bool);
    void slotSetupDlg();

private:

    class PrintOptionsPagePrivate;
    PrintOptionsPagePrivate* const d;
};

} // namespace Digikam

#endif /* PRINTOPTIONSPAGE_H */
