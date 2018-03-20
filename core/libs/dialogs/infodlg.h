/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-07-11
 * Description : general info list dialog
 *
 * Copyright (C) 2008-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009      by Andi Clemens <andi dot clemens at gmail dot com>
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

#ifndef INFODLG_H
#define INFODLG_H

// Qt includes

#include <QMap>
#include <QDialog>

// Local includes

#include "digikam_export.h"

class QTreeWidget;

namespace Digikam
{

class DIGIKAM_EXPORT InfoDlg : public QDialog
{
    Q_OBJECT

public:

    explicit InfoDlg(QWidget* const parent);
    virtual ~InfoDlg();

    virtual void setInfoMap(const QMap<QString, QString>& list);

    QTreeWidget* listView()   const;
    QWidget*     mainWidget() const;

private Q_SLOTS:

    void slotHelp();
    virtual void slotCopy2ClipBoard();

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif  // INFODLG_H
