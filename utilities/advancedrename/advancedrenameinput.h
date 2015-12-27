/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-09-03
 * Description : an input widget for the AdvancedRename utility
 *
 * Copyright (C) 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
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

#ifndef ADVANCEDRENAMEINPUT_H
#define ADVANCEDRENAMEINPUT_H

// Qt includes

#include <QPlainTextEdit>
#include <QWidget>
#include <QComboBox>

// Local includes

#include "comboboxutilities.h"
#include "parser.h"

class QFocusEvent;
class QKeyEvent;
class QMouseEvent;
class QEvent;

namespace Digikam
{

class AdvancedRenameLineEdit : public QPlainTextEdit
{
    Q_OBJECT

public:

    explicit AdvancedRenameLineEdit(QWidget* const parent = 0);
    ~AdvancedRenameLineEdit();

    void    setParser(Parser* parser);
    Parser* parser() const;

    void setParseTimerDuration(int milliseconds);

public Q_SLOTS:

    void slotSetText(const QString&);
    void slotCursorPositionChanged();

Q_SIGNALS:

    void signalTextChanged(const QString&);
    void signalTokenMarked(bool);
    void signalReturnPressed();

protected:

    virtual void keyPressEvent(QKeyEvent* e);
    virtual void wheelEvent(QWheelEvent* e);
    virtual void scrollContentsBy(int dx, int dy);

private Q_SLOTS:

    void slotTextChanged();
    void slotParseTimer();

private:

    AdvancedRenameLineEdit(const AdvancedRenameLineEdit&);
    AdvancedRenameLineEdit& operator=(const AdvancedRenameLineEdit&);

    void setupWidgets();
    void setupConnections();

private:

    class Private;
    Private* const d;
};

// --------------------------------------------------------

class AdvancedRenameInput : public QComboBox
{
    Q_OBJECT

public:

    explicit AdvancedRenameInput(QWidget* const parent = 0);
    ~AdvancedRenameInput();

    void setParser(Parser* parser);
    void setParseTimerDuration(int milliseconds);

    QString text() const;
    void    setText(const QString& text);

Q_SIGNALS:

    void signalTextChanged(const QString&);
    void signalTokenMarked(bool);
    void signalReturnPressed();

public Q_SLOTS:

    void slotAddToken(const QString&);
    void slotClearText();
    void slotClearTextAndHistory();
    void slotSetFocus();

    void slotHighlightLineEdit();
    void slotHighlightLineEdit(const QString& word);

protected:

    virtual void changeEvent(QEvent* e);

private Q_SLOTS:

    void slotClearButtonPressed();
    void slotTextChanged(const QString& text);

private:

    AdvancedRenameInput(const AdvancedRenameInput&);
    AdvancedRenameInput& operator=(const AdvancedRenameInput&);

    void readSettings();
    void writeSettings();
    void enableHighlighter(bool enable);

    void setupWidgets();
    void setupConnections();

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* ADVANCEDRENAMEINPUT_H */
