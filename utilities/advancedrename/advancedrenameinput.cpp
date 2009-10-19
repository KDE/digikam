/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-09-03
 * Description : an input widget for the AdvancedRename utility
 *
 * Copyright (C) 2009 by Andi Clemens <andi dot clemens at gmx dot net>
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

#include "advancedrenameinput.h"
#include "advancedrenameinput.moc"

// Qt includes

#include <QFocusEvent>
#include <QMouseEvent>
#include <QPalette>
#include <QTimer>
#include <QToolButton>

// KDE includes

#include <kapplication.h>
#include <klocale.h>

// Local includes

#include "parser.h"

namespace Digikam
{

class AdvancedRenameLineEditPriv
{
    typedef AdvancedRenameLineEdit::SelectionType ST;

public:

    AdvancedRenameLineEditPriv() :
        userIsTyping(false),
        userIsHighlighting(false),
        tokenMarked(false),
        selectionStart(-1),
        selectionLength(-1),
        curCursorPos(-1),
        parseTimer(0),
        parser(0),
        selectionType(AdvancedRenameLineEdit::Text)
        {}

    bool    userIsTyping;
    bool    userIsHighlighting;
    bool    tokenMarked;

    int     selectionStart;
    int     selectionLength;
    int     curCursorPos;

    QTimer* parseTimer;
    Parser* parser;

    ST      selectionType;
};

AdvancedRenameLineEdit::AdvancedRenameLineEdit(QWidget* parent)
                      : KLineEdit(parent), d(new AdvancedRenameLineEditPriv)
{
    d->curCursorPos = cursorPosition();

    setFocusPolicy(Qt::StrongFocus);
    setClearButtonShown(true);
    setClickMessage(i18n("Enter renaming string (without extension)"));
    setToolTip(i18n("<p>Hold CTRL and move the mouse over the line edit widget to highlight token words.<br/>"
                    "Hold SHIFT and move the mouse to highlight a token and its modifiers.<br/>"
                    "To mark a token, press the left mouse button while it is highlighted."
                    "</p>"));

    // --------------------------------------------------------

    d->parseTimer = new QTimer(this);
    d->parseTimer->setInterval(500);
    d->parseTimer->setSingleShot(true);

    // --------------------------------------------------------

    connect(d->parseTimer, SIGNAL(timeout()),
            this, SLOT(slotParseTimer()));

    connect(this, SIGNAL(textChanged(const QString&)),
            this, SLOT(slotTextChanged()));

    connect(this, SIGNAL(cursorPositionChanged(int, int)),
            this, SLOT(slotCursorPositionChanged(int, int)));
}

AdvancedRenameLineEdit::~AdvancedRenameLineEdit()
{
    delete d;
}

void AdvancedRenameLineEdit::setParser(Parser* parser)
{
    if (parser)
    {
        d->parser = parser;
    }
}

void AdvancedRenameLineEdit::mouseMoveEvent(QMouseEvent* e)
{
    KLineEdit::mouseMoveEvent(e);
    int pos = cursorPositionAt(e->pos());

    if (e->modifiers() == Qt::ControlModifier)
    {
        searchAndHighlightTokens(Token, pos);
    }
    else if (e->modifiers() & Qt::ShiftModifier)
    {
        searchAndHighlightTokens(TokenAndModifiers, pos);
    }
    else if (d->tokenMarked)
    {
        d->userIsHighlighting = false;
    }
    else if (d->userIsHighlighting)
    {
        deselect();
        d->userIsHighlighting  = false;
        resetSelection();
        setCursorPosition(d->curCursorPos);
    }
}

void AdvancedRenameLineEdit::mousePressEvent(QMouseEvent* e)
{
    if ((e->modifiers() == Qt::ControlModifier) || (e->modifiers() & Qt::ShiftModifier))
    {
        if (e->button() == Qt::LeftButton)
        {
            if (d->userIsTyping)
            {
                return;
            }

            if (selectionStart() == d->selectionStart)
            {
                d->tokenMarked = true;
                emit signalTokenMarked(d->tokenMarked);
            }
            else
            {
                d->tokenMarked = false;
            }
        }
    }
    else
    {
        setCursorPosition(cursorPositionAt(e->pos()));
        d->curCursorPos = cursorPosition();
        resetSelection();
        KLineEdit::mousePressEvent(e);
    }
}

void AdvancedRenameLineEdit::leaveEvent(QEvent* e)
{
    rememberSelection();
    KLineEdit::leaveEvent(e);
}

void AdvancedRenameLineEdit::focusInEvent(QFocusEvent* e)
{
    KLineEdit::focusInEvent(e);

    if (tokenIsSelected())
    {
        setSelection(d->selectionStart, d->selectionLength);
    }
}

void AdvancedRenameLineEdit::focusOutEvent(QFocusEvent* e)
{
    rememberSelection();
    KLineEdit::focusOutEvent(e);
}

void AdvancedRenameLineEdit::rememberSelection()
{
    if ((hasSelectedText() && d->selectionType == Text) ||
        (hasSelectedText() && (d->selectionType ==Token || d->selectionType == TokenAndModifiers) && tokenIsSelected()))
    {
        d->selectionStart  = selectionStart();
        d->selectionLength = selectedText().count();
        d->tokenMarked     = true;
    }
    else
    {
        deselect();
        resetSelection();
        setCursorPosition(d->curCursorPos);
    }
}

void AdvancedRenameLineEdit::searchAndHighlightTokens(SelectionType type, int pos)
{
    if (d->userIsTyping)
    {
        return;
    }

    if (!d->userIsHighlighting)
    {
        d->curCursorPos  = cursorPosition();
        d->userIsHighlighting = true;
    }

    int start;
    int length;

    bool found = false;

    switch (type)
    {
        case Token:
            found = d->parser->tokenAtPosition(text(), pos, start, length);
            break;
        case TokenAndModifiers:
            found = d->parser->tokenModifierAtPosition(text(), pos, start, length);
            break;
        default: break;
    }

    if (found)
    {
        deselect();
        setSelection(start, length);

        d->selectionStart  = start;
        d->selectionLength = length;

        d->selectionType = type;
        setSelectionColor(type);
    }
    else
    {
        deselect();
        resetSelection();
    }
}

bool AdvancedRenameLineEdit::tokenIsSelected()
{
    bool selected = (d->selectionStart != -1) && (d->selectionLength != -1) && d->tokenMarked;
    return selected;
}

void AdvancedRenameLineEdit::slotTextChanged()
{
    d->userIsTyping = true;
    d->parseTimer->start();
}

void AdvancedRenameLineEdit::slotParseTimer()
{
    d->userIsTyping = false;
    emit signalTextChanged(text());
}

void AdvancedRenameLineEdit::slotCursorPositionChanged(int oldPos, int newPos)
{
    Q_UNUSED(oldPos)

    if (d->userIsTyping)
    {
        d->curCursorPos = newPos;
    }
    resetSelection();
}

void AdvancedRenameLineEdit::resetSelection()
{
    d->tokenMarked     = false;
    d->selectionStart  = -1;
    d->selectionLength = -1;
    d->selectionType   = Text;
    setSelectionColor(Text);
    emit signalTokenMarked(d->tokenMarked);
}

void AdvancedRenameLineEdit::slotAddToken(const QString& token)
{
    if (!token.isEmpty())
    {
        if (tokenIsSelected())
        {
            setSelection(d->selectionStart, d->selectionLength);
        }

        if (hasSelectedText())
        {
            del();
            resetSelection();
        }

        int cursorPos = cursorPosition();
        QString tmp   = text();
        tmp.insert(cursorPos, token);
        setText(tmp);
        setCursorPosition(cursorPos + token.count());
    }
    setFocus();
}

void AdvancedRenameLineEdit::slotAddModifier(const QString& token)
{
    if (!token.isEmpty())
    {
        if (tokenIsSelected())
        {
            int cursorPos = cursorPosition();
            QString tmp   = text();
            tmp.insert(cursorPos, token);
            setText(tmp);
            setCursorPosition(cursorPos + token.count());
        }
    }
    setFocus();
}

void AdvancedRenameLineEdit::setSelectionColor(SelectionType type)
{
    QPalette p = palette();

    switch (type)
    {
        case Token:
            p.setColor(QPalette::Active,   QPalette::Highlight,       Qt::red);
            p.setColor(QPalette::Active,   QPalette::HighlightedText, Qt::white);
            p.setColor(QPalette::Inactive, QPalette::Highlight,       Qt::red);
            p.setColor(QPalette::Inactive, QPalette::HighlightedText, Qt::white);
            break;
        case TokenAndModifiers:
            p.setColor(QPalette::Active,   QPalette::Highlight,       Qt::yellow);
            p.setColor(QPalette::Active,   QPalette::HighlightedText, Qt::black);
            p.setColor(QPalette::Inactive, QPalette::Highlight,       Qt::yellow);
            p.setColor(QPalette::Inactive, QPalette::HighlightedText, Qt::black);
            break;
        case Text:
        default:
            p = kapp->palette();
            break;
    }
    setPalette(p);
}

// --------------------------------------------------------

class AdvancedRenameInputPriv
{
public:

