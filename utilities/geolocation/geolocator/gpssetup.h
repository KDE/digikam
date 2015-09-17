/** ===========================================================
 * @file
 *
 * This file is a part of kipi-plugins project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2010-08-27
 * @brief  GPSSetup widget for geo correlator.
 *
 * @author Copyright (C) 2010 by Michael G. Hansen
 *         <a href="mailto:mike at mghansen dot de">mike at mghansen dot de</a>
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

#ifndef GPS_SETUP_H
#define GPS_SETUP_H

// KDE includes

#include <kpagedialog.h>

class KConfig;

namespace Digikam
{

class GPSSetupGlobalObject : public QObject
{
    Q_OBJECT

public:

    static GPSSetupGlobalObject* instance();

    QVariant readEntry(const QString& name);
    void writeEntry(const QString& name, const QVariant& value);

    void triggerSignalSetupChanged();

Q_SIGNALS:

    void signalSetupChanged();

private:

    GPSSetupGlobalObject();
    ~GPSSetupGlobalObject();

private:

    class Private;
    Private* const d;

    friend class GPSSetupGlobalObjectCreator;
};

// --------------------------------------------------------------------------

class GPSSetupTemplate : public QWidget
{
    Q_OBJECT

public:

    explicit GPSSetupTemplate(QWidget* const parent = 0);
    ~GPSSetupTemplate();

public Q_SLOTS:

    virtual void slotApplySettings() = 0;

};

// --------------------------------------------------------------------------

class GPSSetup : public KPageDialog
{
    Q_OBJECT

public:

    explicit GPSSetup(QWidget* const parent = 0);
    ~GPSSetup();

Q_SIGNALS:

    void signalGPSSetupChanged();


private Q_SLOTS:

    void slotApplyClicked();
    void slotOkClicked();

private:

    class Private;
    Private* const d;
};

} /* namespace Digikam */

#endif /* GPS_SETUP_H */
