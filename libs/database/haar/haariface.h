/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-01-17
 * Description : Haar Database interface
 *
 * Copyright (C) 2003 by Ricardo Niederberger Cabral <nieder at mail dot ru>
 * Copyright (C) 2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef HAARIFACE_H
#define HAARIFACE_H

// C++ Includes

#include <map>
#include <list>

// Qt includes.

#include <QString>

// Local includes.

#include "haar.h"

namespace Digikam
{

class HaarIface
{

public:

    HaarIface();
    ~HaarIface();

private:

    /** signature structure 
    */
    typedef struct sigStruct_
    {
        long int  id;               // picture id
        Haar::Idx sig1[NUM_COEFS];  // Y positions with largest magnitude
        Haar::Idx sig2[NUM_COEFS];  // I positions with largest magnitude
        Haar::Idx sig3[NUM_COEFS];  // Q positions with largest magnitude
        double    avgl[3];          // YIQ for position [0,0]
        double    score;            // used when doing queries

        // image properties extracted when opened for the first time
        int       width;            // in pixels
        int       height;           // in pixels

        bool operator< (const sigStruct_& right) const 
        {
            return score < (right.score);
        }
    } sigStruct;

    struct cmpf
    {
        bool operator()(const long int s1, const long int s2) const
        {
            return s1<s2;
        }
    };

    typedef std::map<const long int, sigStruct*, cmpf>::iterator sigIterator;
    typedef std::list<long int>                                  long_list;
    typedef long_list::iterator                                  long_listIterator;
    typedef std::priority_queue<sigStruct>                       priqueue;
    typedef std::list<long_list>                                 long_list_2;       // a list of lists
    typedef std::map<const long int, sigStruct*, cmpf>           sigMap;

    sigMap        m_sigs;

    unsigned char m_imgBin[16384];

    /** Lists of picture ids, indexed by [color-channel][sign][position], i.e.,
        R=0/G=1/B=2, pos=0/neg=1, (i*NUM_PIXELS+j)
    */
    long_list     m_imgbuckets[3][2][16384];

    /** results priority queue; largest at top 
    */
    priqueue      m_pqResults;

    /** current result waiting to be returned 
    */
    sigStruct     m_curResult;

    /** number of results found 
    */
    int           m_numres;

private:

    Haar::ImageData *m_data;
    Haar::WeightBin *m_bin;

    //void        initImgBin();
    //void        freeSigs();

    //int         getImageWidth(long int id);
    //int         getImageHeight(long int id);

    QImage      loadQImage(const QString &filename);
    void        fillPixelData(const QImage &image, Haar::ImageData *data);
    bool        addImage(const QString& filename);
    void        queryImgData(Haar::Idx* sig1, Haar::Idx* sig2, Haar::Idx* sig3,
                             double* avgl, int numres, int sketch);
    bool        queryImgFile(const QString& filename, int numres, int sketch);
    /*
    long_list   queryImgDataForThres(sigMap* tsigs, Haar::Idx* sig1, Haar::Idx* sig2, Haar::Idx* sig3,
                                     double* avgl, float thresd, int sketch);
    long_list   queryImgDataForThresFast(sigMap* tsigs, double* avgl, float thresd, int sketch);

    long_list_2 clusterSim(float thresd, bool fast=false);
    int         getNumResults();
    long int    getResultID();
    double      getResultScore();

    void        queryImgID(long int id, int numres);
    */
    //double      calcAvglDiff(long int id1, long int id2);
    //double      calcDiff(long int id1, long int id2);

    // TODO: Marcel, these methods can be removed.
#if 0
    int         loadDB(char* filename);
    int         saveDB(char* filename);
    int         resetDB();
    void        removeIDFromDB(long int id);

    int         getLongListSize(long_list& li);
    long int    popLongList(long_list& li);
    int         getLongList2Size(long_list_2& li);
    long_list   popLong2List(long_list_2& li);
#endif
};

}  // namespace Digikam

#endif // HAARIFACE_H
