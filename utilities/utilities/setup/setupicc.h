/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at gmail dot com> 
 *          F.J. Cruz <fj.cruz@supercable.es>
 * Date   : 2005-11-24
 * Description : ICC profils setup tab.
 * 
 * Copyright 2005-2007 by Gilles Caulier and F.J. Cruz
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

#ifndef SETUPICC_H
#define SETUPICC_H

// Qt includes.

#include <qwidget.h>
#include <qmap.h>
#include <qdir.h>

// Local Includes.

#include "digikam_export.h"

class KDialogBase;

namespace Digikam
{

class SetupICCPriv;

class DIGIKAM_EXPORT SetupICC : public QWidget
{
    Q_OBJECT

public:

    SetupICC(QWidget* parent = 0, KDialogBase* dialog = 0);
    ~SetupICC();

    void applySettings();

    static bool iccRepositoryIsValid();

private:

    void readSettings(bool restore=false);
    void fillCombos(const QString& path, bool report);
    void enableWidgets();
    void disableWidgets();
    void profileInfo(const QString&);
    bool parseProfilesfromDir(const QFileInfoList* files);

private slots:

    void processLCMSURL(const QString&);
    void slotToggledWidgets(bool);
    void slotToggleManagedView(bool);
    void slotFillCombos(const QString&);
    void slotClickedIn();
    void slotClickedWork();
    void slotClickedMonitor();
    void slotClickedProof();

private:

    SetupICCPriv* d;
};

}  // namespace Digikam

#endif // SETUPICC_H 
