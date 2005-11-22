/* ============================================================
 * File  : imagedialogbase.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-07-23
 * Description : simple plugins dialog without threadable 
 *               filter interface. The dialog laytou is 
 *               designed to accept custom widgets in 
 *               preview and settings area.
 * 
 * Copyright 2005 by Gilles Caulier
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

#ifndef IMAGEDIALOGBASE_H
#define IMAGEDIALOGBASE_H

// Qt includes

#include <qstring.h>

// KDE includes.

#include <kdialogbase.h>

class QGridLayout;

class KAboutData;

namespace DigikamImagePlugins
{

class ImageDialogBase : public KDialogBase
{
    Q_OBJECT

public:

    ImageDialogBase(QWidget *parent, QString title, QString name, bool loadFileSettings=true);
    ~ImageDialogBase();

    void setAboutData(KAboutData *about);
    void setPreviewAreaWidget(QWidget *w);
    void setUserAreaWidget(QWidget *w);
    
private:

    QGridLayout    *m_mainLayout;
    
    QWidget        *m_parent;
    
    QString         m_name;
    
private slots:

    void slotHelp();
};

}  // NameSpace DigikamImagePlugins

#endif /* IMAGEDIALOGBASE */
