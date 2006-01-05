/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr> 
 *         F.J. Cruz <fj.cruz@supercable.es>
 * Date  : 2005-11-24
 * Description : ICC profils setup tab.
 * 
 * Copyright 2005-2006 by Gilles Caulier and F.J. Cruz
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

class QCheckBox;
class QRadioButton;
class KURLRequester;
class KComboBox;
// class QStringList;
typedef QMap<QString, QString> ICCfilesPath;

namespace Digikam
{

class SetupICC : public QWidget
{
    Q_OBJECT

public:

    SetupICC(QWidget* parent = 0);
    ~SetupICC();

    void applySettings();

private:

    void readSettings();
    void fillCombos();
    void enableWidgets();
    void disableWidgets();
    void profileInfo(const QString&);

private:

    QCheckBox       *m_enableColorManagement;
    QCheckBox       *m_bpcAlgorithm;
    
    QRadioButton    *m_defaultApplyICC;
    QRadioButton    *m_defaultAskICC;

    QStringList     m_inICCFiles_file;
    QStringList     m_workICCFiles_file;
    QStringList     m_proofICCFiles_file;
    QStringList     m_monitorICCFiles_file;

    KURLRequester   *m_defaultPath;

    KComboBox       *m_inProfiles;
    KComboBox       *m_workProfiles;
    KComboBox       *m_proofProfiles;
    KComboBox       *m_monitorProfiles;
    KComboBox       *m_renderingIntent;

    ICCfilesPath    m_ICCfilesPath;

private slots:

    void slotToggledWidgets(bool t);
    void slotFillCombos(const QString&);
    void slotClickedIn();
    void slotClickedWork();
    void slotClickedMonitor();
    void slotClickedProof();
    void slotChangeWorkProfile(int index);
    void slotChangeInProfile(int index);
    void slotChangeMonitorProfile(int index);
    void slotChangeProofProfile(int index);

};

}  // namespace Digikam

#endif // SETUPICC_H 
