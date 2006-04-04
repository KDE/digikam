/* ============================================================
 * File  : imageguidedlg.h
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2005-05-07
 * Description : A threaded filter plugin dialog with a preview 
 *               image guide widget and a settings user area
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

#ifndef IMAGEGUIDEDLG_H
#define IMAGEGUIDEDLG_H

// Qt includes

#include <qstring.h>

// KDE include.

#include <kdialogbase.h>

// Local includes.

#include "imagewidget.h"
#include "imageguidewidget.h"
#include "digikam_export.h"

class QFrame;
class QTimer;
class QGridLayout;
class QSpinBox;

class KAboutData;
class KProgress;
class KColorButton;

namespace Digikam
{

class ImageGuideDlgPriv;
class DImgThreadedFilter;

class DIGIKAM_EXPORT ImageGuideDlg : public KDialogBase
{
    Q_OBJECT

public:

    ImageGuideDlg(QWidget* parent, QString title, QString name,
                  bool loadFileSettings=false, bool progress=true,
                  bool guideVisible=true,
                  int guideMode=ImageGuideWidget::HVGuideMode,
                  QFrame* bannerFrame=0,
                  bool prevModeOptions=false);
    ~ImageGuideDlg();

    void setAboutData(KAboutData *about);
    void setUserAreaWidget(QWidget *w);
    
public:
        
    DImgThreadedFilter *m_threadedFilter;
    
    ImageWidget        *m_imagePreviewWidget;

public slots: 

    void slotTimer();       
    void slotEffect();
    void slotOk();

private slots:
    
    virtual void slotCancel();
    virtual void slotUser1();
    virtual void slotDefault();
    virtual void slotInit();
    virtual void readUserSettings(void){ slotDefault(); };
    
    void slotResized();           
    void slotHelp();
    
protected:

    void closeEvent(QCloseEvent *e);
    void customEvent(QCustomEvent *event);
    void abortPreview(void);
    void readSettings(void);
    void writeSettings(void);
    void keyPressEvent(QKeyEvent *e);
            
    virtual void writeUserSettings(void){};
    virtual void resetValues(void){};
    virtual void prepareEffect(void){};
    virtual void prepareFinal(void){};
    virtual void putPreviewData(void){};
    virtual void putFinalData(void){};
    virtual void renderingFinished(void){};

private:

    void setProgressVisible(bool v);

private:
    
    ImageGuideDlgPriv* d;
};

}  // NameSpace Digikam

#endif /* IMAGEGUIDEDLG_H */
