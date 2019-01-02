/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : abstract class to define external plugin
 *
 * Copyright (C) 2018-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include <QStringList>
#include <QtPlugin>
#include <QObject>

// Local includes

#include "dinfointerface.h"
#include "dpluginauthor.h"
#include "dpluginaction.h"
#include "digikam_export.h"

namespace Digikam
{

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

    /**
     * Return all plugin actions registered in setup() method with addAction().
     */
    QList<DPluginAction*> actions() const;

    /**
     * Return a plugin action instance found by name in plugin action list.
     */
    DPluginAction* findActionByName(const QString& name) const;

    /**
     * Return a list of plugin categories as strings.
     */
    QStringList pluginCategories() const;

    /**
     * Return a list of plugin authors as strings.
     */
    QStringList pluginAuthors() const;

    /**
     * Return the internal version used to check the binary compatibility at run-time.
     */
    QString internalVersion() const;

    /**
     * Return the loaded property.
     * If it's true, the plugin have been loaded at startup by plugin loader after a setup() call.
     */
    bool isLoaded() const;

    /**
     * Accessor to adjust the loaded plugin property.
     * This property is adjusted by plugin loader at start-up.
     */
    void setLoaded(bool b);

public:

    /**
     * Plugin factory method to create all internal actions.
     * To retrieve all plugin actions, use actions() public methods.
     * To register a new plugin action in this method, use addAction() protected method.
     */
    virtual void setup() = 0;

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

    /** 
     * @brief Returns the version string of the plugin.
     */
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

protected:

    void addAction(DPluginAction* const ac);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

Q_DECLARE_TYPEINFO(Digikam::DPluginAuthor, Q_MOVABLE_TYPE);

Q_DECLARE_INTERFACE(Digikam::DPlugin, "org.kde.digikam.DPlugin/1.0.0" )

#endif // DIGIKAM_DPLUGIN_H
