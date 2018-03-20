/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-06-21
 * Description : a simple web browser dialog based on Qt WebEngine.
 *
 * Copyright (C) 2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef WEB_BROWSER_DLG_H
#define WEB_BROWSER_DLG_H

// Qt include

#include <QDialog>
#include <QWidget>
#include <QUrl>
#include <QString>
#include <QCloseEvent>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class SearchTextSettings;

class DIGIKAM_EXPORT WebBrowserDlg : public QDialog
{
    Q_OBJECT

public:

    explicit WebBrowserDlg(const QUrl& url, QWidget* const parent = 0);
    ~WebBrowserDlg();

private Q_SLOTS:

    void slotUrlChanged(const QUrl&);
    void slotLoadingStarted();
    void slotLoadingFinished(bool);
    void slotIconChanged(const QIcon&);
    void slotTitleChanged(const QString&);
    void slotSearchTextChanged(const SearchTextSettings&);
    void slotGoHome();
    void slotDesktopWebBrowser();

protected:

    void closeEvent(QCloseEvent*);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // WEB_BROWSER_DLG_H
