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

#ifndef ADVANCEDRENAMEINPUT_H
#define ADVANCEDRENAMEINPUT_H

// Qt includes

#include <QWidget>

// KDE includes

#include <klineedit.h>
#include <kcombobox.h>

// Local includes

#include "parser.h"

class QMouseEvent;
class QFocusEvent;

namespace Digikam
{

class AdvancedRenameLineEditPriv;

class AdvancedRenameLineEdit : public KLineEdit
{
    Q_OBJECT

public:

    AdvancedRenameLineEdit(QWidget* parent = 0);
    ~AdvancedRenameLineEdit();

    void setParser(Parser* parser);

public Q_SLOTS:

    void slotAddToken(const QString&);
    void slotAddModifier(const QString&);

Q_SIGNALS:

    void signalTextChanged(const QString&);
    void signalTokenMarked(bool);

protected:

    virtual void mouseMoveEvent(QMouseEvent* e);
    virtual void mousePressEvent(QMouseEvent* e);
    virtual void leaveEvent(QEvent* e);
    virtual void focusInEvent(QFocusEvent* e);
    virtual void focusOutEvent(QFocusEvent* e);

private Q_SLOTS:

    void slotTextChanged();
    void slotParseTimer();
    void slotCursorPositionChanged(int, int);

private:

    void setTokenSelected(bool selected);
    bool selectionIsValid();
    bool tokenIsSelected();
    void searchAndHighlightTokens(Parser::Type type, int pos);

    void setSelectionColor(Parser::Type type);
    void rememberSelection();
    void resetSelection();

private:

    AdvancedRenameLineEditPriv* const d;
};

// --------------------------------------------------------

class AdvancedRenameInputPriv;

class AdvancedRenameInput : public KComboBox
{
    Q_OBJECT

public:

    AdvancedRenameInput(QWidget* parent = 0);
    ~AdvancedRenameInput();

    void setParser(Parser* parser);

Q_SIGNALS:

    void signalTextChanged(const QString&);
    void signalTokenMarked(bool);

public Q_SLOTS:

    void slotAddToken(const QString&);
    void slotAddModifier(const QString&);

private:

    void readSettings();
    void writeSettings();

private:

    AdvancedRenameInputPriv* const d;
};

}  // namespace Digikam

#endif /* ADVANCEDRENAMEINPUT_H */
