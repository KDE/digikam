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

#include <QMouseEvent>
#include <QFocusEvent>
#include <QToolButton>
#include <QHBoxLayout>
#include <QTimer>

// KDE includes

#include <kiconloader.h>
#include <klocale.h>

// Local includes

#include "parser.h"

namespace Digikam
{

class AdvancedRenameLineEditPriv
{
public:

    AdvancedRenameLineEditPriv()
    {
        userIsTyping    = false;
        userIsMarking   = false;
        tokenMarked     = false;
        selectionStart  = -1;
        selectionLength = -1;
        curCursorPos    = -1;
        markedTokenPos  = -1;
        parseTimer      = 0;
        parser          = 0;
    }

    bool    userIsTyping;
    bool    userIsMarking;
    bool    tokenMarked;

    int     selectionStart;
    int     selectionLength;
    int     curCursorPos;
    int     markedTokenPos;

    QTimer* parseTimer;
    Parser* parser;
};

AdvancedRenameLineEdit::AdvancedRenameLineEdit(QWidget* parent)
                    : KLineEdit(parent), d(new AdvancedRenameLineEditPriv)
{
    setFocusPolicy(Qt::StrongFocus);
    setClearButtonShown(true);
    setCompletionMode(KGlobalSettings::CompletionAuto);
    setClickMessage(i18n("Enter renaming string (without extension)"));
    setToolTip(i18n("<p>Hold CTRL and move the mouse over the line edit widget to highlight token words.<br/>"
                    "To mark a token, press the left mouse button while it is highlighted."
//                    "<br/>"
//                    "Marked tokens can be moved around with the control buttons."
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

AdvancedRenameLineEdit::~AdvancedRenameLineEdit()
{
    delete d;
}

void AdvancedRenameLineEdit::setParser(Parser* parser)
{
    if (parser)
        d->parser = parser;
}

void AdvancedRenameLineEdit::mouseMoveEvent(QMouseEvent* e)
{
    KLineEdit::mouseMoveEvent(e);
    if (e->modifiers() == Qt::ControlModifier)
    {
        if (d->userIsTyping)
            return;

        int start;
        int length;
        int pos = cursorPositionAt(e->pos());

        bool found = d->parser->tokenAtPosition(text(), pos, start, length);
        if (found)
        {
            d->markedTokenPos = start;
            highlightToken(pos);
        }
    }
    else if (d->tokenMarked)
    {
        d->userIsMarking = false;
    }
    else
    {
        deselect();
        d->markedTokenPos = -1;
        d->userIsMarking  = false;
        d->tokenMarked    = false;
        setCursorPosition(d->curCursorPos);
    }
}

void AdvancedRenameLineEdit::mousePressEvent(QMouseEvent* e)
{
    if (e->modifiers() == Qt::ControlModifier)
    {
        if (e->button() == Qt::LeftButton)
        {
            if (d->userIsTyping)
                return;

            if (selectionStart() == d->markedTokenPos)
            {
                d->tokenMarked = true;
                emit signalTokenMarked(true);
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
        KLineEdit::mousePressEvent(e);
    }
}

void AdvancedRenameLineEdit::focusInEvent(QFocusEvent* e)
{
    KLineEdit::focusInEvent(e);
    setCursorPosition(d->curCursorPos);

    if (tokenIsSelected())
    {
        setSelection(d->selectionStart, d->selectionLength);
    }
}

void AdvancedRenameLineEdit::focusOutEvent(QFocusEvent* e)
{
    if (hasSelectedText() && tokenIsSelected())
    {
        d->selectionStart  = selectionStart();
        d->selectionLength = selectedText().count();
    }
    else
    {
        d->selectionStart  = -1;
        d->selectionLength = -1;
    }
    d->curCursorPos = cursorPosition();

    KLineEdit::focusOutEvent(e);
}

bool AdvancedRenameLineEdit::highlightToken(int pos)
{
    if (!d->userIsMarking)
    {
        d->curCursorPos  = cursorPosition();
        d->userIsMarking = true;
    }

    int start  = 0;
    int length = 0;
    bool found = d->parser->tokenAtPosition(text(), pos, start, length);

    if (found)
    {
        deselect();
        setSelection(start, length);

        d->selectionStart  = start;
        d->selectionLength = length;
    }
    else
    {
        deselect();
        d->selectionStart  = -1;
        d->selectionLength = -1;
    }

    return (found && hasSelectedText());
}

bool AdvancedRenameLineEdit::tokenIsSelected()
{
    bool selected = false;
    selected      = (d->selectionStart != -1) && (d->selectionLength != -1) &&
                    (d->markedTokenPos == d->selectionStart) && d->tokenMarked;
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
        d->curCursorPos = newPos;

    d->tokenMarked = false;
    emit signalTokenMarked(d->tokenMarked);
}

void AdvancedRenameLineEdit::slotAddToken(const QString& token)
{
    if (!token.isEmpty())
    {
        if (tokenIsSelected())
            setSelection(d->selectionStart, d->selectionLength);

        if (hasSelectedText())
            del();

        int cursorPos = cursorPosition();
        QString tmp   = text();
        tmp.insert(cursorPos, token);
        setText(tmp);
        setCursorPosition(cursorPos + token.count());
    }
    setFocus();
}

// --------------------------------------------------------

class AdvancedRenameInputPriv
{
public:

    AdvancedRenameInputPriv()
    {
        moveTokenLeft   =  0;
        moveTokenRight  =  0;
        parserInput     =  0;
        parser          =  0;
    }

    QToolButton*          moveTokenLeft;
    QToolButton*          moveTokenRight;

    AdvancedRenameLineEdit* parserInput;
    Parser*               parser;
};

// --------------------------------------------------------

AdvancedRenameInput::AdvancedRenameInput(QWidget* parent)
                 : QWidget(parent), d(new AdvancedRenameInputPriv)
{
    d->parserInput    = new AdvancedRenameLineEdit;

    d->moveTokenLeft  = new QToolButton;
    d->moveTokenRight = new QToolButton;

    d->moveTokenLeft->setIcon(SmallIcon("arrow-left"));
    d->moveTokenRight->setIcon(SmallIcon("arrow-right"));

    d->moveTokenLeft->setEnabled(false);
    d->moveTokenRight->setEnabled(false);

    d->moveTokenLeft->setVisible(false);
    d->moveTokenRight->setVisible(false);

    d->moveTokenLeft->setToolTip(i18n("Move selected token to the left"));
    d->moveTokenRight->setToolTip(i18n("Move selected token to the right"));

    // --------------------------------------------------------

    QHBoxLayout* mainLayout = new QHBoxLayout;
    mainLayout->addWidget(d->parserInput);
    mainLayout->addWidget(d->moveTokenLeft);
    mainLayout->addWidget(d->moveTokenRight);
    mainLayout->setSpacing(0);
    mainLayout->setMargin(0);
    setLayout(mainLayout);

    // --------------------------------------------------------

    connect(d->parserInput, SIGNAL(signalTokenMarked(bool)),
            d->moveTokenLeft, SLOT(setEnabled(bool)));

    connect(d->parserInput, SIGNAL(signalTokenMarked(bool)),
            d->moveTokenRight, SLOT(setEnabled(bool)));

    connect(d->moveTokenLeft, SIGNAL(clicked()),
            this, SLOT(slotMoveTokenLeft()));

    connect(d->moveTokenRight, SIGNAL(clicked()),
            this, SLOT(slotMoveTokenRight()));

    connect(d->parserInput, SIGNAL(signalTextChanged(const QString&)),
            this, SIGNAL(signalTextChanged(const QString&)));
}

AdvancedRenameInput::~AdvancedRenameInput()
{
    delete d;
}

AdvancedRenameLineEdit* AdvancedRenameInput::input() const
{
    return d->parserInput;
}

void AdvancedRenameInput::slotAddToken(const QString& token)
{
    d->parserInput->slotAddToken(token);
}

void AdvancedRenameInput::slotMoveTokenLeft()
{
//    if (d->selectionStart == -1 || d->selectionLength == -1)
//        return;
//
//    d->parserInput->setSelection(d->selectionStart, d->selectionLength);
//
//    int curPos = d->parserInput->selectionStart() - 1;
//    int pos;
//    int length;
//
//    bool found = d->parserInput->findToken(curPos, pos, length);
//    int newPos = found ? pos - 1 : curPos;
//
//    d->parserInput->cut();
//    d->parserInput->setCursorPosition(newPos);
//    d->parserInput->paste();
//
//    d->selectionStart = newPos;
//    d->parserInput->setSelection(d->selectionStart, d->selectionLength);
}

void AdvancedRenameInput::slotMoveTokenRight()
{
//    if (d->selectionStart == -1 || d->selectionLength == -1)
//        return;
//
//    d->parserInput->setSelection(d->selectionStart, d->selectionLength);
//
//    int curPos = d->parserInput->selectionStart() + d->selectionLength;
//    int pos;
//    int length;
//
//    bool found = d->parserInput->findToken(curPos, pos, length);
//    int newPos = found ? pos + length : curPos;
//
//    d->parserInput->cut();
//    d->parserInput->setCursorPosition(newPos);
//    d->parserInput->paste();
//
//    d->selectionStart = newPos;
//    d->parserInput->setSelection(d->selectionStart, d->selectionLength);
}

}  // namespace Digikam
