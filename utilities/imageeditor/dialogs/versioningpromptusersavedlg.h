/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-09-16
 * Description : Dialog to prompt users about versioning
 *
 * Copyright (C) 2010-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2013-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef VERSIONINGPROMPTUSERSAVEDIALOG_H
#define VERSIONINGPROMPTUSERSAVEDIALOG_H

// Local includes

#include "triplechoicedialog.h"

namespace Digikam
{

class VersioningPromptUserSaveDialog : public TripleChoiceDialog
{
public:

    explicit VersioningPromptUserSaveDialog(QWidget* const parent, bool allowCancel = true);
    ~VersioningPromptUserSaveDialog();

    bool shallSave()    const;
    bool newVersion()   const;
    bool shallDiscard() const;
};

} // namespace Digikam

#endif // VERSIONINGPROMPTUSERSAVEDIALOG_H
