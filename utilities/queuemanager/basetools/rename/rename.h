/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-09-06
 * Description : batch tool for renaming files.
 *
 * Copyright (C) 2009 by Andi Clemens <andi dot clemens at gmx dot net>
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

#ifndef RENAMETOOL_H
#define RENAMETOOL_H

// Local includes

#include "batchtool.h"

namespace Digikam
{

class RenamePriv;

class Rename : public BatchTool
{
    Q_OBJECT

public:

    Rename(QObject* parent=0);
    ~Rename();

    BatchToolSettings defaultSettings();

    virtual QString outputBaseName() const;

private Q_SLOTS:

    void slotSettingsChanged();

private:

    void assignSettings2Widget();
    bool toolOperations();

private:

    RenamePriv* const d;
};

}  // namespace Digikam

#endif /* AUTO_CORRECTION_H */
