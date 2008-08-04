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

// Qt includes.

#include <qstring.h>

// KDE includes.

#include <kdialogbase.h>

// Local includes.

#include <imageinfo.h>

class QCloseEvent;
class QCustomEvent;
class QTimer;

namespace KDcrawIface
{
class DcrawSettingsWidget;
}

namespace Digikam
{

class DImg;
class RawPreview;
class ActionThread;
class SaveSettingsWidget;
class RawImportDlgPriv;

class RawImportDlg : public KDialogBase
{
    Q_OBJECT

public:

    RawImportDlg(const ImageInfo& info, QWidget *parent);
    ~RawImportDlg();

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

    void slotDefault();
    void slotClose();
    void slotHelp();
    void slotUser1();
    void slotUser2();
    void slotUser3();

    void slotChannelChanged(int channel);
    void slotScaleChanged(int scale);

private:

    RawImportDlgPriv *d;
};

} // NameSpace Digikam

#endif // RAWIMPORTDLG_H
