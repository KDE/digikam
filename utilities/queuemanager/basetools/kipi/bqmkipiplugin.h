/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2014-11-15
 * Description : A generic class for plugged Kipi Tools
 *
 * Copyright (C) 2008-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2014 by Shourya Singh Gupta <shouryasgupta@gmail.com>
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

#ifndef BQMKIPIPLUGIN_H
#define BQMKIPIPLUGIN_H

// Local includes

#include "batchtool.h"
#include "kipipluginloader.h"

namespace Digikam
{  
  
class BqmKipiPlugin : public BatchTool
{
    Q_OBJECT

public:

    explicit BqmKipiPlugin(QString name, QObject* const parent = 0);
    ~BqmKipiPlugin();
    
    QString outputSuffix() const;
    BatchToolSettings defaultSettings();

    BatchTool* clone(QObject* const parent) const;
 
    void registerSettingsWidget();
   
public Q_SLOTS:

    void slotAssignSettings2Widget();
    void slotSettingsChanged();
    void slotKipiSettingsChanged(QString pluginName,QMap<QString, QVariant> settings);
    
private:
    
    EmbeddablePlugin* plugin;
    bool toolOperations();
    
};

}  // namespace Digikam

#endif /* BQMKIPIPLUGIN_H */
