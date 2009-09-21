/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-07-14
 * Description : main digiKam theme designer window
 *
 * Copyright (C) 2005 by Renchi Raju <renchi at pooh.tam.uiuc.edu>
 * Copyright (C) 2007-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

// Qt includes

#include <QWidget>
#include <QMap>

// KDE includes

#include <kdialog.h>

class KComboBox;
class QCheckBox;
class QLabel;
class KColorButton;

namespace Digikam
{

class ImagePropertiesTab;
class FolderView;
class ThemedIconView;
class Theme;

class MainWindowPriv;

class MainWindow : public KDialog
{
    Q_OBJECT

public:

    enum PROPERTY
    {
        BASE = 0,
        REGULARTEXT,
        SELECTEDTEXT,
        REGULARSPECIALTEXT,
        SELECTEDSPECIALTEXT,
        BANNER,
        THUMBNAILREGULAR,
        THUMBNAILSELECTED,
        LISTVIEWREGULAR,
        LISTVIEWSELECTED
    };

    enum BEVEL
    {
        FLAT = 0,
        RAISED,
        SUNKEN
    };

    enum GRADIENT
    {
        SOLID = 0,
        HORIZONTAL,
        VERTICAL,
        DIAGONAL
    };

public:

    MainWindow();
    ~MainWindow();

private Q_SLOTS:

    void slotLoad();
    void slotSave();
    void slotPropertyChanged();
    void slotUpdateTheme();

private:

    MainWindowPriv* const d;
};

}  // namespace Digikam

#endif // MAINWINDOW_H
