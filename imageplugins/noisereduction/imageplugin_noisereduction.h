/* ============================================================
 * File  : imageplugin_noisereduction.h
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2004-08-24
 * Description : 
 * 
 * Copyright 2004-2006 by Gilles Caulier
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


#ifndef IMAGEPLUGIN_NOISEREDUCTION_H
#define IMAGEPLUGIN_NOISEREDUCTION_H

// Digikam includes.

#include <digikamheaders.h>

class KAction;

class DIGIKAMIMAGEPLUGINS_EXPORT ImagePlugin_NoiseReduction : public Digikam::ImagePlugin
{
    Q_OBJECT
    
public:

    ImagePlugin_NoiseReduction(QObject *parent, const char* name,
                               const QStringList &args);
    ~ImagePlugin_NoiseReduction();

    void setEnabledActions(bool enable);

private:

    KAction *m_noiseReductionAction;
    
private slots:

    void slotNoiseReduction();

};
    
#endif /* IMAGEPLUGIN_NOISEREDUCTION_H */
