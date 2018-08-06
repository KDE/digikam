/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : template for external plugin
 *
 * Copyright (C) 2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_DPLUGIN_H
#define DIGIKAM_DPLUGIN_H

// Qt includes

#include <QList>
#include <QCoreApplication>
#include <QString>
#include <QtPlugin>
#include <QObject>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dinfointerface.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DPluginAuthor
{
public:

    explicit DPluginAuthor(const QString& n,
                           const QString& e,
                           const QString& y,
                           const QString& r = i18n("Developer"))
      : name(n),
        email(e),
        years(y),
        roles(r)
    {
    }

    QString asString() const
    {
        return (QString::fromLatin1("%1 <%2> %3 [%4]").arg(name).arg(email).arg(years).arg(roles));
    }

public:

    QString name;    // Author name and surname
    QString email;   // Email anti-spammed
    QString years;   // Copyrights years
    QString roles;   // Author roles, as "Developer", "Designer", "Translator", etc.
};

/**
 * A digiKam external plugin template class.
 */
class DIGIKAM_EXPORT DPlugin : public QObject
{
    Q_OBJECT

public:

    /**
     * Constructor with optional parent object
     */
    explicit DPlugin(QObject* const parent = 0);

    /**
     * Destructor
     */
    ~DPlugin() override;

    void setInfoIface(DInfoInterface* const iface);
    DInfoInterface* infoIface() const;

    /** Plugin factory method to create a new internal tool instance.
     */
    virtual void init() = 0;

    /**
     * @brief Returns the user-visible name of the plugin.
     *
     * The user-visible name should be context free, i.e. the name should
     * provide enough information as to what the plugin is about in the context
     * of digiKam.
     */
    virtual QString name() const = 0;

    /**
     * @brief Returns the unique identification string of the plugin.
     */
    virtual QString id() const = 0;

    virtual QString version() const = 0;

    /**
     * @brief Returns a user description of the plugin.
     */
    virtual QString description() const = 0;

    /**
     * @brief Returns an icon for the plugin.
     */
    virtual QIcon icon() const;

    /**
     * @brief Returns authors list for the plugin.
     */
    virtual QList<DPluginAuthor> authors() const = 0;

    /**
     * @brief Returns about text (credits) for external data the plugin uses.
     *
     * The default implementation returns the empty string. Please override
     * this method to give credits for all data from 3rd-partys.
     */
    virtual QString aboutDataText() const;

public Q_SLOTS:

    virtual void slotRun() = 0;

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

Q_DECLARE_TYPEINFO(Digikam::DPluginAuthor, Q_MOVABLE_TYPE);

Q_DECLARE_INTERFACE(Digikam::DPlugin, "org.kde.digikam.DPlugin/1.0.0" )

#endif // DIGIKAM_DPLUGIN_H
