/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-04-04
 * Description : Web Service Tool dialog
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

#ifndef WS_TOOL_DIALOG_H
#define WS_TOOL_DIALOG_H

// KDE includes

#include <QDialog>
#include <QDialogButtonBox>

// Local includes

#include "digikam_export.h"

class QAbstractButton;
class QPushButton;

namespace Digikam
{

class DIGIKAM_EXPORT WSToolDialog : public QDialog
{
    Q_OBJECT

public:

    explicit WSToolDialog(QWidget* const parent = 0);
    ~WSToolDialog();

    void setMainWidget(QWidget* const widget);

    void setRejectButtonMode(QDialogButtonBox::StandardButton button);

    QPushButton* startButton() const;

    void addButton(QAbstractButton* button, QDialogButtonBox::ButtonRole role);

private Q_SLOTS:

    void slotCloseClicked();

Q_SIGNALS:

    void cancelClicked();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // WS_TOOL_DIALOG_H
