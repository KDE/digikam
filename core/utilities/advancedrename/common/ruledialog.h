/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-05-01
 * Description : a dialog that can be used to display a configuration
 *               dialog for a rule
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

#ifndef RULEDIALOG_H
#define RULEDIALOG_H

// Qt includes

#include <QDialog>

namespace Digikam
{

class Rule;

class RuleDialog : public QDialog
{
    Q_OBJECT

public:

    explicit RuleDialog(Rule* const parent);
    virtual ~RuleDialog();

    void setSettingsWidget(QWidget* const settingsWidget);

private:

    RuleDialog(const RuleDialog&);
    RuleDialog& operator=(const RuleDialog&);

    void setDialogTitle(const QString& title);
    void setDialogDescription(const QString& description);
    void setDialogIcon(const QPixmap& pixmap);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif /* RULEDIALOG_H */
