/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2003-02-01
 * Description :
 *
 * Copyright 2003-2004 by Renchi Raju
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


#ifndef SETUPGENERAL_H
#define SETUPGENERAL_H

// Qt includes.

#include <qwidget.h>

class QComboBox;
class QRadioButton;
class QCheckBox;
class QLineEdit;

class KDialogBase;

class SetupGeneral : public QWidget
{
    Q_OBJECT

public:
    enum ThumbnailSizes { /*TinyThumb, */SmallThumb = 0, MediumThumb, LargeThumb, HugeThumb };

    SetupGeneral(QWidget* parent = 0, KDialogBase* dialog = 0 );
    ~SetupGeneral();

    void applySettings();

private:

    void readSettings();

    QLineEdit    *albumPathEdit;

    QComboBox    *thumbnailSize_;

    QCheckBox    *recurseTagsBox_;
    QCheckBox    *showToolTipsBox_;

    QCheckBox    *iconShowNameBox_;
    QCheckBox    *iconShowSizeBox_;
    QCheckBox    *iconShowDateBox_;
    QCheckBox    *iconShowResolutionBox_;
    QCheckBox    *iconShowCommentsBox_;
    QCheckBox    *iconShowTagsBox_;
    QCheckBox    *iconShowFileCommentsBox_;

    KDialogBase  *mainDialog_;

private slots:

    void slotChangeAlbumPath();
    void slotPathEdited(const QString& newPath);

};

#endif // SETUPGENERAL_H
