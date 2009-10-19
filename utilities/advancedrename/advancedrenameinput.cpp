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
#include <QTimer>
#include <QToolButton>

// KDE includes

#include <klocale.h>

// Local includes

#include "parser.h"

namespace Digikam
{

class AdvancedRenameInputPriv
{
public:

    AdvancedRenameInputPriv() :
        userIsTyping(false),
        userIsHighlighting(false),
        tokenMarked(false),
        selectionStart(-1),
        selectionLength(-1),
        curCursorPos(-1),
        parseTimer(0),
        parser(0)
    {}

    bool    userIsTyping;
    bool    userIsHighlighting;
    bool    tokenMarked;

    int     selectionStart;
    int     selectionLength;
    int     curCursorPos;

    QTimer* parseTimer;
    Parser* parser;
};

AdvancedRenameInput::AdvancedRenameInput(QWidget* parent)
                    : KLineEdit(parent), d(new AdvancedRenameInputPriv)
{
    setFocusPolicy(Qt::StrongFocus);
    setClearButtonShown(true);
    setCompletionMode(KGlobalSettings::CompletionAuto);
    setClickMessage(i18n("Enter renaming string (without extension)"));
    setToolTip(i18n("<p>Hold CTRL and move the mouse over the line edit widget to highlight token words.<br/>"
                    "Hold SHIFT and move the mouse to highlight a token and its modifiers.<br/>"
                    "To mark a token, press the left mouse button while it is highlighted."
                    "</p>"));

    d->curCursorPos = cursorPosition();

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

AdvancedRenameInput::~AdvancedRenameInput()
{
    delete d;
}

void AdvancedRenameInput::setParser(Parser* parser)
{
    if (parser)
    {
        d->parser = parser;
    }
}

void AdvancedRenameInput::mouseMoveEvent(QMouseEvent* e)
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

void AdvancedRenameInput::mousePressEvent(QMouseEvent* e)
{
    if (
            (e->modifiers() == Qt::ControlModifier)                                             ||
//            ((e->modifiers() & (Qt::ControlModifier)) && (e->modifiers())& (Qt::ShiftModifier)) ||
            (e->modifiers() & Qt::ShiftModifier)
       )
    {
        if (e->button() == Qt::LeftButton)
        {
            if (d->userIsTyping)
            {
                return;
            }

            if (selectionStart() == d->selectionStart)
            {
                d->tokenMarked    = true;
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

void AdvancedRenameInput::leaveEvent(QEvent* e)
{
    rememberSelection();
    KLineEdit::leaveEvent(e);
}

void AdvancedRenameInput::focusInEvent(QFocusEvent* e)
{
    KLineEdit::focusInEvent(e);

    if (tokenIsSelected())
    {
        setSelection(d->selectionStart, d->selectionLength);
    }
}

void AdvancedRenameInput::focusOutEvent(QFocusEvent* e)
{
    rememberSelection();
    KLineEdit::focusOutEvent(e);
}

void AdvancedRenameInput::rememberSelection()
{
    if (hasSelectedText())
    {
        d->selectionStart  = selectionStart();
        d->selectionLength = selectedText().count();
    }
    else
    {
        deselect();
        resetSelection();
        setCursorPosition(d->curCursorPos);
    }
}

void AdvancedRenameInput::searchAndHighlightTokens(SelectionType type, int pos)
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
        setSelectionColor(type);

        deselect();
        setSelection(start, length);

        d->selectionStart  = start;
        d->selectionLength = length;
    }
    else
    {
        deselect();
        resetSelection();
    }
}

bool AdvancedRenameInput::tokenIsSelected()
{
    bool selected = (d->selectionStart != -1) && (d->selectionLength != -1) && d->tokenMarked;
    return selected;
}

void AdvancedRenameInput::slotTextChanged()
{
    d->userIsTyping = true;
    d->parseTimer->start();
}

void AdvancedRenameInput::slotParseTimer()
{
    d->userIsTyping = false;
    emit signalTextChanged(text());
}

void AdvancedRenameInput::slotCursorPositionChanged(int oldPos, int newPos)
{
    Q_UNUSED(oldPos)

    if (d->userIsTyping)
    {
        d->curCursorPos = newPos;
    }
    resetSelection();
}

void AdvancedRenameInput::resetSelection()
{
    d->tokenMarked     = false;
    d->selectionStart  = -1;
    d->selectionLength = -1;
    setSelectionColor(Text);
    emit signalTokenMarked(d->tokenMarked);
}

void AdvancedRenameInput::slotAddToken(const QString& token)
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

void AdvancedRenameInput::slotAddModifier(const QString& token)
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

void AdvancedRenameInput::setSelectionColor(SelectionType type)
{
    QString cssTemplate("QLineEdit { selection-background-color: %1; selection-color: %2;}");
    QString css;

    switch (type)
    {
        case Token:
            css = cssTemplate.arg("red").arg("white");
            break;
        case TokenAndModifiers:
            css = cssTemplate.arg("yellow").arg("black");
            break;
        case Text:
            css = cssTemplate.arg("palette(highlight)").arg("palette(highlighted-text)");
    }
    setStyleSheet(css);
}

}  // namespace Digikam
