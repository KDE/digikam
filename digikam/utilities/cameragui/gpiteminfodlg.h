/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-09-30
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju

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

#ifndef GPITEMINFODLG_H
#define GPITEMINFODLG_H

#include <kdialogbase.h>

class GPItemInfo;

class GPItemInfoDlg : public KDialogBase
{
public:

    GPItemInfoDlg(QWidget* parent, const GPItemInfo* itemInfo);
    ~GPItemInfoDlg();
};

#endif /* GPITEMINFODLG_H */
