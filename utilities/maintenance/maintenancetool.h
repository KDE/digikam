/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-01-16
 * Description : Maintenance tool class
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

#ifndef MAINTENANCETOOL_H
#define MAINTENANCETOOL_H

// Qt includes

#include <QString>

// Local includes

#include "progressmanager.h"

namespace Digikam
{

class MaintenanceTool : public ProgressItem
{
    Q_OBJECT

public:

    enum Mode
    {
        AllItems = 0,  /// Process all items from whole collections
        MissingItems,  /// Process missing items from whole collections
        AlbumItems     /// Process items from current album set by albumId
    };

public:

    MaintenanceTool(const QString& id, Mode mode=AllItems, int albumId=-1);
    virtual ~MaintenanceTool();

Q_SIGNALS:

    void signalProcessDone();

protected:

    void setTitle(const QString& title);

    bool cancel() const;

    /** Called when all is done. It fire signalProcessDone().
     */
    void complete();

    /** Return all paths to process. Data container can be customized.
     */
    QStringList&         allPicturesPath();

    /** Return mode set in contructor. see Mode enum for details.
     */
    Mode                 mode() const;

    /** Call this method into processOne() to check if another item must be processed.
     */
    bool                 checkToContinue() const;

    /** Called by slotRun() to populate all pictures path to process.
     */
    virtual void populateAllPicturesPath();

    /** In this method, you can filter items to manage, hosted by allPicturePath(). These paths will be 
     *  used calling processOne().
     */
    virtual void listItemstoProcess() = 0;

    /** In this method, you can customize your items processor.
     */
    virtual void processOne() = 0;

private Q_SLOTS:

    /** This slot call listItemstoProcess() when tool is started.
     */
    void slotRun();

    /** This slot is called when user cancel tool from gui.
     */
    void slotCancel();

private:

    class MaintenanceToolPriv;
    MaintenanceToolPriv* const d;
};

}  // namespace Digikam

#endif /* MAINTENANCETOOL_H */
