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

#ifndef DIGIKAM_DS_WINDOW_H
#define DIGIKAM_DS_WINDOW_H

// Qt includes

#include <QUrl>
#include <QList>

// Local includes

#include "wstooldialog.h"
#include "dinfointerface.h"

class QCloseEvent;
class QAbstractButton;

using namespace Digikam;

namespace DigikamGenericDebianScreenshotsPlugin
{

class DSTalker;
class DSWidget;

class DSWindow : public WSToolDialog
{
    Q_OBJECT
    Q_ENUMS(MassageKind)

public:

    explicit DSWindow(DInfoInterface* const iface, QWidget* const parent);
    ~DSWindow();

    /**
     * Use this method to (re-)activate the dialog after it has been created
     * to display it. This also loads the currently selected images.
     */
    void reactivate();

private Q_SLOTS:

    void slotStartTransfer();
    void slotMaybeEnableStartButton();
    void slotButtonClicked(QAbstractButton*);
    void slotRequiredPackageInfoAvailableReceived(bool enabled);
    void slotAddScreenshotDone(int errCode, const QString& errMsg);
    void slotStopAndCloseProgressBar();

private:

    enum MassageType
    {
        None = 0,
        ImageIsRaw,
        ResizeRequired,
        NotPNG
    };

private:

    bool prepareImageForUpload(const QString& imgPath, MassageType massage);
    void uploadNextPhoto();
    void buttonStateChange(bool state);
    void closeEvent(QCloseEvent*) override;

private:

    bool         m_uploadEnabled;

    unsigned int m_imagesCount;
    unsigned int m_imagesTotal;

    QString      m_tmpDir;
    QString      m_tmpPath;

    QList<QUrl>  m_transferQueue;

    DSTalker*    m_talker;
    DSWidget*    m_widget;
};

} // namespace DigikamGenericDebianScreenshotsPlugin

#endif // DIGIKAM_DS_WINDOW_H
