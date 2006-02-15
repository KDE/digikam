/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-06-05
 * Description : 
 * 
 * Copyright 2004-2005 by Renchi Raju
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

#ifndef IMAGERESIZEDLG_H
#define IMAGERESIZEDLG_H

// KDE includes.

#include <kdialogbase.h>

namespace Digikam
{
class ImageResizeDlgPriv;

class ImageResizeDlg : public KDialogBase
{
    Q_OBJECT

public:

    ImageResizeDlg(QWidget* parent, int* width, int* height);
    ~ImageResizeDlg();

private slots:

    void slotOk();
    void slotChanged();

private:

    ImageResizeDlgPriv *d;
};

}  // namespace Digikam

#endif /* IMAGERESIZEDLG_H */
