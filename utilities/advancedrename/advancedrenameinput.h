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

// Local includes

#include "digikam_export.h"

class QMouseEvent;
class QFocusEvent;

namespace Digikam
{

class Parser;
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

Q_SIGNALS:

    void signalTextChanged(const QString&);
    void signalTokenMarked(bool);

protected:

    virtual void mouseMoveEvent(QMouseEvent* e);
    virtual void mousePressEvent(QMouseEvent* e);
    virtual void focusInEvent(QFocusEvent* e);
    virtual void focusOutEvent(QFocusEvent* e);

private Q_SLOTS:

    void slotTextChanged();
    void slotParseTimer();
    void slotCursorPositionChanged(int, int);

private:

    bool highlightToken(int pos);
    bool tokenIsSelected();

private:

    AdvancedRenameLineEditPriv* const d;
};

// --------------------------------------------------------

class AdvancedRenameInputPriv;

class DIGIKAM_EXPORT AdvancedRenameInput : public QWidget
{
    Q_OBJECT

public:

    AdvancedRenameInput(QWidget* parent = 0);
    ~AdvancedRenameInput();

    AdvancedRenameLineEdit* input() const;

public Q_SLOTS:

    void slotAddToken(const QString&);

Q_SIGNALS:

    void signalTextChanged(const QString&);

private:

    AdvancedRenameInputPriv* const d;
};

}  // namespace Digikam

#endif /* ADVANCEDRENAMEINPUT_H */
