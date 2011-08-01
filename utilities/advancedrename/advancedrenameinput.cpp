/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-09-03
 * Description : an input widget for the AdvancedRename utility
 *
 * Copyright (C) 2009-2010 by Andi Clemens <andi dot clemens at gmx dot net>
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
#include <QScrollBar>
#include <QTimer>

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

namespace
{
static const short INVALID = -1;
}

namespace Digikam
{

AdvancedRenameLineEditProxy::AdvancedRenameLineEditProxy(QWidget* parent)
    : ProxyLineEdit(parent)
{
    setClearButtonShown(true);
}

void AdvancedRenameLineEditProxy::setWidget(QWidget* widget)
{
    delete m_widget;
    delete m_layout;

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

class AdvancedRenameLineEdit::AdvancedRenameLineEditPriv
{
public:

    AdvancedRenameLineEditPriv() :
        allowDirectoryCreation(false),
        verticalSliderPosition(INVALID),
        parseTimer(0),
        parser(0)
    {}

    bool    allowDirectoryCreation;
    int     verticalSliderPosition;
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

void AdvancedRenameLineEdit::setParseTimerDuration(int milliseconds)
{
    d->parseTimer->setInterval(milliseconds);
}

void AdvancedRenameLineEdit::setParser(Parser* parser)
{
    if (parser)
    {
        d->parser = parser;
    }
}

Parser* AdvancedRenameLineEdit::parser() const
{
    return d->parser;
}

void AdvancedRenameLineEdit::keyPressEvent(QKeyEvent* e)
{
    switch (e->key())
    {
        // avoid newlines in the new name
        case Qt::Key_Enter:
        case Qt::Key_Return:
        {
            emit signalReturnPressed();
            break;
        }

        // the keys "Up, Down, PageUp, PageDown" should be send to the QComboBox
        case Qt::Key_Up:
        case Qt::Key_PageUp:
        case Qt::Key_Down:
        case Qt::Key_PageDown:
        {
            e->setAccepted(false);
            break;
        }

        // the key "/" should not be allowed (QTextEdit is not able to use a QValidator, so we must do it in here)
        case Qt::Key_Slash:
        {
            if (!d->allowDirectoryCreation)
            {
                // do nothing
            }

            break;
        }

        default:
        {
            QTextEdit::keyPressEvent(e);
            break;
        }
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
        ParseSettings settings;
        settings.parseString = toPlainText();
        int start            = INVALID;
        int length           = INVALID;
        int pos              = textCursor().position();
        found                = d->parser->tokenAtPosition(Parser::OptionToken,
                                                          settings, pos, start, length);
        found                = found && ( (start + length) == pos );

        if (!found)
        {
            found = d->parser->tokenAtPosition(Parser::OptionModifiersToken,
                                               settings, pos, start, length);
            found = found && ( (start + length) == pos );
        }
    }

    emit signalTokenMarked(found);
}

void AdvancedRenameLineEdit::slotSetText(const QString& text)
{
    clear();
    setPlainText(text);
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::EndOfLine);
    setTextCursor(cursor);
    setFocus();
}

// --------------------------------------------------------

class AdvancedRenameInput::AdvancedRenameInputPriv
{
public:

    AdvancedRenameInputPriv() :
        maxVisibleItems(10),
        maxHistoryItems(30),
        lineEdit(0),
        highlighter(0)
    {}

    static const QString    configGroupName;
    static const QString    configPatternHistoryListEntry;

    const int               maxVisibleItems;
    const int               maxHistoryItems;

    QStringList             patternHistory;

    AdvancedRenameLineEdit* lineEdit;
    Highlighter*            highlighter;
};
const QString AdvancedRenameInput::AdvancedRenameInputPriv::configGroupName("AdvancedRename Input");
const QString AdvancedRenameInput::AdvancedRenameInputPriv::configPatternHistoryListEntry("Pattern History List");

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

    connect(d->lineEdit, SIGNAL(signalTextChanged(QString)),
            proxy, SLOT(setText(QString)));

    connect(d->lineEdit, SIGNAL(signalTextChanged(QString)),
            this, SIGNAL(signalTextChanged(QString)));

    connect(d->lineEdit, SIGNAL(signalTokenMarked(bool)),
            this, SIGNAL(signalTokenMarked(bool)));

    connect(d->lineEdit, SIGNAL(signalReturnPressed()),
            this, SIGNAL(signalReturnPressed()));

    connect(this, SIGNAL(activated(QString)),
            d->lineEdit, SLOT(slotSetText(QString)));

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
    enableHighlighter(true);
}

void AdvancedRenameInput::setParseTimerDuration(int milliseconds)
{
    d->lineEdit->setParseTimerDuration(milliseconds);
}

void AdvancedRenameInput::setText(const QString& text)
{
    d->lineEdit->slotSetText(text);
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
    d->lineEdit->ensureCursorVisible();
}

void AdvancedRenameInput::slotHighlightLineEdit()
{
    d->lineEdit->selectAll();
}

void AdvancedRenameInput::slotHighlightLineEdit(const QString& word)
{
    QTextCursor cursor = d->lineEdit->textCursor();
    cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::MoveAnchor);
    d->lineEdit->setTextCursor(cursor);
    d->lineEdit->find(word, QTextDocument::FindCaseSensitively);
}

void AdvancedRenameInput::enableHighlighter(bool enable)
{
    delete d->highlighter;
    d->highlighter = enable
                     ? new Highlighter(d->lineEdit, d->lineEdit->parser())
                     : 0;
}

void AdvancedRenameInput::changeEvent(QEvent* e)
{
    KComboBox::changeEvent(e);

    if (e->type() == QEvent::EnabledChange)
    {
        enableHighlighter(isEnabled());
    }
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
    slotSetFocus();
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
