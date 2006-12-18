/* ============================================================
 * File  : imageeffect_charcoal.h
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2004-08-26
 * Description : a digiKam image editor plugin for 
 *               simulate charcoal drawing.
 * 
 * Copyright 2004-2006 by Gilles Caulier
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

#ifndef IMAGEEFFECT_CHARCOAL_H
#define IMAGEEFFECT_CHARCOAL_H

// Local includes.

#include <digikamheaders.h>

class KIntNumInput;

namespace DigikamCharcoalImagesPlugin
{

class ImageEffect_Charcoal : public Digikam::CtrlPanelDlg
{
    Q_OBJECT

public:

    ImageEffect_Charcoal(QWidget* parent, QString title, QFrame* banner);
    ~ImageEffect_Charcoal();

private:

    KIntNumInput *m_pencilInput;
    KIntNumInput *m_smoothInput;

protected:
    
    void prepareEffect(void);
    void prepareFinal(void);
    void putPreviewData(void);
    void putFinalData(void);
    void resetValues(void);   
    void renderingFinished(void);
};

}  // NameSpace DigikamCharcoalImagesPlugin

#endif /* IMAGEEFFECT_CHARCOAL_H */
