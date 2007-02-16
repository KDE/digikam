/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at gmail dot com>
 * Date   : 2005-07-23
 * Description : simple plugins dialog without threadable 
 *               filter interface. The dialog layout is 
 *               designed to accept custom widgets in 
 *               preview and settings area.
 * 
 * Copyright 2005-2007 by Gilles Caulier
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

class QWidget;

class KAboutData;

namespace Digikam
{

class ImageDlgBasePriv;

class DIGIKAM_EXPORT ImageDlgBase : public KDialogBase
{
    Q_OBJECT

public:

    ImageDlgBase(QWidget *parent, QString title, QString name, 
                 bool loadFileSettings=true, bool tryAction=false, QFrame* bannerFrame=0);
    ~ImageDlgBase();

    void setAboutData(KAboutData *about);
    void setPreviewAreaWidget(QWidget *w);
    void setUserAreaWidget(QWidget *w);

protected slots:

    virtual void slotDefault();
    virtual void slotTimer();       
    
protected:
    
    void closeEvent(QCloseEvent *e);
    virtual void finalRendering(){};
    virtual void writeUserSettings(void){};
    virtual void readUserSettings(void){ slotDefault(); };
    virtual void resetValues(void){};

private slots:
    
    void slotHelp();
    void slotCancel();
    void slotOk();        
    virtual void slotEffect(){};

private:

    void readSettings(void);
    void writeSettings(void);
 
private:

    ImageDlgBasePriv* d;
};

}  // NameSpace Digikam

#endif /* IMAGEDLGBASE */
