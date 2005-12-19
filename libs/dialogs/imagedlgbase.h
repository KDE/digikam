/* ============================================================
 * File  : imagedlgbase.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-07-23
 * Description : simple plugins dialog without threadable 
 *               filter interface. The dialog layout is 
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

#ifndef IMAGEDLGBASE_H
#define IMAGEDLGBASE_H

// Qt includes

#include <qstring.h>

// KDE includes.

#include <kdialogbase.h>

// Local includes.

#include "digikam_export.h"

class QGridLayout;
class QTimer;
class QWidget;

class KAboutData;

namespace Digikam
{

class DIGIKAM_EXPORT ImageDlgBase : public KDialogBase
{
    Q_OBJECT

public:

    ImageDlgBase(QWidget *parent, QString title, QString name, 
                    bool loadFileSettings=true, QFrame* bannerFrame=0);
    ~ImageDlgBase();

    void setAboutData(KAboutData *about);
    void setPreviewAreaWidget(QWidget *w);
    void setUserAreaWidget(QWidget *w);
    
private:

    QGridLayout    *m_mainLayout;
    
    QWidget        *m_parent;
    
    QString         m_name;

    QTimer         *m_timer;

    KAboutData     *m_about;
    
protected slots:

    void slotTimer();       
    
private slots:
    
    void slotHelp();
    virtual void slotEffect(){};
};

}  // NameSpace Digikam

#endif /* IMAGEDLGBASE */
