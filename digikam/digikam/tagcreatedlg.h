/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-07-01
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

#ifndef TAGCREATEDLG_H
#define TAGCREATEDLG_H

#include <kdialogbase.h>
#include <qstring.h>

class QComboBox;
class QLineEdit;
class QPushButton;

class TagCreateDlg : public KDialogBase
{
    Q_OBJECT

public:

    TagCreateDlg(TAlbum* parent);
    ~TagCreateDlg();

    QString title() const;
    QString icon() const;

    static bool tagCreate(TAlbum* parent, QString& title,
                          QString& icon);

private slots:

    void slotIconChange();
    void slotTitleChanged(const QString& newtitle);
    
private:

    QLineEdit*    m_titleEdit;
    QString       m_icon;
    QPushButton*  m_iconButton;
};

class TagEditDlg : public KDialogBase
{
    Q_OBJECT

public:

    TagEditDlg(TAlbum* album);
    ~TagEditDlg();

    QString title() const;
    QString icon() const;

    static bool tagEdit(TAlbum* album, QString& title,
                        QString& icon);

private slots:

    void slotIconChange();
    void slotTitleChanged(const QString& newtitle);
    
private:

    QLineEdit*    m_titleEdit;
    QString       m_icon;
    QPushButton*  m_iconButton;
};

#endif /* TAGCREATEDLG_H */
