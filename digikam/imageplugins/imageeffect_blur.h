/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-07-09
 * Description : Blur image filter for ImageEditor
 * 
 * Copyright 2004-2005 by Gilles Caulier
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

#ifndef IMAGEEFFECT_BLUR_H
#define IMAGEEFFECT_BLUR_H

// KDE include.

#include <kdialogbase.h>

class QTimer;

class KIntNumInput;

namespace Digikam
{
class ImagePreviewWidget;
}

class ImageEffect_Blur : public KDialogBase
{
    Q_OBJECT

public:

    ImageEffect_Blur(QWidget *parent);
    ~ImageEffect_Blur();

protected:

    void closeEvent(QCloseEvent *e);
        
private:

    bool          m_cancel;
    
    QWidget      *m_parent;
    
    QTimer       *m_timer;
            
    KIntNumInput *m_radiusInput;
    
    Digikam::ImagePreviewWidget *m_imagePreviewWidget;
    
private slots:

    void slotEffect();
    void slotOk();
    void slotTimer();
    void slotCancel();
};

#endif /* IMAGEEFFECT_BLUR_H */
