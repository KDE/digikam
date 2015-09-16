/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2009-07-15
 * @brief  a text edit widget with click message.
 *
 * @author Copyright (C) 2009-2015 by Gilles Caulier
 *         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "msgtextedit.h"

// Qt includes

#include <QtGui/QPalette>
#include <QtGui/QPainter>

namespace Digikam
{

class Q_DECL_HIDDEN MsgTextEdit::Private
{
public:

    Private()
    {
    }

    QString message;
};

MsgTextEdit::MsgTextEdit(QWidget* const parent)
    : KTextEdit(parent), d(new Private)
{
    setAcceptRichText(false);
}

MsgTextEdit::~MsgTextEdit()
{
    delete d;
}

QString MsgTextEdit::clickMessage() const
{
    return d->message;
}

void MsgTextEdit::setClickMessage(const QString& msg)
{
    d->message = msg;
    viewport()->update();
}

void MsgTextEdit::setText(const QString& txt)
{
    KTextEdit::setText(txt);
    viewport()->update();
}

void MsgTextEdit::paintEvent(QPaintEvent* e)
{
    KTextEdit::paintEvent(e);

    if (toPlainText().isEmpty() && !hasFocus())
    {
        QPainter p(viewport());
        QPen tmp = p.pen();
        p.setPen(palette().color(QPalette::Disabled, QPalette::Text));
        QRect cr = contentsRect();
        p.drawText(cr, Qt::AlignTop, d->message);
        p.setPen(tmp);
    }
}

void MsgTextEdit::dropEvent(QDropEvent* e)
{
    viewport()->update();
    KTextEdit::dropEvent(e);
}

void MsgTextEdit::focusInEvent(QFocusEvent* e)
{
    viewport()->update();
    KTextEdit::focusInEvent(e);
}

void MsgTextEdit::focusOutEvent(QFocusEvent* e)
{
    viewport()->update();
    KTextEdit::focusOutEvent(e);
}

}  // namespace Digikam
