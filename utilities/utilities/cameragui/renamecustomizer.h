/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Caulier Gilles <caulier dot gilles at gmail dot com>
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

class QDateTime;

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

    RenameCustomizer(QWidget* parent, const QString& cameraTitle);
    ~RenameCustomizer();

    void    setUseDefault(bool val);
    bool    useDefault() const;
    QString newName(const QDateTime &date, int index, const QString &extension) const;
    Case    changeCase() const;
    int     startIndex() const;

signals:

    void signalChanged();
    
public slots:

    void restoreFocus();

private:

    void readSettings();
    void saveSettings();
    
private slots:

    void slotRadioButtonClicked(int);
    void slotRenameOptionsChanged();
    void slotDateTimeBoxToggled(bool);
    void slotDateTimeFormatChanged(int);
    void slotDateTimeButtonClicked();

private:
    
    RenameCustomizerPriv *d;
};

}  // namespace Digikam

#endif /* RENAMECUSTOMIZER_H */
