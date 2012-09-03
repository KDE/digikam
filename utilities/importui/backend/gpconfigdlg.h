/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-09-07
 * Description : Gphoto2 camera config dialog
 *
 * Copyright (C) 2007-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef GPCONFIGDLG_H
#define GPCONFIGDLG_H

// C ANSI includes

extern "C"
{
#include <gphoto2.h>
}

// KDE includes

#include <kdialog.h>

class QWidget;

namespace Digikam
{

class GPConfigDlgPrivate;

class GPConfigDlg : public KDialog
{
    Q_OBJECT

public:

    GPConfigDlg(Camera* camera, CameraWidget* widget, QWidget* parent = 0);
    ~GPConfigDlg();

private Q_SLOTS:

    void slotOk();

private:

    void appendWidget(QWidget* parent, CameraWidget* widget);
    void updateWidgetValue(CameraWidget* widget);

private:

    GPConfigDlgPrivate* const d;
};

}  // namespace Digikam

#endif // GPCONFIGDLG_H
