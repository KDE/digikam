/* ============================================================
 * File  : imagecommentedit.h
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-03-09
 * Description :
 *
 * Copyright 2003 by Renchi Raju
 *
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
 
#ifndef IMAGEDESCEDIT_H
#define IMAGEDESCEDIT_H

#include <kdialogbase.h>
#include <qstring.h>

class QLabel;
class QTextEdit;

class ImageCommentEdit : public KDialogBase
{
    Q_OBJECT

public:

    ImageCommentEdit(const QString& itemName,
                  const QString& itemComments,
                  QWidget *parent=0);
    ~ImageCommentEdit();

    static bool editComments(const QString& itemName,
                             QString& itemComments,
                             QWidget *parent=0);

private:

    QLabel* mNameLabel;
    QTextEdit* mCommentsEdit;
    QString mItemName;

private slots:

    void slot_textChanged();

};

#endif
