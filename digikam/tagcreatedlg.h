/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Caulier Gilles <caulier dot gilles at kdemail dot net>
 * Date   : 2004-07-01
 * Description : dialog to edit and create digiKam Tags
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

#ifndef TAGCREATEDLG_H
#define TAGCREATEDLG_H

// Qt includes.

#include <qstring.h>

// KDE includes.

#include <kdialogbase.h>

class QComboBox;
class QLineEdit;
class QPushButton;
class QWidget;

namespace Digikam
{

class TagCreateDlgPriv;
class TagEditDlgPriv;

class TagCreateDlg : public KDialogBase
{
    Q_OBJECT

public:

    TagCreateDlg(QWidget *parent, TAlbum* album);
    ~TagCreateDlg();

    QString title() const;
    QString icon() const;

    static bool tagCreate(QWidget *parent, TAlbum* album, QString& title, QString& icon);

private slots:

    void slotIconChange();
    void slotTitleChanged(const QString& newtitle);
    
private:

    TagCreateDlgPriv *d;
};

// -------------------------------------------------------------------------------------

class TagEditDlg : public KDialogBase
{
    Q_OBJECT

public:

    TagEditDlg(QWidget *parent, TAlbum* album);
    ~TagEditDlg();

    QString title() const;
    QString icon() const;

    static bool tagEdit(QWidget *parent, TAlbum* album, QString& title, QString& icon);

private slots:

    void slotIconChange();
    void slotIconResetClicked();
    void slotTitleChanged(const QString& newtitle);
    
private:

    TagEditDlgPriv *d;
};

}  // namespace Digikam

#endif /* TAGCREATEDLG_H */
