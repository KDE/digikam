/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-07
 * Description : A threaded filter control panel dialog for 
 *               image editor plugins using DImg
 * 
 * Copyright (C) 2005-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <QString>
#include <QEvent>
#include <QKeyEvent>
#include <QCloseEvent>

// KDE include.

#include <kdialog.h>

// Local includes

#include "imagepannelwidget.h"
#include "digikam_export.h"

namespace Digikam
{

class CtrlPanelDlgPriv;
class DImgThreadedFilter;

class DIGIKAM_EXPORT CtrlPanelDlg : public KDialog
{
    Q_OBJECT

public:

    CtrlPanelDlg(QWidget* parent, QString title, QString name,
                 bool loadFileSettings=false, 
                 bool tryAction=false, 
                 bool progressBar=true,
                 int separateViewMode=ImagePannelWidget::SeparateViewAll);
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

protected slots:

    virtual void slotButtonClicked(int button);
    void slotFilterStarted();
    void slotFilterFinished(bool success);
    void slotFilterProgress(int progress);

private slots:

    virtual void slotDefault();
    virtual void slotCancel();
    virtual void slotUser1();
    virtual void slotUser2(){};
    virtual void slotUser3(){};
    virtual void slotInit();
    virtual void readUserSettings(void){ slotDefault(); };

    void slotHelp();
    void slotFocusChanged(void);

protected:

    void closeEvent(QCloseEvent *e);
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
