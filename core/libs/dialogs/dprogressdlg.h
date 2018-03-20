/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-30-08
 * Description : a progress dialog for digiKam
 *
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DPROGRESSDLG_H
#define DPROGRESSDLG_H

// Qt includes

#include <QPixmap>
#include <QDialog>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DProgressDlg : public QDialog
{
    Q_OBJECT

public:

    explicit DProgressDlg(QWidget* const parent=0, const QString& caption=QString());
    ~DProgressDlg();

    void setLabel(const QString& text);
    void setTitle(const QString& text);

    int  value() const;

Q_SIGNALS:

    void signalCancelPressed();

public Q_SLOTS:

    void setMaximum(int max);
    void incrementMaximum(int added);
    void advance(int offset);
    void setValue(int value);
    void setButtonText(const QString& text);

    void addedAction(const QPixmap &icon, const QString& text);
    void reset();

protected Q_SLOTS:

    void slotCancel();

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif  // DPROGRESSDLG_H
