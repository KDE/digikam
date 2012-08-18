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

#ifndef ADVANCEDRENAMEINPUTTEST_H
#define ADVANCEDRENAMEINPUTTEST_H

// Qt includes

#include <QtCore/QObject>

// Local includes

#include "advancedrenamemanager.h"
#include "defaultrenameparser.h"
#include "parsesettings.h"

class AdvancedRenameWidgetTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    /*
     * TOKEN TESTS
     */
    void testFileNameToken();

    void testFileExtensionToken();
    void testFileExtensionToken_data();

    void testFileOwnerToken();

    void testFileGroupToken();

    void testDirectoryNameToken();
    void testDirectoryNameToken_data();

    void testNumberToken();
    void testNumberToken_data();

    /*
     * MODIFIER TESTS
     */
    //    void testUniqueModifier();

    void testReplaceModifier();
    void testReplaceModifier_data();

    void testRangeModifier();
    void testRangeModifier_data();

    void testDefaultValueModifier();
    void testDefaultValueModifier_data();

    void testUppercaseModifier();

    void testLowercaseModifier();

    void testFirstLetterOfEachWordUppercaseModifier_data();
    void testFirstLetterOfEachWordUppercaseModifier();

    void testChainedModifiers();
    void testChainedModifiers_data();

    void testUniqueModifier();

    /*
     * OTHER TESTS
     */
    void testEmptyParseString();
};

#endif /* ADVANCEDRENAMEINPUTTEST_H_ */
