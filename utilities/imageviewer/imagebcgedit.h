/* ============================================================
 * File  : imagebcgedit.h
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-03-11
 * Description : 
 * 
 * Copyright 2003 by Renchi Raju

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

#ifndef IMAGEBCGEDIT_H
#define IMAGEBCGEDIT_H

#include <kdialogbase.h>

class ImageBCGEdit : public KDialogBase
{
    Q_OBJECT

public:

    ImageBCGEdit(QWidget* parent);
    ~ImageBCGEdit();

signals:

    void signalBrightnessIncrease();
    void signalBrightnessDecrease();
    void signalContrastIncrease();
    void signalContrastDecrease();
    void signalGammaIncrease();
    void signalGammaDecrease();

protected slots:
    
    void slotUser1();
};

#endif /* IMAGEBCGEDIT_H */
