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

#ifndef ADVANCEDRENAMEINPUT_H
#define ADVANCEDRENAMEINPUT_H

// Qt includes

#include <QTextEdit>
#include <QWidget>

// KDE includes

#include <kcombobox.h>

// Local includes

#include "comboboxutilities.h"
#include "parser.h"

class QFocusEvent;
class QKeyEvent;
class QMouseEvent;
class QEvent;

namespace Digikam
{

class AdvancedRenameLineEditProxy : public ProxyLineEdit
{
    Q_OBJECT

public:

    AdvancedRenameLineEditProxy(QWidget* parent);
    virtual void setWidget(QWidget* widget);

protected:

    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);
};

// --------------------------------------------------------

class AdvancedRenameLineEdit : public QTextEdit
{
    Q_OBJECT

public:

    AdvancedRenameLineEdit(QWidget* parent = 0);
    ~AdvancedRenameLineEdit();

    void    setParser(Parser* parser);
    Parser* parser() const;

    void setAllowDirectoryCreation(bool allow);
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

    class AdvancedRenameLineEditPriv;
    AdvancedRenameLineEditPriv* const d;
};

// --------------------------------------------------------

class AdvancedRenameInput : public KComboBox
{
    Q_OBJECT

public:

    AdvancedRenameInput(QWidget* parent = 0);
    ~AdvancedRenameInput();

    void setParser(Parser* parser);
    void setAllowDirectoryCreation(bool allow);
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

private:

    void readSettings();
    void writeSettings();
    void enableHighlighter(bool enable);

private:

    class AdvancedRenameInputPriv;
    AdvancedRenameInputPriv* const d;
};

}  // namespace Digikam

#endif /* ADVANCEDRENAMEINPUT_H */
