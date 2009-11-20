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

#include "advancedrenameinput.moc"

// Qt includes

#include <QFontMetrics>
#include <QLayout>
#include <QTextEdit>
#include <QTimer>

// KDE includes

#include <kapplication.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdialog.h>
#include <klocale.h>

// Local includes

#include "comboboxutilities.h"
#include "defaultrenameparser.h"
#include "highlighter.h"
#include "parser.h"

// const variables

const int INVALID = -1;

namespace Digikam
{

class AdvancedRenameLineEditPriv
{
public:

    AdvancedRenameLineEditPriv() :
        parseTimer(0),
        parser(0)
    {}

    QTimer* parseTimer;
    Parser* parser;
};

AdvancedRenameLineEdit::AdvancedRenameLineEdit(QWidget* parent)
                      : QTextEdit(parent), d(new AdvancedRenameLineEditPriv)
{
    setLineWrapMode(QTextEdit::NoWrap);
    setWordWrapMode(QTextOption::NoWrap);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameStyle(QFrame::NoFrame);
    setPalette(kapp->palette());
    setFocusPolicy(Qt::StrongFocus);

    viewport()->setAutoFillBackground(false);
    setAutoFillBackground(false);

    QFontMetrics fm = fontMetrics();
    setFixedHeight(fm.height());

    // --------------------------------------------------------

    d->parseTimer = new QTimer(this);
    d->parseTimer->setInterval(500);
    d->parseTimer->setSingleShot(true);

    // --------------------------------------------------------

    connect(d->parseTimer, SIGNAL(timeout()),
            this, SLOT(slotParseTimer()));

    connect(this, SIGNAL(textChanged()),
            this, SLOT(slotTextChanged()));

    connect(this, SIGNAL(cursorPositionChanged()),
            this, SLOT(slotCursorPositionChanged()));
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

Parser* AdvancedRenameLineEdit::parser()
{
    return d->parser;
}

void AdvancedRenameLineEdit::keyPressEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return)
    {
        emit signalReturnPressed();
    }
    else
    {
        QTextEdit::keyPressEvent(e);
    }
}

void AdvancedRenameLineEdit::wheelEvent(QWheelEvent* e)
{
    e->setAccepted(false);
}

void AdvancedRenameLineEdit::slotTextChanged()
{
    d->parseTimer->start();
}

void AdvancedRenameLineEdit::slotParseTimer()
{
    emit signalTextChanged(toPlainText());
}

void AdvancedRenameLineEdit::slotCursorPositionChanged()
{
    bool found = false;

    if (d->parser)
    {
        int start           = INVALID;
        int length          = INVALID;
        QString parseString = toPlainText();
        int pos             = textCursor().position();
        found               = d->parser->tokenAtPosition(Parser::Token,
                                                         parseString, pos, start, length);
        found               = found && ( (start + length) == pos );

        if (!found)
        {
            found = d->parser->tokenAtPosition(Parser::TokenAndModifiers,
                                               parseString, pos, start, length);
            found = found && ( (start + length) == pos );
        }
    }
    emit signalTokenMarked(found);
}

void AdvancedRenameLineEdit::slotSetHistoryItem(const QString& text)
{
    clear();
    setPlainText(text);
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::EndOfLine);
    setTextCursor(cursor);
    setFocus();
}

// --------------------------------------------------------

class AdvancedRenameInputPriv
{
public:

    AdvancedRenameInputPriv() :
        configGroupName("AdvancedRename Input"),
        configPatternHistoryListEntry("Pattern History List"),

        maxVisibleItems(10),
        maxHistoryItems(20),
        lineEdit(0),
        highlighter(0)
    {}

    const QString           configGroupName;
    const QString           configPatternHistoryListEntry;

    const int               maxVisibleItems;
    const int               maxHistoryItems;

    QStringList             patternHistory;

    AdvancedRenameLineEdit* lineEdit;
    Highlighter*            highlighter;
};

// --------------------------------------------------------

AdvancedRenameInput::AdvancedRenameInput(QWidget* parent)
                   : KComboBox(parent), d(new AdvancedRenameInputPriv)
{
    // important: setEditable() has to be called before adding the actual line edit widget, otherwise
    //            our lineEdit gets removed again.
    setEditable(true);
    setMaxVisibleItems(d->maxVisibleItems);
    setMaxCount(d->maxHistoryItems);
    setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);

    d->lineEdit          = new AdvancedRenameLineEdit(this);
    ProxyLineEdit* proxy = new ProxyLineEdit(this);
    proxy->setWidget(d->lineEdit);

    setLineEdit(proxy);
    proxy->setAutoFillBackground(false);

    // --------------------------------------------------------

    connect(d->lineEdit, SIGNAL(signalTextChanged(const QString&)),
            this, SIGNAL(signalTextChanged(const QString&)));

    connect(d->lineEdit, SIGNAL(signalTokenMarked(bool)),
            this, SIGNAL(signalTokenMarked(bool)));

    connect(d->lineEdit, SIGNAL(signalReturnPressed()),
            this, SIGNAL(signalReturnPressed()));

    connect(this, SIGNAL(activated(const QString&)),
            d->lineEdit, SLOT(slotSetHistoryItem(const QString&)));

    // --------------------------------------------------------

    readSettings();
}

AdvancedRenameInput::~AdvancedRenameInput()
{
    writeSettings();
    delete d;
}

void AdvancedRenameInput::setParser(Parser* parser)
{
    d->lineEdit->setParser(parser);

    delete d->highlighter;
    d->highlighter = new Highlighter(d->lineEdit, parser);
}

void AdvancedRenameInput::setText(const QString& text)
{
    d->lineEdit->setPlainText(text);
}

void AdvancedRenameInput::clearText()
{
    d->lineEdit->clear();
}

QString AdvancedRenameInput::text() const
{
    return d->lineEdit->toPlainText();
}

void AdvancedRenameInput::slotAddToken(const QString& token)
{
    d->lineEdit->insertPlainText(token);
    d->lineEdit->setFocus();
    d->lineEdit->ensureCursorVisible();
}

void AdvancedRenameInput::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    d->patternHistory = group.readEntry(d->configPatternHistoryListEntry, QStringList());
    d->patternHistory.removeAll(QString(""));
    addItems(d->patternHistory);
    d->lineEdit->clear();
}

void AdvancedRenameInput::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    // remove duplicate entries and save pattern history, omit empty strings
    QString pattern = d->lineEdit->toPlainText();
    d->patternHistory.removeAll(pattern);
    d->patternHistory.removeAll(QString(""));
    d->patternHistory.prepend(pattern);
    group.writeEntry(d->configPatternHistoryListEntry, d->patternHistory);
}

}  // namespace Digikam
