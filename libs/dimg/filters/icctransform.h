/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-11-18
 * Description : a class to apply ICC color correction to image.
 *
 * Copyright (C) 2005-2006 by F.J. Cruz <fj.cruz@supercable.es>
 * Copyright (C) 2005-2007 by Gilles Caulier <caulier dot gilles at gmail dot com> 
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

#ifndef ICCTRANSFORM_H
#define ICCTRANSFORM_H

// Qt includes.

#include <qstring.h>

// Local includes.

#include "dimg.h"
#include "digikam_export.h"

namespace Digikam
{

class IccTransformPriv;

class DIGIKAM_EXPORT IccTransform
{
public:

    IccTransform();
    ~IccTransform();

    bool apply(DImg& image);
    bool apply(DImg& image, QByteArray& profile, int intent,
               bool useBPC=false, bool checkGamut=false, bool useBuiltin=false);

    void getTransformType(bool do_proof_profile);
    void getEmbeddedProfile(const DImg& image);
    int  getRenderingIntent();
    bool getUseBPC();

    bool hasInputProfile();
    bool hasOutputProfile();

    QByteArray embeddedProfile() const;
    QByteArray inputProfile() const;
    QByteArray outputProfile() const;
    QByteArray proofProfile() const;

    /** Input profile from file methods */
    void setProfiles(const QString& input_profile, const QString& output_profile);
    void setProfiles(const QString& input_profile, const QString& output_profile, const QString& proof_profile);

    /** Embedded input profile methods */
    void setProfiles(const QString& output_profile);
    void setProfiles(const QString& output_profile, const QString& proof_profile, bool forProof);

    /** Profile info methods */
    QString getProfileDescription(const QString& profile);

    QString getEmbeddedProfileDescriptor();
    QString getInputProfileDescriptor();
    QString getOutpoutProfileDescriptor();
    QString getProofProfileDescriptor();

private:

    QByteArray loadICCProfilFile(const QString& filePath);

private:

    IccTransformPriv* d;

};

}  // NameSpace Digikam

#endif   // ICCTRANSFORM_H
