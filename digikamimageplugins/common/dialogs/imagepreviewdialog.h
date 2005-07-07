/* ============================================================
 * File  : imagepreviewdialog.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-05-07
 * Description : A simple plugin dialog with a preview widget 
 *               and a settings user area
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

#ifndef IMAGEPREVIEWDIALOG_H
#define IMAGEPREVIEWDIALOG_H

// Qt includes

#include <qstring.h>

// KDE include.

#include <kdialogbase.h>

class QTimer;
class QGridLayout;

class KAboutData;
class KProgress;

namespace Digikam
{
class ImageWidget;
class ThreadedFilter;
}

namespace DigikamImagePlugins
{

class ImagePreviewDialog : public KDialogBase
{
    Q_OBJECT

public:

    ImagePreviewDialog(QWidget* parent, QString title, QString name, 
                       bool loadFileSettings=false, bool progress=true);
    ~ImagePreviewDialog();

    void setAboutData(KAboutData *about);
    void setUserAreaWidget(QWidget *w);
    
public:
        
    Digikam::ThreadedFilter *m_threadedFilter;
    
    Digikam::ImageWidget    *m_imagePreviewWidget;

            
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
    
    int                   m_currentRenderingMode;

    QWidget              *m_parent;
    
    QTimer               *m_timer;
    
    QString               m_name;

    QGridLayout          *m_mainLayout;

    KProgress            *m_progressBar;
    
private slots:
    
    virtual void slotCancel();
    virtual void slotUser1();

    void slotResized();           
    void slotHelp();
    void slotInit();
    
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

#endif /* IMAGEPREVIEWDIALOG_H */
