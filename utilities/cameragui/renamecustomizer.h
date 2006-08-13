/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Caulier Gilles <caulier dot gilles at kdemail dot net>
 * Date   : 2004-09-19
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
#include <qdatetime.h>

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
    QString newName(const QDateTime &date, int index, const QString &suffix) const;
    Case    changeCase() const;
    int     startIndex() const;

signals:

    void signalChanged();
    
public slots:

    void setFocusToCustomPrefix();

private:

    void readSettings();
    void saveSettings();
    
private slots:

    void slotRadioButtonClicked(int);
    void slotRenameOptionsChanged();
    void slotCustomOptionsActived(int);

private:
    
    RenameCustomizerPriv *d;
};

}  // namespace Digikam

#endif /* RENAMECUSTOMIZER_H */
