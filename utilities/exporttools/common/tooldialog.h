/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-04-04
 * Description : export Tool dialog
 *
 * Copyright (C) 2012-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef TOOL_DIALOG_H
#define TOOL_DIALOG_H

// KDE includes

#include <QWizard>
#include <QDialogButtonBox>

// Local includes

#include "digikam_export.h"

class QAbstractButton;
class QPushButton;
class QDialog;

namespace Digikam
{

class DIGIKAM_EXPORT ToolDialog : public QDialog
{
    Q_OBJECT

public:

    explicit ToolDialog(QWidget* const parent = 0);
    ~ToolDialog();

    void setMainWidget(QWidget* const widget);

    void setRejectButtonMode(QDialogButtonBox::StandardButton button);

    QPushButton* startButton() const;

    void addButton(QAbstractButton* button, QDialogButtonBox::ButtonRole role);

private Q_SLOTS:

    void slotCloseClicked();

Q_SIGNALS:

    void cancelClicked();

private:

    QPushButton* helpButton() const;

private:

    friend class KPDialogBase;

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // TOOL_DIALOG_H