    AdvancedRenameInputPriv() :
        lineEdit(0)
    {}

    AdvancedRenameLineEdit* lineEdit;
};

// --------------------------------------------------------

AdvancedRenameInput::AdvancedRenameInput(QWidget* parent)
                   : KComboBox(parent), d(new AdvancedRenameInputPriv)
{
    // important: setEditable() has to be called before adding the actual line edit widget, otherwise
    //            our lineEdit gets removed again.
    setEditable(true);

    d->lineEdit = new AdvancedRenameLineEdit(this);
    setLineEdit(d->lineEdit);

    connect(d->lineEdit, SIGNAL(signalTextChanged(const QString&)),
            this, SIGNAL(signalTextChanged(const QString&)));

    connect(d->lineEdit, SIGNAL(signalTokenMarked(bool)),
            this, SIGNAL(signalTokenMarked(bool)));
}

AdvancedRenameInput::~AdvancedRenameInput()
{
    delete d;
}

AdvancedRenameLineEdit* AdvancedRenameInput::lineEdit()
{
    return d->lineEdit;
}

void AdvancedRenameInput::setParser(Parser* parser)
{
    d->lineEdit->setParser(parser);
}

void AdvancedRenameInput::slotAddToken(const QString& str)
{
    d->lineEdit->slotAddToken(str);
}

void AdvancedRenameInput::slotAddModifier(const QString& str)
{
    d->lineEdit->slotAddModifier(str);
}

}  // namespace Digikam
