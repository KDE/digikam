/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-06-04
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju and Gilles Caulier
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


#ifndef IMAGEPLUGIN_CORE_H
#define IMAGEPLUGIN_CORE_H

// Digikam includes.

#include <imageplugin.h>

class KAction;

class ImagePlugin_Core : public Digikam::ImagePlugin
{
    Q_OBJECT
    
public:

    ImagePlugin_Core(QObject *parent, const char* name,
                     const QStringList &args);
    ~ImagePlugin_Core();

    QStringList guiDefinition() const;

    void setEnabledSelectionActions(bool enable);

private:

    KAction     *m_redeyeAction;
        
private slots:

    void slotBlur();
    void slotSharpen();
    void slotBCG();
    void slotRGB();
    void slotHSL();
    void slotNormalize();
    void slotEqualize();
    void slotAutoLevels();
    void slotHistogramViewer();
    void slotBW();
    void slotSepia();
    void slotRedEye();

};
    
#endif /* IMAGEPLUGIN_CORE_H */
