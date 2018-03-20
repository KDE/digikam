/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-07-28
 * Description : Common widgets shared by Web Service tools
 *
 * Copyright (C) 2013      by Pankaj Kumar <me at panks dot me>
 * Copyright (C) 2015      by Shourya Singh Gupta <shouryasgupta at gmail dot com>
 * Copyright (C) 2016-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef WS_SETTINGS_WIDGET_H
#define WS_SETTINGS_WIDGET_H

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

class DIGIKAM_EXPORT WSSettingsWidget : public QWidget
{
    Q_OBJECT

public:

    explicit WSSettingsWidget(QWidget* const parent,
                              DInfoInterface* const iface,
                              const QString& toolName);
    ~WSSettingsWidget();

public:

    void              replaceImageList(QWidget* const widget);
    void              addWidgetToSettingsBox(QWidget* const widget);

    QString           getDestinationPath()   const;
    DImagesList*      imagesList()           const;
    DProgressWdg*     progressBar()          const;

    QWidget*          getSettingsBox()       const;
    QVBoxLayout*      getSettingsBoxLayout() const;

    QGroupBox*        getAlbumBox()          const;
    QGridLayout*      getAlbumBoxLayout()    const;

    QGroupBox*        getOptionsBox()        const;
    QGridLayout*      getOptionsBoxLayout()  const;

    QGroupBox*        getUploadBox()         const;
    QVBoxLayout*      getUploadBoxLayout()   const;

    QGroupBox*        getSizeBox()           const;
    QVBoxLayout*      getSizeBoxLayout()     const;

    QGroupBox*        getAccountBox()        const;
    QGridLayout*      getAccountBoxLayout()  const;

    QLabel*           getHeaderLbl()         const;
    QLabel*           getUserNameLabel()     const;
    QPushButton*      getChangeUserBtn()     const;
    QComboBox*        getDimensionCoB()      const;
    QPushButton*      getNewAlbmBtn()        const;
    QPushButton*      getReloadBtn()         const;
    QCheckBox*        getOriginalCheckBox()  const;
    QCheckBox*        getResizeCheckBox()    const;
    QSpinBox*         getDimensionSpB()      const;
    QSpinBox*         getImgQualitySpB()     const;
    QComboBox*        getAlbumsCoB()         const;

public:

    virtual void updateLabels(const QString& name = QString(),
                              const QString& url = QString()) = 0;

protected Q_SLOTS:

    void slotResizeChecked();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // WS_SETTINGS_WIDGET_H
