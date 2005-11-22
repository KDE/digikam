/* ============================================================
 * File  : threadedfilterdialog.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-05-07
 * Description : A basic template of threaded filter plugin 
 *               dialog without widgets.
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

#ifndef THREADEDFILTERDIALOG_H
#define THREADEDFILTERDIALOG_H

// Qt includes

#include <qstring.h>

// KDE include.

#include <kdialogbase.h>

// Digikam includes.

#include <digikamheaders.h>

class QTimer;

class KAboutData;
class KProgress;

namespace DigikamImagePlugins
{

class ThreadedFilterDialog : public KDialogBase
{
    Q_OBJECT

public:

    ThreadedFilterDialog(QWidget* parent, QString title, QString name, 
                         bool loadFileSettings=false);
    ~ThreadedFilterDialog();

    void setAboutData(KAboutData *about);
    
public:
        
    Digikam::ThreadedFilter *m_threadedFilter;
            
public slots: 

    virtual void slotTimer();       
    virtual void slotEffect();
    virtual void slotOk();

private:
    
    QWidget      *m_parent;
    
    QTimer       *m_timer;
    
    QString       m_name;

private slots:
    
    virtual void slotCancel();
    virtual void slotUser1();
    virtual void slotDefault();
    
    void slotHelp();
    
protected:

    enum RunningMode
    {
    NoneRendering=0,
    PreviewRendering,
    FinalRendering
    };
    
    int        m_currentRenderingMode;

    KProgress *m_progressBar;

protected:
    
    void closeEvent(QCloseEvent *e);
    void customEvent(QCustomEvent *event);
    void abortPreview(void);
            
    virtual void resetValues(void){};
    virtual void prepareEffect(void){};
    virtual void prepareFinal(void){};
    virtual void putPreviewData(void){};
    virtual void putFinalData(void){};
    virtual void renderingFinished(void){};
};

}  // NameSpace DigikamImagePlugins

#endif /* THREADEDFILTERDIALOG_H */
