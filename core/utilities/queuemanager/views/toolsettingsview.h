/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-11-27
 * Description : a view to show Batch Tool Settings.
 *
 * Copyright (C) 2008-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef TOOL_SETTINGS_VIEW_H
#define TOOL_SETTINGS_VIEW_H

// Qt includes

#include <QStackedWidget>

// Local includes

#include "batchtool.h"
#include "batchtoolutils.h"

namespace Digikam
{

class ToolSettingsView : public QStackedWidget
{
    Q_OBJECT

public:

    explicit ToolSettingsView(QWidget* const parent = 0);
    ~ToolSettingsView();

    void setBusy(bool b);

Q_SIGNALS:

    void signalSettingsChanged(const BatchToolSet&);

public Q_SLOTS:

    void slotToolSelected(const BatchToolSet&);

private Q_SLOTS:

    void slotSettingsChanged(const BatchToolSettings&);
    void slotThemeChanged();

private:

    int  viewMode() const;
    void setViewMode(int mode);
    void setToolSettingsWidget(QWidget* const w);

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* TOOL_SETTINGS_VIEW_H */
