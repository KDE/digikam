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

ManualRenameLineEdit::ManualRenameLineEdit(QWidget* parent)
                 : KLineEdit(parent)
{
    setClearButtonShown(true);
    setCompletionMode(KGlobalSettings::CompletionAuto);
    setClickMessage(i18n("Enter custom rename string"));
    setToolTip(i18n("CTRL+click a token in the line edit widget to mark it."));

    m_userTyping  = false;
    m_tokenMarked = false;

    // --------------------------------------------------------

    m_parseTimer = new QTimer(this);
    m_parseTimer->setInterval(500);
    m_parseTimer->setSingleShot(true);

    m_markTimer = new QTimer(this);
    m_markTimer->setInterval(100);
    m_markTimer->setSingleShot(true);

    // --------------------------------------------------------

    connect(m_parseTimer, SIGNAL(timeout()),
            this, SLOT(slotParseTimer()));

    connect(m_markTimer, SIGNAL(timeout()),
            this, SLOT(slotMarkTimer()));

    connect(this, SIGNAL(textChanged(const QString&)),
            this, SLOT(slotTextChanged()));

    connect(this, SIGNAL(cursorPositionChanged(int, int)),
            this, SLOT(slotCursorPositionChanged()));
}

ManualRenameLineEdit::~ManualRenameLineEdit()
{
}

void ManualRenameLineEdit::setParser(ManualRenameParser* parser)
{
    if (parser)
        m_parser = parser;
}

void ManualRenameLineEdit::mousePressEvent(QMouseEvent* e)
{
    KLineEdit::mousePressEvent(e);
    if (e->modifiers() == Qt::ControlModifier && e->button() == Qt::LeftButton)
    {
        if (m_userTyping)
            return;
        m_markTimer->start();
    }
}

void ManualRenameLineEdit::highlightTokens()
{
//    int pos    = 0;
//    int length = 0;
//    bool found = false;
//
//    QString text = d->parseStringLineEdit->text();
//
//    for (int i = 0; i < text.count(); ++i)
//    {
//        found = findToken(d->parseStringLineEdit->cursorPosition(), pos, length);
//        if (found)
//        {
//            i = pos + length;
//        }
//        else
//        {
//        }
//    }
}

bool ManualRenameLineEdit::findToken(int curPos, int& pos, int& length)
{
    if (!m_parser)
        return false;

    ManualRenameParser::TokenMap map = m_parser->tokenMap();
    QMapIterator<QString, QString> it(map);

    bool found = false;

    while (it.hasNext())
    {
        it.next();
        QString key = it.key();
        QStringList k = key.split(":", QString::SkipEmptyParts);
        if (!k.count() == 2)
            continue;

        length  = k.last().toInt();
        pos     = k.first().toInt();

        if ((curPos >= pos) && (curPos <= pos + length))
        {
            found = true;
            break;
        }
    }
    return found;
}

void ManualRenameLineEdit::slotMarkTimer()
{
    if (hasSelectedText())
        return;

    int pos    = 0;
    int length = 0;
    bool found = findToken(cursorPosition(), pos, length);

    if (found)
    {
        deselect();
        setSelection(pos, length);

        m_selectionStart  = pos;
        m_selectionLength = length;
    }
    else
    {
        deselect();
        m_selectionStart  = -1;
        m_selectionLength = -1;
    }


    m_tokenMarked = found && hasSelectedText();
    emit signalTokenMarked(m_tokenMarked);
}

void ManualRenameLineEdit::slotTextChanged()
{
    m_userTyping = true;
    m_parseTimer->start();
}

void ManualRenameLineEdit::slotParseTimer()
{
    m_userTyping = false;
    highlightTokens();
    emit signalTextChanged(text());
}

void ManualRenameLineEdit::slotCursorPositionChanged()
{
    m_tokenMarked = false;
    emit signalTokenMarked(m_tokenMarked);
}

void ManualRenameLineEdit::slotAddToken(const QString& token)
{
    if (!token.isEmpty())
    {
        int cursorPos = cursorPosition();
        QString tmp   = text();
        tmp.insert(cursorPos, token);
        setText(tmp);
        setCursorPosition(cursorPos + token.count());
    }
    setFocus();
}

void ManualRenameLineEdit::slotMoveTokenLeft()
{
    if (m_selectionStart == -1 || m_selectionLength == 1)
        return;

    setSelection(m_selectionStart, m_selectionLength);

    int curPos = selectionStart() - 1;
    int pos;
    int length;

    bool found = findToken(curPos, pos, length);
    int newPos = found ? pos - 1 : curPos;

    cut();
    setCursorPosition(newPos);
    paste();

    m_selectionStart = newPos;
    setSelection(m_selectionStart, m_selectionLength);
}

void ManualRenameLineEdit::slotMoveTokenRight()
{
    if (m_selectionStart == -1 || m_selectionLength == 1)
        return;

    setSelection(m_selectionStart, m_selectionLength);

    int curPos = selectionStart() + m_selectionLength;
    int pos;
    int length;

    bool found = findToken(curPos, pos, length);
    int newPos = found ? pos + length : curPos;

    cut();
    setCursorPosition(newPos);
    paste();

    m_selectionStart = newPos;
    setSelection(m_selectionStart, m_selectionLength);
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
        userTyping      =  false;
        parser          =  0;
        selectionStart  = -1;
        selectionLength = -1;
    }

    bool                  userTyping;

    int                   selectionStart;
    int                   selectionLength;

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
    if (!token.isEmpty())
    {
        int cursorPos = d->parserInput->cursorPosition();
        QString tmp   = d->parserInput->text();
        tmp.insert(cursorPos, token);
        d->parserInput->setText(tmp);
        d->parserInput->setCursorPosition(cursorPos + token.count());
    }
    d->parserInput->setFocus();
}

void ManualRenameInput::slotMoveTokenLeft()
{
    if (d->selectionStart == -1 || d->selectionLength == 1)
        return;

    d->parserInput->setSelection(d->selectionStart, d->selectionLength);

    int curPos = d->parserInput->selectionStart() - 1;
    int pos;
    int length;

    bool found = d->parserInput->findToken(curPos, pos, length);
    int newPos = found ? pos - 1 : curPos;

    d->parserInput->cut();
    d->parserInput->setCursorPosition(newPos);
    d->parserInput->paste();

    d->selectionStart = newPos;
    d->parserInput->setSelection(d->selectionStart, d->selectionLength);
}

void ManualRenameInput::slotMoveTokenRight()
{
    if (d->selectionStart == -1 || d->selectionLength == 1)
        return;

    d->parserInput->setSelection(d->selectionStart, d->selectionLength);

    int curPos = d->parserInput->selectionStart() + d->selectionLength;
    int pos;
    int length;

    bool found = d->parserInput->findToken(curPos, pos, length);
    int newPos = found ? pos + length : curPos;

    d->parserInput->cut();
    d->parserInput->setCursorPosition(newPos);
    d->parserInput->paste();

    d->selectionStart = newPos;
    d->parserInput->setSelection(d->selectionStart, d->selectionLength);
}

}  // namespace ManualRename
}  // namespace Digikam
