/* ============================================================
 * File  : ctrlpaneldlg.h
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2005-05-07
 * Description : A threaded filter control panel dialog for 
 *               image editor plugins using DImg
 * 
 * Copyright 2005-2006 by Gilles Caulier
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

#ifndef CTRLPANELDLG_H
#define CTRLPANELDLG_H

// Qt includes

#include <qstring.h>

// KDE include.

#include <kdialogbase.h>

// Local includes

#include "imagepannelwidget.h"
#include "digikam_export.h"

class QFrame;

namespace Digikam
{

class CtrlPanelDlgPriv;
class DImgThreadedFilter;

class DIGIKAM_EXPORT CtrlPanelDlg : public KDialogBase
{
    Q_OBJECT

public:

    CtrlPanelDlg(QWidget* parent, QString title, QString name,
                 bool loadFileSettings=false, bool tryAction=false, bool progressBar=true,
                 int separateViewMode=ImagePannelWidget::SeparateViewAll,
                 QFrame* bannerFrame=0);
    ~CtrlPanelDlg();

    void setAboutData(KAboutData *about);

public:
        
    ImagePannelWidget  *m_imagePreviewWidget;

    DImgThreadedFilter *m_threadedFilter;
            
public slots: 

    void slotTimer();       
    void slotEffect();
    void slotOk();
    void slotTry();

private slots:
    
    virtual void slotDefault();
    virtual void slotCancel();
    virtual void slotUser1();
    virtual void slotInit();
    virtual void readUserSettings(void){ slotDefault(); };
     
    void slotHelp();
    void slotFocusChanged(void);    
    
protected:

    void closeEvent(QCloseEvent *e);
    void customEvent(QCustomEvent *event);
    void abortPreview(void);
    void keyPressEvent(QKeyEvent *e);

    virtual void writeUserSettings(void){};            
    virtual void resetValues(void){};
    virtual void prepareEffect(void){};
    virtual void prepareFinal(void){};
    virtual void putPreviewData(void){};
    virtual void putFinalData(void){};
    virtual void renderingFinished(void){};

private:
    
    CtrlPanelDlgPriv* d;
};

}  // NameSpace Digikam

#endif /* CTRLPANELDLG_H */
