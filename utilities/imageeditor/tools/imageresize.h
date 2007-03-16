/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at gmail dot com>
 * Date   : 2005-04-07
 * Description : a tool to resize a picture
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

#ifndef IMAGE_RESIZE_H
#define IMAGE_RESIZE_H

// Qt include.

#include <qstring.h>

// KDE include.

#include <kdialogbase.h>

// Local includes.

#include "digikam_export.h"

class QCheckBox;
class QCustomEvent;
class QTabWidget;

class KIntNumInput;
class KDoubleNumInput;
class KProgress;

namespace Digikam
{

class GreycstorationIface;
class GreycstorationWidget;
class ImageIface;

class DIGIKAM_EXPORT ImageResize : public KDialogBase
{
    Q_OBJECT

public:

    ImageResize(QWidget* parent);
    ~ImageResize();
       
protected:

    void closeEvent(QCloseEvent *e);
    
private:

    void customEvent(QCustomEvent *event);
    void writeUserSettings();

private slots:

    void slotOk();
    void slotCancel();
    void slotDefault();
    void slotUser2();
    void slotUser3();
    void processCImgURL(const QString&);
    void slotValuesChanged();
    void readUserSettings();        
    void slotRestorationToggled(bool);    

private:

    enum RunningMode
    {
        NoneRendering=0,
        FinalRendering
    };

    int              m_currentRenderingMode;
    int              m_orgWidth;    
    int              m_orgHeight;   
    int              m_prevW; 
    int              m_prevH; 

    double           m_prevWP;    
    double           m_prevHP;    

    QWidget         *m_parent;
    
    QCheckBox       *m_preserveRatioBox;
    QCheckBox       *m_useGreycstorationBox;
    
    QTabWidget      *m_mainTab;

    KIntNumInput    *m_wInput;
    KIntNumInput    *m_hInput;

    KDoubleNumInput *m_wpInput;
    KDoubleNumInput *m_hpInput;
    
    KProgress       *m_progressBar;
    
    ImageIface      *m_iface;

    GreycstorationIface  *m_cimgInterface;
    GreycstorationWidget *m_settingsWidget;
};
    
}  // NameSpace Digikam

#endif /* IMAGE_RESIZE_H */
