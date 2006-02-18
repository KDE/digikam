/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2004-06-06
 * Description : Red eyes correction tool for image editor
 *
 * Copyright 2004-2005 by Renchi Raju, Gilles Caulier
 * Copyright 2006 by Gilles Caulier
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

#ifndef IMAGEEFFECT_REDEYE_H
#define IMAGEEFFECT_REDEYE_H

// KDE includes.

#include <kdialogbase.h>

class QRadioButton;

namespace DigikamImagesPluginCore
{

class ImageEffect_RedEye
{
public:

    static void removeRedEye(QWidget *parent);
};

class ImageEffect_RedEyeDlg : public KDialogBase
{
    Q_OBJECT

public:

    enum Result
    {
        Mild = 0,
        Aggressive = 1
    };

    ImageEffect_RedEyeDlg(QWidget* parent);
    Result result() const;

private slots:

    void slotClicked(int id);

private:

    int m_selectedId;
};

}  // NameSpace DigikamImagesPluginCore

#endif /* IMAGEEFFECT_REDEYE_H */
