/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-08-23
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju

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

#ifndef SETUPMISC_H
#define SETUPMISC_H

#include <qwidget.h>

class QCheckBox;

class SetupMisc : public QWidget
{
public:

    SetupMisc(QWidget* parent);
    ~SetupMisc();

    void applySettings();

private:

    void readSettings();

    QCheckBox* m_showSplashCheck;
    QCheckBox* m_useTrashCheck;
};

#endif /* SETUPMISC_H */
