/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-07
 * Description : A threaded filter plugin dialog with a preview 
 *               image guide widget and a settings user area
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

#ifndef IMAGEGUIDEDLG_H
#define IMAGEGUIDEDLG_H

// Qt includes

#include <QString>
#include <QEvent>
#include <QKeyEvent>
#include <QCloseEvent>

// KDE include.

#include <kdialog.h>

// Local includes.

#include "imagewidget.h"
#include "imageguidewidget.h"
#include "digikam_export.h"

class KAboutData;

namespace Digikam
{

class ImageGuideDlgPriv;
class DImgThreadedFilter;

class DIGIKAM_EXPORT ImageGuideDlg : public KDialog
{
    Q_OBJECT

public:

    ImageGuideDlg(QWidget* parent, QString title, QString name,
                  bool loadFileSettings=false, 
                  bool progress=true,
                  bool guideVisible=true,
                  int  guideMode=ImageGuideWidget::HVGuideMode,
                  bool prevModeOptions=false,
                  bool useImageSelection=false,
                  bool tryAction=false);
    ~ImageGuideDlg();

    void setAboutData(KAboutData *about);
    void setUserAreaWidget(QWidget *w);
    void setProgressVisible(bool v);

public:

    DImgThreadedFilter *m_threadedFilter;

    ImageWidget        *m_imagePreviewWidget;

public slots: 

    void slotTimer();
    void slotEffect();
    void slotOk();
    void slotTry();

protected slots:

    virtual void slotButtonClicked(int button);
    virtual void slotCancel();
    virtual void slotUser1();
    virtual void slotUser2(){};
    virtual void slotUser3(){};
    virtual void slotDefault();
    virtual void slotInit();
    virtual void readUserSettings(){ slotDefault(); };

    void slotFilterStarted();
    void slotFilterFinished(bool success);
    void slotFilterProgress(int progress);

private slots:

    void slotResized();
    void slotHelp();

protected:

    void closeEvent(QCloseEvent *e);
    void abortPreview();
    void readSettings();
    void writeSettings();
    void keyPressEvent(QKeyEvent *e);

    virtual void writeUserSettings(){};
    virtual void resetValues(){};
    virtual void prepareEffect(){};
    virtual void prepareFinal(){};
    virtual void putPreviewData(){};
    virtual void putFinalData(){};
    virtual void renderingFinished(){};

private:

    ImageGuideDlgPriv* d;
};

}  // NameSpace Digikam

#endif /* IMAGEGUIDEDLG_H */
