/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-05-28
 * Description : a dialog for showing the advancedrename tooltip
 *
 * Copyright (C) 2010-2012 by Andi Clemens <andi dot clemens at gmail dot com>
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

#ifndef TOOLTIPDIALOG_H
#define TOOLTIPDIALOG_H

// Qt includes

#include <QDialog>

namespace Digikam
{

class TooltipDialog : public QDialog
{
    Q_OBJECT

public:

    explicit TooltipDialog(QWidget* const parent);
    virtual ~TooltipDialog();

    void setTooltip(const QString& tooltip);
    void clearTooltip();

private:

    TooltipDialog(const TooltipDialog&);
    TooltipDialog& operator=(const TooltipDialog&);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif /* TOOLTIPDIALOG_H */
