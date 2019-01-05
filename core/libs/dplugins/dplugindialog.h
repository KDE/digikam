/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-12-31
 * Description : digiKam plugin main dialog
 *
 * Copyright (C) 2018-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_DPLUGIN_DIALOG_H
#define DIGIKAM_DPLUGIN_DIALOG_H

// Qt includes

#include <QDialog>

// Local includes

#include "digikam_export.h"
#include "dplugin.h"

namespace Digikam
{

class DIGIKAM_EXPORT DPluginDialog : public QDialog
{
    Q_OBJECT

public:

    explicit DPluginDialog(QWidget* const parent, const QString& objName);
    ~DPluginDialog();
    
    void setPlugin(DPlugin* const tool);

protected:

    virtual void restoreDialogSize();
    virtual void saveDialogSize();

private Q_SLOTS:

    void slotAboutPlugin();

private:

    DPlugin* m_tool;
    
    Q_DISABLE_COPY(DPluginDialog)
};

} // namespace Digikam

#endif // DIGIKAM_DPLUGIN_DIALOG_H
