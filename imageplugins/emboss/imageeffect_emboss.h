/* ============================================================
 * File  : imageeffect_emboss.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-08-26
 * Description : 
 * 
 * Copyright 2004 by Gilles Caulier
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published bythe Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * ============================================================ */

#ifndef IMAGEEFFECT_EMBOSS_H
#define IMAGEEFFECT_EMBOSS_H

// KDE include.

#include <kdialogbase.h>

class QPushButton;

class KDoubleNumInput;

namespace Digikam
{
class ImagePreviewWidget;
}

namespace DigikamEmbossImagesPlugin
{

class ImageEffect_Emboss : public KDialogBase
{
    Q_OBJECT

public:

    ImageEffect_Emboss(QWidget* parent);
    ~ImageEffect_Emboss();

protected:

    void closeEvent(QCloseEvent *e);
    
private:

    QWidget         *m_parent;
    
    QPushButton     *m_helpButton;
    
    KDoubleNumInput *m_radiusInput;
    KDoubleNumInput *m_sigmaInput;

    Digikam::ImagePreviewWidget *m_imagePreviewWidget;
    
private slots:

    void slotHelp();
    void slotEffect();
    void slotOk();
};

}  // NameSpace DigikamEmbossImagesPlugin

#endif /* IMAGEEFFECT_EMBOSS_H */
