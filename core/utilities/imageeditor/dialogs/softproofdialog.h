/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-09-16
 * Description : Dialog to adjust soft proofing settings
 *
 * Copyright (C) 2009-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2013-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef SOFTPROOFDIALOG_H
#define SOFTPROOFDIALOG_H

// Qt includes

#include <QDialog>

// Local includes

#include "digikam_export.h"
#include "iccsettingscontainer.h"

namespace Digikam
{

class SoftProofDialog : public QDialog
{
    Q_OBJECT

public:

    explicit SoftProofDialog(QWidget* const parent);
    virtual ~SoftProofDialog();

    bool shallEnableSoftProofView() const;

protected:

    void readSettings();
    void writeSettings();

protected Q_SLOTS:

    void updateGamutCheckState();
    void updateOkButtonState();

private Q_SLOTS:

    void slotOk();
    void slotProfileInfo();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // SOFTPROOFDIALOG_H
