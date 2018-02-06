/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-17-06
 * Description : a tool to export images to Flickr web service
 *
 * Copyright (C) 2005-2008 by Vardhman Jain <vardhman at gmail dot com>
 * Copyright (C) 2008-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef FLICKR_WINDOW_H
#define FLICKR_WINDOW_H

// Qt includes

#include <QHash>
#include <QList>
#include <QPair>
#include <QLabel>
#include <QLinkedList>
#include <QLineEdit>
#include <QUrl>
#include <QComboBox>
#include <QDialog>

// Local includes

#include "wscomboboxintermediate.h"
#include "dinfointerface.h"
#include "wstooldialog.h"
#include "digikam_export.h"

class QProgressDialog;
class QPushButton;
class QSpinBox;
class QCheckBox;

namespace Digikam
{

class WSSelectUserDlg;
class FlickrWidget;
class FlickrTalker;
class FlickrList;
class FlickrNewAlbumDlg;
class FPhotoInfo;

class DIGIKAM_EXPORT FlickrWindow : public WSToolDialog
{
    Q_OBJECT

public:

    explicit FlickrWindow(DInfoInterface* const iface,
                          QWidget* const parent,
                          const QString& serviceName = QLatin1String("Flickr"));
    ~FlickrWindow();

    /**
     * Use this method to (re-)activate the dialog after it has been created
     * to display it. This also loads the currently selected images.
     */
    void reactivate();

    void setItemsList(const QList<QUrl>& urls);

private Q_SLOTS:

    void slotLinkingSucceeded();
    void slotBusy(bool val);
    void slotError(const QString& msg);
    void slotFinished();
    void slotUser1();
    void slotCancelClicked();

    void slotCreateNewPhotoSet();
    void slotUserChangeRequest();
    void slotRemoveAccount();
    void slotPopulatePhotoSetComboBox();
    void slotAddPhotoNext();
    void slotAddPhotoSucceeded();
    void slotAddPhotoFailed(const QString& msg);
    void slotAddPhotoSetSucceeded();
    void slotListPhotoSetsFailed(const QString& msg);
    void slotAddPhotoCancelAndClose();
    void slotAuthCancel();
    void slotImageListChanged();
    void slotReloadPhotoSetRequest();

private:

    QString guessSensibleSetName(const QList<QUrl>& urlList) const;

    void closeEvent(QCloseEvent*)  Q_DECL_OVERRIDE;
    void readSettings(QString uname);
    void writeSettings();

    void setUiInProgressState(bool inProgress);

private:

    unsigned int                           m_uploadCount;
    unsigned int                           m_uploadTotal;

    QString                                m_serviceName;

    QPushButton*                           m_newAlbumBtn;
    QPushButton*                           m_changeUserButton;
    QPushButton*                           m_removeAccount;

    QComboBox*                             m_albumsListComboBox;
    QCheckBox*                             m_publicCheckBox;
    QCheckBox*                             m_familyCheckBox;
    QCheckBox*                             m_friendsCheckBox;
    QCheckBox*                             m_exportHostTagsCheckBox;
    QCheckBox*                             m_stripSpaceTagsCheckBox;
    QCheckBox*                             m_addExtraTagsCheckBox;
    QCheckBox*                             m_originalCheckBox;
    QCheckBox*                             m_resizeCheckBox;

    QSpinBox*                              m_dimensionSpinBox;
    QSpinBox*                              m_imageQualitySpinBox;

    QPushButton*                           m_extendedPublicationButton;
    QPushButton*                           m_extendedTagsButton;
    WSComboBoxIntermediate*                m_contentTypeComboBox;
    WSComboBoxIntermediate*                m_safetyLevelComboBox;

    QString                                m_username;
    QString                                m_userId;
    QString                                m_lastSelectedAlbum;

    QLabel*                                m_userNameDisplayLabel;

    QProgressDialog*                       m_authProgressDlg;

    QList< QPair<QUrl, FPhotoInfo> >       m_uploadQueue;

    QLineEdit*                             m_tagsLineEdit;

    FlickrWidget*                          m_widget;
    FlickrTalker*                          m_talker;

    FlickrList*                            m_imglst;
    WSSelectUserDlg*                       m_select;
    FlickrNewAlbumDlg*                     m_albumDlg;

    DInfoInterface*                        m_iface;
};

} // namespace Digikam

#endif // FLICKR_WINDOW_H
