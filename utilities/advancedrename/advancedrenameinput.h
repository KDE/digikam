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
#include <QTextEdit>

// KDE includes

#include <klineedit.h>
#include <kcombobox.h>

// Local includes

#include "parser.h"

class QMouseEvent;
class QFocusEvent;
class QKeyEvent;

namespace Digikam
{

class AdvancedRenameLineEditPriv;

class AdvancedRenameLineEdit : public QTextEdit
{
    Q_OBJECT

public:

    AdvancedRenameLineEdit(QWidget* parent = 0);
    ~AdvancedRenameLineEdit();

    void    setParser(Parser* parser);
    Parser* parser();

public Q_SLOTS:

    void slotSetHistoryItem(const QString&);
    void slotCursorPositionChanged();

Q_SIGNALS:

    void signalTextChanged(const QString&);
    void signalTokenMarked(bool);
    void signalReturnPressed();

protected:

    virtual void keyPressEvent(QKeyEvent* e);
    virtual void wheelEvent(QWheelEvent* e);

private Q_SLOTS:

    void slotTextChanged();
    void slotParseTimer();

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

    QString text() const;
    void    setText(const QString& text);

    void    clearText();
    void    clearTextAndHistory();

Q_SIGNALS:

    void signalTextChanged(const QString&);
    void signalTokenMarked(bool);
    void signalReturnPressed();

public Q_SLOTS:

    void slotAddToken(const QString&);

private:

    void readSettings();
    void writeSettings();

private:

    AdvancedRenameInputPriv* const d;
};

}  // namespace Digikam

#endif /* ADVANCEDRENAMEINPUT_H */
