/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-08-21
 * Description : Editor tool settings template box
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

#ifndef EDITORTOOLSETTINGS_H
#define EDITORTOOLSETTINGS_H

// Qt includes.

#include <qwidget.h>

// Local includes.

#include "digikam_export.h"

class KPushButton;

namespace Digikam
{

class EditorToolSettingsPriv;

class DIGIKAM_EXPORT EditorToolSettings : public QWidget
{
    Q_OBJECT

public:

    enum ButtonCode
    {
        Default = 0x00000001,
        Try     = 0x00000002,
        Ok      = 0x00000004,
        Cancel  = 0x00000008,
        User1   = 0x00000010,
        User2   = 0x00000020,
        User3   = 0x00000040
    };

public:

    EditorToolSettings(int buttonMask, QWidget *parent=0);
    ~EditorToolSettings();

    virtual void setBusy(bool){};
    virtual void saveSettings(){};
    virtual void readSettings(){};

signals:

    void signalOkClicked();
    void signalCancelClicked();
    void signalTryClicked();

public slots:

    /** Re-implement this slots to reset all settings to defaults values
        when Default button is clicked */
    virtual void slotDefaultSettings(){};

protected:

    int marginHint();
    int spacingHint();

    QWidget *plainPage() const;

    KPushButton* button(int buttonCode) const;

protected slots:

    virtual void slotUser1(){};
    virtual void slotUser2(){};
    virtual void slotUser3(){};

private:

    EditorToolSettingsPriv *d;
};

} // NameSpace Digikam

#endif // EDITORTOOLSETTINGS_H
