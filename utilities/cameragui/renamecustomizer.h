/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-09-19
 * Description : a options group to set renaming files
 *               operations during camera downloading
 * 
 * Copyright 2004-2005 by Renchi Raju
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

#ifndef RENAMECUSTOMIZER_H
#define RENAMECUSTOMIZER_H

// Qt includes.

#include <qbuttongroup.h>

namespace Digikam
{

class RenameCustomizerPriv;

class RenameCustomizer : public QButtonGroup
{
    Q_OBJECT

public:

    enum Case
    {
        NONE = 0,
        UPPER,
        LOWER
    };

    enum CustomOptions
    {
        ADDDATETIME = 0,
        ADDSEQNUMB,
        ADDBOTH
    };

    RenameCustomizer(QWidget* parent);
    ~RenameCustomizer();

    void    setUseDefault(bool val);
    bool    useDefault() const;
    QString nameTemplate() const;
    Case    changeCase() const;

signals:

    void signalChanged();
    
private:

    void readSettings();
    void saveSettings();
    
private slots:

    void slotRadioButtonClicked(int);
    void slotPrefixChanged(const QString&);
    void slotCustomOptionsChanged(const QString&);
    void slotCaseTypeChanged(const QString&);

private:
    
    RenameCustomizerPriv *d;
};

}  // namespace Digikam

#endif /* RENAMECUSTOMIZER_H */
