/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-01-16
 * Description : Maintenance tool using preview load thread as items processor.
 *
 * Copyright (C) 2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef MAINTENANCEPREVIEWTOOL_H
#define MAINTENANCEPREVIEWTOOL_H

// Local includes

#include "maintenancepictpathtool.h"

namespace Digikam
{

class DImg;
class LoadingDescription;
class PreviewLoadThread;

class MaintenancePreviewTool : public MaintenancePictPathTool
{
    Q_OBJECT

public:

    MaintenancePreviewTool(const QString& id, Mode mode=AllItems, int albumId=-1);
    virtual ~MaintenancePreviewTool();

protected:

    /** Return preview loader instance
     */
    PreviewLoadThread* previewLoadThread() const;

    /** Re-implement this if you want to use preview loader as items processor
     */
    virtual void gotNewPreview(const LoadingDescription&, const DImg&) {};

private Q_SLOTS:

    /** Called by preview thread. This slot call gotNewPreview()
     */
    void slotGotImagePreview(const LoadingDescription&, const DImg&);

private:

    class MaintenancePreviewToolPriv;
    MaintenancePreviewToolPriv* const d;
};

}  // namespace Digikam

#endif /* MAINTENANCEPREVIEWTOOL_H */
