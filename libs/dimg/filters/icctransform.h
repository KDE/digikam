/* ============================================================
 * Author: F.J. Cruz <fj.cruz@supercable.es>
 * Date  : 2005-11-18
 * Copyright 2005-2006 by F.J. Cruz
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
    void apply(DImg& image, QByteArray& profile, bool useBPC = false, bool checkGamut = false,
               bool useBuiltin=false);
    void getEmbeddedProfile(DImg image);
    
    //Input profile from file
    void setProfiles(QString input_profile, QString output_profile);
    void setProfiles(QString input_profile, QString output_profile, QString proof_profile);
    
    //Embedded input profile
    void setProfiles(QString output_profile);
    //void setProfiles(QString * output_profile, QString * proof_profile);

    //Profile info
    QString getProfileDescription(QString profile);

    QString getEmbeddedProfileDescriptor();
    
private:

    IccTransformPriv* d;
    
};

}  // NameSpace Digikam

#endif   // ICCTRANSFORM_H
