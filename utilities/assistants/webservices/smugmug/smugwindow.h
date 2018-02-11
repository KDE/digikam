/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-17-06
 * Description : a tool to export images to Smugmug web service
 *
 * Copyright (C) 2005-2008 by Vardhman Jain <vardhman at gmail dot com>
 * Copyright (C) 2008-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2008-2009 by Luka Renko <lure at kubuntu dot org>
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

#ifndef SMUG_WINDOW_H
#define SMUG_WINDOW_H

// Qt includes

#include <QList>
#include <QUrl>
#include <QCloseEvent>

// Local includes

#include "wstooldialog.h"
#include "wslogindialog.h"
#include "smugitem.h"
#include "dinfointerface.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT SmugWindow : public WSToolDialog
{
    Q_OBJECT

public:

    explicit SmugWindow(DInfoInterface* const iface,
                        QWidget* const parent,
                        bool import=false);
    ~SmugWindow();

    /**
     * Use this method to (re-)activate the dialog after it has been created
     * to display it. This also loads the currently selected images.
     */
    void reactivate();

protected:

    void closeEvent(QCloseEvent*) Q_DECL_OVERRIDE;

private Q_SLOTS:

    void slotBusy(bool val);
    void slotLoginProgress(int step, int maxStep, const QString& label);
    void slotLoginDone(int errCode, const QString& errMsg);
    void slotAddPhotoDone(int errCode, const QString& errMsg);

    void slotGetPhotoDone(int errCode,
                          const QString& errMsg,
                          const QByteArray& photoData);

    void slotCreateAlbumDone(int errCode,
                             const QString& errMsg,
                             qint64 newAlbumID,
                             const QString& newAlbumKey);

    void slotListAlbumsDone(int errCode,
                            const QString& errMsg,
                            const QList <SmugAlbum>& albumsList);

    void slotListPhotosDone(int errCode,
                            const QString& errMsg,
                            const QList <SmugPhoto>& photosList);

    void slotListAlbumTmplDone(int errCode,
                               const QString& errMsg,
                               const QList <SmugAlbumTmpl>& albumTList);

    void slotListCategoriesDone(int errCode,
                                const QString& errMsg,
                                const QList <SmugCategory>& categoriesList);

    void slotListSubCategoriesDone(int errCode,
                                   const QString& errMsg,
                                   const QList <SmugCategory>& categoriesList);

    void slotUserChangeRequest(bool anonymous);
    void slotReloadAlbumsRequest();
    void slotNewAlbumRequest();

    void slotStartTransfer();
    void slotCancelClicked();
    void slotStopAndCloseProgressBar();
    void slotDialogFinished();

    void slotImageListChanged();
    void slotTemplateSelectionChanged(int index);
    void slotCategorySelectionChanged(int index);

private:

    bool prepareImageForUpload(const QString& imgPath) const;
    void uploadNextPhoto();
    void downloadNextPhoto();

    void readSettings();
    void writeSettings();

    void authenticate(const QString& email = QString(),
                      const QString& password = QString());

    void buttonStateChange(bool state);
    void setUiInProgressState(bool inProgress);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // SMUG_WINDOW_H
