/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2003-02-10
 * Description : camera setup tab.
 * 
 * Copyright 2003-2005 by Renchi Raju Gilles Caulier
 * Copyright 2006 by Gilles Caulier 
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

#ifndef SETUPCAMERA_H
#define SETUPCAMERA_H

// QT includes.

#include <qwidget.h>

class QListView;
class QListViewItem;
class QPushButton;

namespace Digikam
{

class SetupCamera : public QWidget
{
    Q_OBJECT

public:

    SetupCamera( QWidget* parent = 0 );
    ~SetupCamera();

    void applySettings();
    
private:
    
    QListView*   listView_;
    QPushButton* addButton_;
    QPushButton* removeButton_;
    QPushButton* editButton_;
    QPushButton* autoDetectButton_;

private slots:

    void slotSelectionChanged();

    void slotAddCamera();
    void slotRemoveCamera();
    void slotEditCamera();
    void slotAutoDetectCamera();

    void slotAddedCamera(const QString& title, const QString& model,
                         const QString& port,  const QString& path);
    void slotEditedCamera(const QString& title, const QString& model,
                          const QString& port,  const QString& path);
};

}  // namespace Digikam

#endif // SETUPCAMERA_H 
