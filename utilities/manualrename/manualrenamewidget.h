/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-05-22
 * Description : a control widget for the ManualRenameParser
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

#ifndef MANUALRENAMEWIDGET_H
#define MANUALRENAMEWIDGET_H

// Qt includes

#include <QtGui/QWidget>
#include <QtCore/QString>

// Local includes

#include "digikam_export.h"

class QDateTime;
class QAction;

class KLineEdit;

namespace Digikam
{
namespace ManualRename
{

class MainParser;
class ManualRenameWidgetPriv;
class ParseInformation;

class DIGIKAM_EXPORT ManualRenameWidget : public QWidget
{
    Q_OBJECT

public:

    enum InputStyle
    {
        None       = 0x0,
        BigButtons = 0x1,
        ToolButton = 0x2
    };
    Q_DECLARE_FLAGS(InputStyles, InputStyle)

    ManualRenameWidget(QWidget* parent = 0);
    ~ManualRenameWidget();

    QString text() const;
    void    setText(const QString& text);
    void    clear();

    void setParser(MainParser* parser);
    /**
     * set the columns for the parser input buttons. This will only affect the BigButtons input style.
     * @param col number of columns
     */
    void setInputColumns(int col);
    void setInputStyle(InputStyles inputMask);

    QString parse(ParseInformation& info) const;

    void setTrackerAlignment(Qt::Alignment alignment);

Q_SIGNALS:

    void signalTextChanged(const QString&);

public Q_SLOTS:

    void slotUpdateTrackerPos();
    void slotHideToolTipTracker();

private Q_SLOTS:

    void slotToolTipButtonToggled(bool);

private:

    void createToolTip();
    void registerParsers();
    void setupWidgets();

private:

    ManualRenameWidgetPriv* const d;
};

}  // namespace ManualRename
}  // namespace Digikam

Q_DECLARE_OPERATORS_FOR_FLAGS(Digikam::ManualRename::ManualRenameWidget::InputStyles)

#endif /* MANUALRENAMEWIDGET_H */
