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

#ifndef MANUALRENAMEINPUT_H
#define MANUALRENAMEINPUT_H

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
namespace ManualRename
{

class ManualRenameParser;
class ManualRenameLineEditPriv;

class ManualRenameLineEdit : public KLineEdit
{
    Q_OBJECT

public:

    ManualRenameLineEdit(QWidget* parent = 0);
    ~ManualRenameLineEdit();

    void setParser(ManualRenameParser* parser);

    bool findToken(int curPos);
    bool findToken(int curPos, int& pos, int& length);

public Q_SLOTS:

    void slotAddToken(const QString&);

Q_SIGNALS:

    void signalTextChanged(const QString&);
    void signalTokenMarked(bool);

protected:

    virtual void mouseMoveEvent(QMouseEvent* e);
    virtual void mousePressEvent(QMouseEvent* e);
    virtual void focusOutEvent(QFocusEvent* e);

private Q_SLOTS:

    void slotTextChanged();
    void slotParseTimer();
    void slotCursorPositionChanged(int, int);

private:

    bool highlightToken(int cursorPos);

private:

    ManualRenameLineEditPriv* const d;
};

// --------------------------------------------------------

class ManualRenameInputPriv;

class DIGIKAM_EXPORT ManualRenameInput : public QWidget
{
    Q_OBJECT

public:

    ManualRenameInput(QWidget* parent = 0);
    ~ManualRenameInput();

    ManualRenameLineEdit* input() const;

public Q_SLOTS:

    void slotAddToken(const QString&);

Q_SIGNALS:

    void signalTextChanged(const QString&);

private Q_SLOTS:

    void slotMoveTokenLeft();
    void slotMoveTokenRight();

private:

    ManualRenameInputPriv* const d;
};

}  // namespace ManualRename
}  // namespace Digikam

#endif /* MANUALRENAMEINPUT_H */
