/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : manager to load external plugins at run-time
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

#ifndef DIGIKAM_DPLUGIN_LOADER_H
#define DIGIKAM_DPLUGIN_LOADER_H

// Qt includes

#include <QObject>
#include <QList>

// Local includes

#include "digikam_export.h"
#include "dinfointerface.h"
#include "dplugin.h"
#include "dpluginaction.h"

namespace Digikam
{

/**
 * @short The class that handles digiKam's external plugins.
 *
 * Ownership policy for plugins:
 *
 * The DPluginLoader creates new objects and transfer ownership.
 * In order to create the objects, the DPluginLoader internally has a list of the tools
 * which are owned by the DPluginLoader and destroyed by it.
 *
 */
class DIGIKAM_EXPORT DPluginLoader : public QObject
{
    Q_OBJECT

 public:

    /**
     * @brief instance: returns the singleton of plugin loader
     * @return DPluginLoader global instance
     */
    static DPluginLoader* instance();

    /**
     * Return the config group name used to store the list of plugins to load at startup.
     */
    QString configGroupName() const;

    /**
     * Register all Generic plugin actions to parent object.
     */
    void registerGenericPlugins(QObject* const parent);

    /**
     * Register all Editor plugin actions to parent object.
     */
    void registerEditorPlugins(QObject* const parent);

    /**
     * Init plugin loader. Call this method to parse and load relevant plugins installed on your system.
     */
    void init();

     /**
     * Unload all loaded plugins. Call this method before the main instance is closed.
     */
    void cleanUp();

    /**
     * @brief Returns all available plugins.
     */
    QList<DPlugin*> allPlugins() const;

    /**
     * @brief Returns a list of plugin actions set as type for a given parent.
     *        If no plugin have found in this category, this returns an empty list.
     */
    QList<DPluginAction*> pluginsActions(DPluginAction::ActionType type, QObject* const parent) const;

    /**
     * @brief Returns a list of plugin actions set as category for a given parent.
     *        If no plugin have found in this category, this returns an empty list.
     */
    QList<DPluginAction*> pluginsActions(DPluginAction::ActionCategory cat, QObject* const parent) const;

    /**
     * @brief Returns the plugin actions corresponding to a plugin internal ID string for a given parent.
     *        If not found, this returns an empty list.
     */
    QList<DPluginAction*> pluginActions(const QString& pluginIID, QObject* const parent) const;

    /**
     * @brief Returns the plugin action corresponding to a action name for a given parent.
     *        If not found, this returns a null pointer.
     */
    DPluginAction* pluginAction(const QString& actionName, QObject* const parent) const;

    /**
     * @brief Returns all xml sections as string of plugin actions set with a kind of category for a given parent.
     */
    QString pluginXmlSections(DPluginAction::ActionCategory cat, QObject* const parent) const;

    /**
     * @brief appendPluginToBlackList Prevent that a plugin is loaded from the given filename
     * @param filename The name of the file excluding file extension to blacklist. E.g.
     * to ignore "HtmlGalleryPlugin.so" on Linux and "HtmlGalleryPlugin.dll" on Windows, pass "HtmlGalleryPlugin"
     */
    void appendPluginToBlackList(const QString& filename);

    /**
     * @brief appendPluginToWhiteList Add a plugin to the whitelist of tools. If the whitelist is not
     * empty, only whitelisted tools are loaded. If a tool is both whitelisted and blacklisted,
     * it will not be loaded.
     * @param filename The name of the file excluding file extension to whitelist. E.g.
     * to not ignore "HtmlGalleryPlugin.so" on Linux and "HtmlGalleryPlugin.dll" on Windows, pass "HtmlGalleryPlugin"
     */
    void appendPluginToWhiteList(const QString& filename);

private:

    // Disable constructor and destructor
    DPluginLoader();
    ~DPluginLoader();

    Q_DISABLE_COPY(DPluginLoader)

    class Private;
    Private* const d;

    friend class DPluginLoaderCreator;
};

} // namespace Digikam

#endif // DIGIKAM_DPLUGIN_LOADER_H
