/* ============================================================
 * File  : setupgeneral.h
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-02-15
 * Description : 
 * 
 * Copyright 2003 by Renchi Raju

 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published bythe Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * ============================================================ */

#ifndef SETUPGENERAL_H
#define SETUPGENERAL_H

#include <qwidget.h>

class QRadioButton;
class QPushButton;
class QCheckBox;
class QLineEdit;
class QListBox;
class QListBoxItem;

class SetupGeneral : public QWidget
{
    Q_OBJECT
    
public:

    SetupGeneral(QWidget* parent = 0);
    ~SetupGeneral();

    void applySettings();

private:

    void readSettings();

    QLineEdit    *albumPathEdit;
    QLineEdit    *fileFilterEdit;
    
    QRadioButton *smallIconButton_;
    QRadioButton *mediumIconButton_;
    QRadioButton *largeIconButton_;
    QRadioButton *hugeIconButton_;

    QCheckBox    *iconShowMimeBox_;
    QCheckBox    *iconShowSizeBox_;
    QCheckBox    *iconShowDateBox_;
    QCheckBox    *iconShowCommentsBox_;

    QListBox     *albumCollectionBox_;
    QPushButton  *addCollectionButton_;
    QPushButton  *delCollectionButton_;
    
private slots:

    void slotChangeAlbumPath();
    void slotCollectionSelectionChanged();
    void slotAddCollection();
    void slotDelCollection();
};

#endif /* SETUPGENERAL_H */
