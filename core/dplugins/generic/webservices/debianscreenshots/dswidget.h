/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-11-29
 * Description : a tool to export images to Debian Screenshots
 *
 * Copyright (C) 2010 by Pau Garcia i Quiles <pgquiles at elpauer dot org>
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

#ifndef DSWIDGET_H
#define DSWIDGET_H

// Qt includes

#include <QWidget>
#include <QUrl>

// Local includes

#include "dactivelabel.h"
#include "ditemslist.h"
#include "statusprogressbar.h"

class QButtonGroup;
class QNetworkReply;
class QComboBox;
class QLineEdit;

namespace KIO
{
    class AccessManager;
}

using namespace Digikam;

namespace GenericDigikamDebianScreenshotsPlugin
{

class DSWidget : public QWidget
{
    Q_OBJECT

public:

    explicit DSWidget(QWidget* const parent);
    ~DSWidget();

    DItemsList* imagesList()         const;
    StatusProgressBar* progressBar() const;

Q_SIGNALS:

    void requiredPackageInfoAvailable(bool available);

private Q_SLOTS:

    void slotCompletePackageName(const QString&);
    void slotCompletePackageNameFinished(QNetworkReply*);
    void slotFindVersionsForPackage(const QString&);
    void slotFindVersionsForPackageFinished(QNetworkReply*);
    void slotEnableUpload();

private:

    QButtonGroup*                  m_dlGrp;
    QString                        m_lastTip;
    QUrl                           m_lastQueryUrl;

    QLineEdit*                     m_pkgLineEdit;
    QComboBox*                     m_versionsComboBox;
    QLineEdit*                     m_descriptionLineEdit;

    KIO::AccessManager*            m_httpManager;
    KIO::AccessManager*            m_jsonManager;

    DActiveLabel*                  m_headerLabel;

    DItemsList*                    m_imgList;
    StatusProgressBar*             m_progressBar;

    friend class DSWindow;
};

} // namespace GenericDigikamDebianScreenshotsPlugin

#endif // DSWIDGET_H
