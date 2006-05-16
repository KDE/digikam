/* ============================================================
 * Authors: F.J. Cruz <fj.cruz@supercable.es>
 *          Gilles Caulier <caulier dot gilles at kdemail dot net> 
 * Date   : 2005-11-18
 * Description : a class to apply ICC color correction to image.
 * 
 * Copyright 2005-2006 by F.J. Cruz and Gilles Caulier
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
    
    void getTransformType(bool do_proof_profile);
    void apply(DImg& image);
    void apply(DImg& image, QByteArray& profile, int intent, 
               bool useBPC=false, bool checkGamut=false, bool useBuiltin=false);
    void getEmbeddedProfile(DImg image);

    int  getRenderingIntent();
    
    QByteArray embeddedProfile() const;
    QByteArray inputProfile() const;
    QByteArray outputProfile() const;
    QByteArray proofProfile() const;
    
    //Input profile from file
    void setProfiles(QString input_profile, QString output_profile);
    void setProfiles(QString input_profile, QString output_profile, QString proof_profile);
    
    //Embedded input profile
    void setProfiles(QString output_profile);
    void setProfiles(QString output_profile, QString proof_profile, bool forProof);

    //Profile info
    QString getProfileDescription(QString profile);

    QString getEmbeddedProfileDescriptor();
    QString getInputProfileDescriptor();
    QString getOutpoutProfileDescriptor();
    QString getProofProfileDescriptor();

    bool hasOutputProfile();
    
private:

    QByteArray loadICCProfilFile(const QString& filePath);

private:

    IccTransformPriv* d;
    
};

}  // NameSpace Digikam

#endif   // ICCTRANSFORM_H
