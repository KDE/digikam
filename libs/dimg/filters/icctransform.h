/* ============================================================
 * Author: F.J. Cruz <fj.cruz@supercable.es>
 * Date  : 2005-11-18
 * Copyright 2005 by F.J. Cruz
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
 * ============================================================ */
 
#ifndef ICCTRANSFORM_H
#define ICCTRANSFORM_H

// Qt includes.

#include <qstring.h>
#include <qcstring.h>

// Local includes.

#include "digikam_export.h"

namespace Digikam
{

class DImg;

class DIGIKAM_EXPORT IccTransform
{
public:
    
    IccTransform();
    ~IccTransform();
    
    void getTransformType(bool do_proof_profile);
    void apply(DImg& image);
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

    bool        m_do_proof_profile;
    bool        m_has_profile; 
    
    QString     m_input_profile;
    QString     m_output_profile;
    QString     m_proof_profile;
    
    QByteArray  m_embedded_profile;
};

}  // NameSpace Digikam

#endif   // ICCTRANSFORM_H
