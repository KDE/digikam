/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-10-15
 * Description : XMP subjects settings page.
 *
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009      by Andi Clemens <andi dot clemens at googlemail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "xmpsubjects.h"

// Qt includes

#include <QValidator>

// KDE includes

#include <klocalizedstring.h>

namespace Digikam
{

XMPSubjects::XMPSubjects(QWidget* const parent)
    : SubjectWidget(parent)
{
    // Subject string do not accept these characters:
    // - '*' (\x2A)
    // - ':' (\x3A)
    // - '?' (\x3F)
    QRegExp subjectRx(QLatin1String("[^*:?]+$"));
    QValidator* const subjectValidator = new QRegExpValidator(subjectRx, this);

    // --------------------------------------------------------

    m_iprEdit->setText(QLatin1String("XMP"));
    m_iprEdit->setValidator(subjectValidator);
    m_iprEdit->setWhatsThis(i18n("Enter here the Informative Provider Reference. "
                                  "I.P.R is a name registered with the XMP/NAA, identifying the "
                                  "provider that provides an indicator of the content. "
                                  "The default value for the I.P.R is \"XMP\" if a standard Reference "
                                  "Code is used."));

    m_refEdit->setWhatsThis(i18n("Enter here the Subject Reference Number. "
                                 "Provides a numeric code to indicate the Subject Name plus "
                                 "optional Subject Matter and Subject Detail Names in the "
                                 "language of the service. Subject Reference is a number "
                                 "from the range 01000000 to 17999999 and represent a "
                                 "language independent international reference to "
                                 "a Subject. A Subject is identified by its Reference Number "
                                 "and corresponding Names taken from a standard lists given "
                                 "by XMP/NAA. If a standard reference code is used, these lists "
                                 "are the English language reference versions. "
                                 "This field is limited to 8 digit code."));

    m_nameEdit->setValidator(subjectValidator);
    m_nameEdit->setWhatsThis(i18n("Enter here the Subject Name. English language is used "
                                  "if you selected a standard XMP/NAA reference code."));

    m_matterEdit->setValidator(subjectValidator);
    m_matterEdit->setWhatsThis(i18n("Enter here the Subject Matter Name. English language is used "
                                    "if you selected a standard XMP/NAA reference code."));

    m_detailEdit->setValidator(subjectValidator);
    m_detailEdit->setWhatsThis(i18n("Enter here the Subject Detail Name. English language is used "
                                    "if you selected a standard XMP/NAA reference code."));

    // reset the note label, not used in XMP view
    delete m_note;

    m_subjectsCheck->setVisible(true);
    m_subjectsCheck->setEnabled(true);
}

XMPSubjects::~XMPSubjects()
{
}

void XMPSubjects::readMetadata(QByteArray& xmpData)
{
    DMetadata meta;
    meta.setXmp(xmpData);
    setSubjectsList(meta.getXmpSubjects());
}

void XMPSubjects::applyMetadata(QByteArray& xmpData)
{
    DMetadata meta;
    meta.setXmp(xmpData);
    QStringList newSubjects = subjectsList();

    // We remove in first all existing subjects.
    meta.removeXmpTag("Xmp.iptc.SubjectCode");

    // And add new list if necessary.
    if (m_subjectsCheck->isChecked())
        meta.setXmpSubjects(newSubjects);

    xmpData = meta.getXmp();
}

}  // namespace Digikam
