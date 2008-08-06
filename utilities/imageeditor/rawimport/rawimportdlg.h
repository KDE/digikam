/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.kipi-plugins.org
 *
 * Date        : 2008-08-04
 * Description : Raw import dialog
 *
 * Copyright (C) 2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef RAWIMPORTDLG_H
#define RAWIMPORTDLG_H

// KDE includes.

#include <kdialogbase.h>
#include <kurl.h>

// Local includes

#include "dimg.h"
#include "digikam_export.h"

class QCloseEvent;

namespace KDcrawIface
{
class RawDecodingSettings;
}

namespace Digikam
{

class RawImportDlgPriv;

class DIGIKAM_EXPORT RawImportDlg : public KDialogBase
{
    Q_OBJECT

public:

    RawImportDlg(const KURL& info, QWidget *parent);
    ~RawImportDlg();

    DRawDecoding rawDecodingSettings();

protected:

    void closeEvent(QCloseEvent *e);

private:

    void readSettings();
    void saveSettings();

    void busy(bool busy);

private slots:

    void slotImageLoaded(const DImg& img);
    void slotLoadingFailed();
    void slotLoadingStarted();
    void slotLoadingProgress(float progress);

    void slotDefault();
    void slotClose();
    void slotOk();
    void slotUser1();
    void slotUser2();

    void slotChannelChanged(int channel);
    void slotScaleChanged(int scale);
    void slotColorsChanged(int color);

    void slotSixteenBitsImageToggled(bool);

private:

    RawImportDlgPriv *d;
};

} // NameSpace Digikam

#endif // RAWIMPORTDLG_H
