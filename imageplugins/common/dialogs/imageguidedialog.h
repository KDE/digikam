/* ============================================================
 * File  : imageguidedialog.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-05-07
 * Description : A threaded filter plugin dialog with a preview 
 *               image guide widget and a settings user area
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

#ifndef IMAGEGUIDEDIALOG_H
#define IMAGEGUIDEDIALOG_H

// Qt includes

#include <qstring.h>

// KDE include.

#include <kdialogbase.h>

// Digikam includes.

#include <digikamheaders.h>

class QTimer;
class QGridLayout;
class QSpinBox;

class KAboutData;
class KProgress;
class KColorButton;

namespace DigikamImagePlugins
{

class ImageGuideDialog : public KDialogBase
{
    Q_OBJECT

public:

    ImageGuideDialog(QWidget* parent, QString title, QString name, 
                     bool loadFileSettings=false, bool progress=true, 
                     bool guideVisible=true, 
                     int guideMode=Digikam::ImageGuideWidget::HVGuideMode);
    ~ImageGuideDialog();

    void setAboutData(KAboutData *about);
    void setUserAreaWidget(QWidget *w);
    
public:
        
    Digikam::ThreadedFilter   *m_threadedFilter;
    
    Digikam::ImageGuideWidget *m_imagePreviewWidget;

            
public slots: 

    void slotTimer();       
    void slotEffect();
    void slotOk();

private:
    
    enum RunningMode
    {
    NoneRendering=0,
    PreviewRendering,
    FinalRendering
    };
    
    int           m_currentRenderingMode;

    QWidget      *m_parent;
    
    QTimer       *m_timer;
    
    QString       m_name;

    QGridLayout  *m_mainLayout;
    
    QSpinBox     *m_guideSize;

    KProgress    *m_progressBar;
        
    KColorButton *m_guideColorBt;

    KAboutData   *m_about;
    
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
};

}  // NameSpace DigikamImagePlugins

#endif /* IMAGEGUIDEDIALOG_H */
