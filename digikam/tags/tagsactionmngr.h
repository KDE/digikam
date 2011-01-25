/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-01-24
 * Description : Tags Action Manager
 *
 * Copyright (C) 2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef TAGSACTIONMNGR_H
#define TAGSACTIONMNGR_H

// Qt includes

#include <QMap>
#include <QObject>
#include <QWidget>

// KDE includes

#include <ksharedconfig.h>

// Local includes

#include "tagproperties.h"
#include "albuminfo.h"

class KActionCollection;

namespace Digikam
{

class TagsActionMngr : public QObject
{
    Q_OBJECT

public:

      TagsActionMngr(QWidget* parent, KActionCollection* actionCollection);
      ~TagsActionMngr();

      /** Create all actions relevant of tags from DB which have a keyboard shortcut.
       */
      void createActions();

      static TagsActionMngr* defaultManager();

Q_SIGNALS:

      void signalAssignTagsFromShortcut(int);

public Q_SLOTS:

      /**
       * Updates the shortcut action for this collection. Call this when a shortcut was
       * added, removed or changed.
       */
      void slotUpdateTagShortcut(int tagId, const QString& ks);

      /**
       * Removes the shortcut actions associated with a folder.
       */
      void slotTagRemoved(int tagId);

private:

      bool createTagActionShortcut(int tagId);
      void createTagActionShortcut(const TagInfo& tinfo, const TagProperties& tprop);

private:

      static TagsActionMngr* m_defaultManager;

      class TagsActionMngrPrivate;
      TagsActionMngrPrivate* const d;
};

} // namespace Digikam

#endif // TAGSACTIONMNGR_H
