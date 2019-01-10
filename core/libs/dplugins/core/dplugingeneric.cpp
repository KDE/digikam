/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : abstract class to define digiKam plugin
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

#include "dplugingeneric.h"

// Qt includes

#include <QApplication>

// Local includes

#include "digikam_version.h"
#include "digikam_debug.h"
#include "dxmlguiwindow.h"
#include "dplugindialog.h"

namespace Digikam
{

class Q_DECL_HIDDEN DPluginGeneric::Private
{
public:

    explicit Private()
    {
    }

    QList<DPluginAction*> actions;
};

DPluginGeneric::DPluginGeneric(QObject* const parent)
    : DPlugin(parent),
      d(new Private)
{
}

DPluginGeneric::~DPluginGeneric()
{
    delete d;
}

DInfoInterface* DPluginGeneric::infoIface(QObject* const ac) const
{
    DPluginAction* const pac = dynamic_cast<DPluginAction*>(ac);

    if (pac)
    {
        DXmlGuiWindow* const gui = dynamic_cast<DXmlGuiWindow*>(pac->parent());

        if (gui)
        {
            return gui->infoIface(pac);
        }

        DInfoInterface* const iface = dynamic_cast<DInfoInterface*>(pac->parent());

        if (iface)
        {
            return iface;
        }
    }

    return 0;
}

int DPluginGeneric::actionCount() const
{
    int count       = 0;
    QObject* parent = 0;

    foreach (DPluginAction* const ac, d->actions)
    {
        if (ac)
        {
            // NOTE: we will return the count of actions registered with the same parents,
            //       as each parent registered the same list of actions through setup().

            if (!count)
            {
                parent = ac->parent(),
                ++count;
            }
            else if (ac->parent() == parent)
            {
                ++count;
            }
        }
    }

    return count;
}

void DPluginGeneric::setVisible(bool b)
{
    foreach (DPluginAction* const ac, d->actions)
    {
        if (ac)
        {
            ac->setVisible(b);
        }
    }
}

QList<DPluginAction*> DPluginGeneric::actions(QObject* const parent) const
{
    QList<DPluginAction*> list;

    foreach (DPluginAction* const ac, d->actions)
    {
        if (ac && (ac->parent() == parent))
        {
            list << ac;
        }
    }

    return list;
}

DPluginAction* DPluginGeneric::findActionByName(const QString& name, QObject* const parent) const
{
    foreach (DPluginAction* const ac, actions(parent))
    {
        if (ac && (ac->objectName() == name))
        {
            return ac;
        }
    }

    return 0;
}

void DPluginGeneric::addAction(DPluginAction* const ac)
{
    ac->setProperty("DPluginIId", iid());
    d->actions.append(ac);
}

QStringList DPluginGeneric::actionCategories() const
{
    QStringList list;

    foreach (DPluginAction* const ac, d->actions)
    {
        QString cat = ac->actionCategoryToString();

        if (!list.contains(cat))
        {
            list << cat;
        }
    }

    list.sort();

    return list;
}


bool DPluginGeneric::reactivateToolDialog(QWidget* const dlg) const
{
    if (dlg && (dlg->isMinimized() || !dlg->isHidden()))
    {
        dlg->showNormal();       // krazy:exclude=qmethods
        dlg->activateWindow();
        dlg->raise();
        return true;
    }

    return false;
}

} // namespace Digikam
