/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-07-28
 * Description : Common widgets shared by export tools
 *
 * Copyright (C) 2013 by Pankaj Kumar <me at panks dot me>
 * Copyright (C) 2015 by Shourya Singh Gupta <shouryasgupta at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef SETTINGS_WIDGET_H
#define SETTINGS_WIDGET_H

//Qt includes

#include <QWidget>

// Local includes

#include "digikam_export.h"
#include "dimageslist.h"
#include "dinfointerface.h"
#include "dprogresswdg.h"

class QLabel;
class QSpinBox;
class QCheckBox;
class QButtonGroup;
class QComboBox;
class QPushButton;
class QGroupBox;
class QGridLayout;
class QVBoxLayout;
class QHBoxLayout;

namespace Digikam
{

class DIGIKAM_EXPORT SettingsWidget : public QWidget
{
    Q_OBJECT

public:

    explicit SettingsWidget(QWidget* const parent, const QString& toolName);
    ~SettingsWidget();

    virtual void updateLabels(const QString& name = QString(), const QString& url = QString()) = 0;

    DImagesList*      imagesList()  const;
    DProgressWdg*     progressBar() const;
    void              replaceImageList(QWidget* const widget);

    QWidget*          getSettingsBox() const;
    QVBoxLayout*      getSettingsBoxLayout() const;
    void              addWidgetToSettingsBox(QWidget* const widget);

    QGroupBox*        getAlbumBox() const;
    QGridLayout*      getAlbumBoxLayout() const;

    QGroupBox*        getOptionsBox() const;
    QGridLayout*      getOptionsBoxLayout() const;

    QGroupBox*        getSizeBox() const;
    QVBoxLayout*      getSizeBoxLayout() const;

    QGroupBox*        getAccountBox() const;
    QGridLayout*      getAccountBoxLayout() const;

    QLabel*           getHeaderLbl() const;
    QLabel*           getUserNameLabel() const;
    QPushButton*      getChangeUserBtn() const;
    QComboBox*        getDimensionCoB() const;
    QPushButton*      getNewAlbmBtn() const;
    QPushButton*      getReloadBtn() const;
    QCheckBox*        getOriginalCheckBox() const;
    QCheckBox*        getResizeCheckBox() const;
    QSpinBox*         getDimensionSpB() const;
    QSpinBox*         getImgQualitySpB() const;
    QComboBox*        getAlbumsCoB() const;

protected Q_SLOTS:

    void slotResizeChecked();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // SETTINGS_WIDGET_H
