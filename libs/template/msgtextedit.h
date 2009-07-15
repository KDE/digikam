/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-07-15
 * Description : a text edit widget with click message.
 * 
 * Copyright (C) 2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef MSGTEXTEDIT_H
#define MSGTEXTEDIT_H

// Qt includes.

#include <QWidget>
#include <QString>

// KDE includes.

#include <ktextedit.h>

namespace Digikam
{

class MsgTextEditPriv;

class MsgTextEdit : public KTextEdit
{
    Q_OBJECT

public:

    MsgTextEdit(QWidget *parent);
    ~MsgTextEdit();

    void    setClickMessage(const QString& msg);
    QString clickMessage() const;

    void setText(const QString& txt);

protected:

    void paintEvent(QPaintEvent*);
    void dropEvent(QDropEvent*);
    void focusInEvent(QFocusEvent*);
    void focusOutEvent(QFocusEvent*);

private :

    MsgTextEditPriv* d;
};

}  // namespace Digikam

#endif /* MSGTEXTEDIT_H */
