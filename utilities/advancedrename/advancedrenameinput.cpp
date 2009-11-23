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
#include <QTimer>
#include <QScrollBar>

// KDE includes

#include <kapplication.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdialog.h>
#include <klocale.h>

// Local includes

#include "highlighter.h"
#include "parser.h"

// const variables

const int INVALID = -1;

namespace Digikam
{

AdvancedRenameLineEditProxy::AdvancedRenameLineEditProxy(QWidget* parent)
                           : ProxyLineEdit(parent)
{
    setClearButtonShown(true);
}

void AdvancedRenameLineEditProxy::setWidget(QWidget *widget)
{
    if (m_widget)
    {
        delete m_widget;
    }

    if (m_layout)
    {
        delete m_layout;
    }

    m_widget = widget;
    m_widget->setParent(this);

    QWidget* placeholder = new QWidget(this);
    placeholder->setFixedHeight(1);
    placeholder->setFixedWidth(clearButtonUsedSize().width());

    QGridLayout* mainLayout = new QGridLayout(this);
    mainLayout->addWidget(m_widget,    0, 0, 1, 1);
    mainLayout->addWidget(placeholder, 0, 1, 1, 1);
    mainLayout->setSpacing(0);
    mainLayout->setMargin(0);
    setLayout(mainLayout);
    updateGeometry();
}

void AdvancedRenameLineEditProxy::mousePressEvent(QMouseEvent* event)
{
    KLineEdit::mousePressEvent(event);
}

void AdvancedRenameLineEditProxy::mouseReleaseEvent(QMouseEvent* event)
{
    KLineEdit::mouseReleaseEvent(event);
}

// --------------------------------------------------------

class AdvancedRenameLineEditPriv
{
public:

    AdvancedRenameLineEditPriv() :
        verticalSliderPosition(INVALID),
        parseTimer(0),
        parser(0)
    {}

    int     verticalSliderPosition;
    QTimer* parseTimer;
    Parser* parser;
};

AdvancedRenameLineEdit::AdvancedRenameLineEdit(QWidget* parent)
                      : KTextEdit(parent), d(new AdvancedRenameLineEditPriv)
{
    setLineWrapMode(KTextEdit::NoWrap);
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

    // layout widget correctly by setting a dummy text and calling ensureCursorVisible().
    // Save the scrollbar position now, to avoid scrolling of the text when selecting with the mouse
    setPlainText("DUMMY TEXT");
    ensureCursorVisible();
    d->verticalSliderPosition = verticalScrollBar()->value();
    clear();

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
        KTextEdit::keyPressEvent(e);
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

void AdvancedRenameLineEdit::scrollContentsBy(int dx, int dy)
{
    Q_UNUSED(dx)
    Q_UNUSED(dy)

    if (d->verticalSliderPosition != INVALID)
    {
        verticalScrollBar()->setValue(d->verticalSliderPosition);
    }
    viewport()->update();
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

    AdvancedRenameLineEditProxy* proxy = new AdvancedRenameLineEditProxy(this);
    d->lineEdit                        = new AdvancedRenameLineEdit(this);
    proxy->setWidget(d->lineEdit);

    setLineEdit(proxy);
    proxy->setAutoFillBackground(false);

    // --------------------------------------------------------

    connect(proxy, SIGNAL(clearButtonClicked()),
            this, SLOT(slotClearButtonPressed()));

    connect(d->lineEdit, SIGNAL(signalTextChanged(const QString&)),
            proxy, SLOT(setText(const QString&)));

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

void AdvancedRenameInput::slotClearText()
{
    d->lineEdit->clear();
}

void AdvancedRenameInput::slotClearTextAndHistory()
{
    d->lineEdit->clear();
    clear();
}

void AdvancedRenameInput::slotSetFocus()
{
    d->lineEdit->setFocus();
}

void AdvancedRenameInput::slotClearButtonPressed()
{
    slotClearText();
    slotSetFocus();
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
