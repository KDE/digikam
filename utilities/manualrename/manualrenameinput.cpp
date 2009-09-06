/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-09-03
 * Description : an input widget for the ManualRenameParser
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

#include "manualrenameinput.h"
#include "manualrenameinput.moc"

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

#include "manualrenameparser.h"

using namespace Digikam::ManualRename;
namespace Digikam
{
namespace ManualRename
{

class ManualRenameLineEditPriv
{
public:

    ManualRenameLineEditPriv()
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

    bool                userIsTyping;
    bool                userIsMarking;
    bool                tokenMarked;

    int                 selectionStart;
    int                 selectionLength;
    int                 curCursorPos;
    int                 markedTokenPos;

    QTimer*             parseTimer;
    ManualRenameParser* parser;
};

ManualRenameLineEdit::ManualRenameLineEdit(QWidget* parent)
                    : KLineEdit(parent), d(new ManualRenameLineEditPriv)
{
    setClearButtonShown(true);
    setCompletionMode(KGlobalSettings::CompletionAuto);
    setClickMessage(i18n("Enter custom rename string"));
    setToolTip(i18n("<p>Hold CTRL and move the mouse over the line edit widget to highlight token words.<br/>"
                    "To mark a token, press the left mouse button while it is highlighted.<br/>"
                    "Marked tokens can be moved around with the control buttons.</p>"));

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

ManualRenameLineEdit::~ManualRenameLineEdit()
{
}

void ManualRenameLineEdit::setParser(ManualRenameParser* parser)
{
    if (parser)
        d->parser = parser;
}

void ManualRenameLineEdit::mouseMoveEvent(QMouseEvent* e)
{
    KLineEdit::mouseMoveEvent(e);
    if (e->modifiers() == Qt::ControlModifier)
    {
        if (d->userIsTyping)
            return;

        int pos;
        int length;
        int curPos = cursorPositionAt(e->pos());

        bool found = findToken(curPos, pos, length);
        if (found)
        {
            d->markedTokenPos = pos;
            highlightToken(curPos);
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

void ManualRenameLineEdit::mousePressEvent(QMouseEvent* e)
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

void ManualRenameLineEdit::focusOutEvent(QFocusEvent* e)
{
    Q_UNUSED(e)
}

bool ManualRenameLineEdit::highlightToken(int cursorPos)
{
    if (!d->userIsMarking)
    {
        d->curCursorPos  = cursorPosition();
        d->userIsMarking = true;
    }

    int pos    = 0;
    int length = 0;
    bool found = findToken(cursorPos, pos, length);

    if (found)
    {
        deselect();
        setSelection(pos, length);

        d->selectionStart  = pos;
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

bool ManualRenameLineEdit::findToken(int curPos)
{
    int pos;
    int length;
    return findToken(curPos, pos, length);
}

bool ManualRenameLineEdit::findToken(int curPos, int& pos, int& length)
{
    if (!d->parser)
        return false;

    ManualRenameParser::TokenMap map = d->parser->tokenMap(text());
    QMapIterator<QString, QString> it(map);

    bool found = false;

    while (it.hasNext())
    {
        it.next();
        QString keys        = it.key();
        QStringList keylist = keys.split(":", QString::SkipEmptyParts);

        if (!keylist.count() == 2)
            continue;

        length = keylist.last().toInt();
        pos    = keylist.first().toInt();

        if ((curPos >= pos) && (curPos <= pos + length))
        {
            found = true;
            break;
        }
    }
    return found;
}

void ManualRenameLineEdit::slotTextChanged()
{
    d->userIsTyping = true;
    d->parseTimer->start();
}

void ManualRenameLineEdit::slotParseTimer()
{
    d->userIsTyping = false;
    emit signalTextChanged(text());
}

void ManualRenameLineEdit::slotCursorPositionChanged(int oldPos, int newPos)
{
    Q_UNUSED(oldPos)

    if (d->userIsTyping)
        d->curCursorPos = newPos;

    d->tokenMarked = false;
    emit signalTokenMarked(d->tokenMarked);
}

void ManualRenameLineEdit::slotAddToken(const QString& token)
{
    if (!token.isEmpty())
    {
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

class ManualRenameInputPriv
{
public:

    ManualRenameInputPriv()
    {
        moveTokenLeft   =  0;
        moveTokenRight  =  0;
        parserInput     =  0;
        parser          =  0;
    }

    QToolButton*          moveTokenLeft;
    QToolButton*          moveTokenRight;

    ManualRenameLineEdit* parserInput;
    ManualRenameParser*   parser;
};

// --------------------------------------------------------

ManualRenameInput::ManualRenameInput(QWidget* parent)
                 : QWidget(parent), d(new ManualRenameInputPriv)
{
    d->parserInput    = new ManualRenameLineEdit;

    d->moveTokenLeft  = new QToolButton;
    d->moveTokenRight = new QToolButton;

    d->moveTokenLeft->setIcon(SmallIcon("arrow-left"));
    d->moveTokenRight->setIcon(SmallIcon("arrow-right"));

    d->moveTokenLeft->setEnabled(false);
    d->moveTokenRight->setEnabled(false);

    d->moveTokenLeft->setVisible(false);
    d->moveTokenRight->setVisible(false);

    QString moveTokenTooltip = i18nc("%1: direction", "Move selected token to the %1");
    d->moveTokenLeft->setToolTip(moveTokenTooltip.arg(i18nc("move to the left", "left")));
    d->moveTokenRight->setToolTip(moveTokenTooltip.arg(i18nc("move to the right", "right")));

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

ManualRenameInput::~ManualRenameInput()
{
    delete d;
}

ManualRenameLineEdit* ManualRenameInput::input() const
{
    return d->parserInput;
}

void ManualRenameInput::slotAddToken(const QString& token)
{
    d->parserInput->slotAddToken(token);
}

void ManualRenameInput::slotMoveTokenLeft()
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

void ManualRenameInput::slotMoveTokenRight()
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

}  // namespace ManualRename
}  // namespace Digikam
