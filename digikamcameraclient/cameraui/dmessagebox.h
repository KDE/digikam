/* ============================================================
 * File  : dmessagebox.h
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-02-22
 * Description : 
 * 
 * Copyright 2003 by Renchi Raju

 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published bythe Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * ============================================================ */

#ifndef DMESSAGEBOX_H
#define DMESSAGEBOX_H

// Digikam Message Box
// If One Message Box is already open, and more messages are posted
//  they will be appended to the open messagebox

#include <qwidget.h>
#include <qstring.h>

class QLabel;
class QTextEdit;

class DMessageBox : public QWidget
{
    Q_OBJECT

public:

    DMessageBox();
    ~DMessageBox();

    void appendMsg(const QString& msg);
    static void showMsg(const QString& msg);
    
private:

    static DMessageBox* s_instance;

    int        count_;
    QLabel    *msgBox_;
    QTextEdit *extraMsgBox_;
    QString    mainMsg_;

private slots:

    void slotOkClicked();
    
};

#endif /* DMESSAGEBOX_H */
