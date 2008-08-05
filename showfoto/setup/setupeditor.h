/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-04-02
 * Description : setup showfoto tab.
 *
 * Copyright (C) 2005-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef SETUPEDITOR_H
#define SETUPEDITOR_H

// Qt includes.

#include <qwidget.h>

namespace ShowFoto
{

class SetupEditorPriv;

class SetupEditor : public QWidget
{
    Q_OBJECT

public:

    SetupEditor(QWidget* parent=0);
    ~SetupEditor();

    void applySettings();

private slots:

    void slotThemeBackgroundColor(bool);

private:

    void readSettings();

private:

    SetupEditorPriv* d;
};

}   // namespace ShowFoto

#endif /* SETUPEDITOR_H */
