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

// Local includes

#include "kp4tooldialog.h"

class QCloseEvent;

class QUrl;

namespace KIPI
{
    class Interface;
}

namespace KIPIPlugins
{
    class KPAboutData;
}

using namespace KIPI;
using namespace KIPIPlugins;

namespace GenericDigikamDebianScreenshotsPlugin
{

class DSTalker;
class DSWidget;

class DSWindow : public KP4ToolDialog
{
    Q_OBJECT

    Q_ENUMS(MassageKind)

public:

    explicit DSWindow(const QString& tmpFolder, QWidget* const parent);
    ~DSWindow();

    /**
     * Use this method to (re-)activate the dialog after it has been created
     * to display it. This also loads the currently selected images.
     */
    void reactivate();

private Q_SLOTS:

    void slotStartTransfer();
    void slotMaybeEnableUser1();
    void slotButtonClicked(int button);
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
    void closeEvent(QCloseEvent*);

private:

    bool         m_uploadEnabled;

    unsigned int m_imagesCount;
    unsigned int m_imagesTotal;

    QString      m_tmpDir;
    QString      m_tmpPath;

    QUrl::List   m_transferQueue;

    DSTalker*    m_talker;
    DSWidget*    m_widget;
};

} // namespace GenericDigikamDebianScreenshotsPlugin

#endif // DIGIKAM_DS_WINDOW_H
