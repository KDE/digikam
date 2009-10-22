/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-05-22
 * Description : a control widget for the AdvancedRename utility
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

#ifndef ADVANCEDRENAMEWIDGET_H
#define ADVANCEDRENAMEWIDGET_H

// Qt includes

#include <QtGui/QWidget>
#include <QtCore/QString>

namespace Digikam
{

class ParseInformation;
class Parser;
class AdvancedRenameWidgetPriv;

class AdvancedRenameWidget : public QWidget
{
    Q_OBJECT

public:

    AdvancedRenameWidget(QWidget* parent = 0);
    ~AdvancedRenameWidget();

    enum ControlWidget
    {
        None               = 0x0,
        ToolTipButton      = 0x1,
        TokenButtons       = 0x2,
        TokenToolButton    = 0x4,
        ModifierButtons    = 0x8,
        ModifierToolButton = 0x10
    };
    Q_DECLARE_FLAGS(ControlWidgets, ControlWidget)

    QString text() const;
    void    setText(const QString& text);
    void    clear();

    void setParser(Parser* parser);
    void setControlWidgets(ControlWidgets mask);

    QString parse(ParseInformation& info) const;

    void setTooltipAlignment(Qt::Alignment alignment);
    void focusLineEdit();

Q_SIGNALS:

    void signalTextChanged(const QString&);

public Q_SLOTS:

    void slotUpdateTrackerPos();
    void slotHideToolTipTracker();

private Q_SLOTS:

    void slotToolTipButtonToggled(bool);
    void slotTokenMarked(bool);

private:

    void createToolTip();
    void registerParserControls();
    void setupWidgets();

    void readSettings();
    void writeSettings();

private:

    AdvancedRenameWidgetPriv* const d;
};

}  // namespace Digikam

Q_DECLARE_OPERATORS_FOR_FLAGS(Digikam::AdvancedRenameWidget::ControlWidgets)

#endif /* ADVANCEDRENAMEWIDGET_H */
