/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-12-24
 * Description : a dialog to display processed messages in background
 *
 * Copyright (C) 2009-2016 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef DOUTPUTDLG_H
#define DOUTPUTDLG_H

// Qt includes

#include <QString>
#include <QDialog>

namespace Digikam
{

class DOutputDlg : public QDialog
{
    Q_OBJECT

public:

    explicit DOutputDlg(QWidget* const parent = 0,
                        const QString& caption = QString(),
                        const QString& messages = QString(),
                        const QString& header = QString());
    virtual ~DOutputDlg();

private Q_SLOTS:

    void slotCopyToCliboard();

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif // DOUTPUTDLG_H
