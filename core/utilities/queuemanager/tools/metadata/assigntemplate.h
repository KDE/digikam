/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-03-04
 * Description : assign metadata template batch tool.
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef ASSIGN_TEMPLATE_H
#define ASSIGN_TEMPLATE_H

// Local includes

#include "batchtool.h"

namespace Digikam
{
class TemplateSelector;
class TemplateViewer;

class AssignTemplate : public BatchTool
{
    Q_OBJECT

public:

    explicit AssignTemplate(QObject* const parent = 0);
    virtual ~AssignTemplate();

    BatchToolSettings defaultSettings();

    BatchTool* clone(QObject* const parent=0) const { return new AssignTemplate(parent); };

    void registerSettingsWidget();

private:

    bool toolOperations();

private Q_SLOTS:

    void slotAssignSettings2Widget();
    void slotSettingsChanged();

private:

    TemplateSelector* m_templateSelector;
    TemplateViewer*   m_templateViewer;
};

} // namespace Digikam

#endif // ASSIGN_TEMPLATE_H
