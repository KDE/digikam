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

class QButtonGroup;
class QNetworkReply;
class QComboBox;
class KLineEdit;

namespace KIO
{
    class AccessManager;
}

namespace KIPI
{
    class UploadWidget;
}

namespace KIPIPlugins
{
    class KPImagesList;
    class KPProgressWidget;
}

namespace GenericDigikamDebianScreenshotsPlugin
{

class KClickableImageLabel;

class DsWidget : public QWidget
{
    Q_OBJECT

public:

    explicit DsWidget(QWidget* const parent);
    ~DsWidget();

    QString getDestinationPath()                 const;
    KIPIPlugins::KPImagesList* imagesList()      const;
    KIPIPlugins::KPProgressWidget* progressBar() const;

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

    KLineEdit*                     m_pkgLineEdit;
    QComboBox*                     m_versionsComboBox;
    KLineEdit*                     m_descriptionLineEdit;

    KIO::AccessManager*            m_httpManager;
    KIO::AccessManager*            m_jsonManager;

    KClickableImageLabel*          m_headerLabel;

    KIPIPlugins::KPImagesList*     m_imgList;
    KIPI::UploadWidget*            m_uploadWidget;
    KIPIPlugins::KPProgressWidget* m_progressBar;

    friend class DsWindow;
};

} // namespace GenericDigikamDebianScreenshotsPlugin

#endif // DSWIDGET_H
