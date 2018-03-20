/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-06-09
 * Description : a test for the AdvancedRename utility
 *
 * Copyright (C) 2009-2011 by Andi Clemens <andi dot clemens at gmail dot com>
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

#ifndef RENAMECUSTOMIZERTEST_H
#define RENAMECUSTOMIZERTEST_H

// Qt includes

#include <QObject>

class RenameCustomizerTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void newName_should_return_empty_string_with_empty_filename_data();
    void newName_should_return_empty_string_with_empty_filename();

    void setCaseType_set_to_none();
    void setCaseType_set_to_upper();
    void setCaseType_set_to_lower();

    void setUseDefault_true();
    void setUseDefault_false();
    void setUseDefault_case_none_should_deliver_original_filename();
    void setUseDefault_case_upper_should_deliver_uppercase_filename();
    void setUseDefault_case_lower_should_deliver_lowercase_filename();
};

#endif /* RENAMECUSTOMIZERTEST_H */
