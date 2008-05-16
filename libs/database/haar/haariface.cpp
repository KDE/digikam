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

// C++ Includes

#include <fstream>
#include <cmath>

// QT Includes

#include <QImage>
#include <QImageReader>

// Local includes.

#include "ddebug.h"
#include "jpegutils.h"
#include "haariface.h"

using namespace std;

namespace Digikam
{

HaarIface::HaarIface()
{
}

HaarIface::~HaarIface()
{
}

/** setup initial fixed haar weights that each coefficient represents
*/
void HaarIface::initImgBin()
{
    int i, j;

    /*
    0 1 2 3 4 5 6 i
    0 0 1 2 3 4 5 5
    1 1 1 2 3 4 5 5
    2 2 2 2 3 4 5 5
    3 3 3 3 3 4 5 5
    4 4 4 4 4 4 5 5
    5 5 5 5 5 5 5 5
    5 5 5 5 5 5 5 5
    j
    */

    // Every position has value 5
    memset(m_imgBin, 5, NUM_PIXELS_SQUARED);

    // Except for the 5 by 5 upper-left quadrant
    for (i = 0; i < 5; i++)
    {
        for (j = 0; j < 5; j++)
        {
            m_imgBin[i*128+j] = qMax(i, j);
            // NOTE: imgBin[0] == 0
        }
    }
}

void HaarIface::free_sigs()
{
    for (sigIterator it = m_sigs.begin(); it != m_sigs.end(); it++)
        delete (*it).second;
}

int HaarIface::getImageWidth(long int id)
{
    if (!m_sigs.count(id))
        return 0;
    return m_sigs[id]->width;
}

int HaarIface::getImageHeight(long int id)
{
    if (!m_sigs.count(id))
        return 0;
    return m_sigs[id]->height;
}

/** id is a unique image identifier
    filename is the image location
    thname is the thumbnail location for this image
    doThumb should be set to 1 if you want to save the thumbnail on thname
    Images with a dimension smaller than ignDim are ignored
*/
int HaarIface::addImage(const long int id, char* filename, char* thname, int doThumb, int ignDim)
{
    int cn;

    // Made static for speed; only used locally
    static Haar::Unit cdata1[16384];
    static Haar::Unit cdata2[16384];
    static Haar::Unit cdata3[16384];

    int    i;
    int    width, height;
    QImage image;

    // TODO: Marcel, all can be optimized here using DImg.

    if (isJpegImage(filename))
    {
        // use fast jpeg loading
        if (!loadJPEGScaled(image, filename, 128))
        {
            // try QT now.
            if (!image.load(filename))
                return 0;

            width  = image.width();
            height = image.height();
            if (ignDim && (width <= ignDim || height <= ignDim))
                return 2;
        }
        else
        {
            // fast jpeg succeeded
            width  = image.width();
            height = image.height();
            if (ignDim && (width <= ignDim || height <= ignDim))
                return 2;
        }
    }
    else
    {
        // use default QT image loading
        if (!image.load(filename))
            return 0;

        width  = image.width();
        height = image.height();
        if (ignDim && (width <= ignDim || height <= ignDim))
            return 2;
    }

    if (doThumb)
        image.scaled(128, 128, Qt::KeepAspectRatio).save(thname, "PNG");

    image = image.scaled(128, 128);

    for (i = 0, cn = 0; i < 128; i++)
    {
        // Get a scanline:
        QRgb *line = (QRgb*)image.scanLine(i);

        for (int j = 0; j < 128; j++)
        {
            QRgb pixel = line[j];

            cdata1[cn] = qRed  (pixel);
            cdata2[cn] = qGreen(pixel);
            cdata3[cn] = qBlue (pixel);
            cn++;
        }
    }

    Haar haar;
    haar.transform(cdata1, cdata2, cdata3);

    sigStruct* nsig = new sigStruct();
    nsig->id        = id;
    nsig->width     = width;
    nsig->height    = height;

    if (m_sigs.count(id))
    {
        DDebug() << "ID already in DB: %ld\n" << endl;
        delete m_sigs[id];
        m_sigs.erase(id);
    }
    m_sigs[id] = nsig;

    haar.calcHaar(cdata1, cdata2, cdata3,
                  nsig->sig1, nsig->sig2, nsig->sig3, nsig->avgl);

    // populate buckets
    for (i = 0; i < NUM_COEFS; i++)
    {
        int x, t;

        // sig[i] never 0

        x = nsig->sig1[i];
        t = (x < 0);         // t = 1 if x neg else 1
        // x - 0 ^ 0 = x; i - 1 ^ 0b111..1111 = 2-compl(x) = -x
        x = (x - t) ^ -t;
        m_imgbuckets[0][t][x].push_back(id);

        x = nsig->sig2[i];
        t = (x < 0);
        x = (x - t) ^ -t;
        m_imgbuckets[1][t][x].push_back(id);

        x = nsig->sig3[i];
        t = (x < 0);
        x = (x - t) ^ -t;
        m_imgbuckets[2][t][x].push_back(id);
    }
    return 1;
}

/** sig1,2,3 are int arrays of length NUM_COEFS
    avgl is the average luminance
    numres is the max number of results
    sketch (0 or 1) tells which set of haar weights to use
*/
void HaarIface::queryImgData(Haar::Idx* sig1, Haar::Idx* sig2, Haar::Idx* sig3,
                             double* avgl, int numres, int sketch)
{
    int        idx, c;
    int        pn;
    Haar::Idx *sig[3] = {sig1, sig2, sig3};

    for (sigIterator sit = m_sigs.begin(); sit!=m_sigs.end(); sit++)
    {
        //TODO: do I really need to score every single sig on db?
        (*sit).second->score = 0;
        for (c = 0; c < 3; c++)
        {
            (*sit).second->score += s_haar_weights[sketch][0][c] * fabs((*sit).second->avgl[c] - avgl[c]);
        }
    }

    for (int b = 0; b < NUM_COEFS; b++)
    {
        // for every coef on a sig
        for (c = 0; c < 3; c++)
        {
            pn  = sig[c][b] <= 0;
            idx = (sig[c][b] - pn) ^ -pn;

            // update the score of every image which has this coef
            long_listIterator end = m_imgbuckets[c][pn][idx].end();
            for (long_listIterator uit = m_imgbuckets[c][pn][idx].begin(); uit != end; uit++)
            {
                m_sigs[(*uit)]->score -= s_haar_weights[sketch][m_imgBin[idx]][c];
            }
        }
    }

    // make sure pqResults is empty.
    // TODO: any faster way to empty it ? didn't find any on STL refs.
    while(!m_pqResults.empty())
        m_pqResults.pop();

    sigIterator sit = m_sigs.begin();

    // Fill up the numres-bounded priority queue (largest at top):
    for (int cnt = 0; cnt < numres; cnt++)
    {
        // No more images; cannot get requested numres, alas.
        if (sit == m_sigs.end())
            return;

        m_pqResults.push(*(*sit).second);
        sit++;
    }

    for (; sit != m_sigs.end(); sit++)
    {
        if ((*sit).second->score < m_pqResults.top().score)
        {
            // Make room by dropping largest entry:
            m_pqResults.pop();
            // Insert new entry:
            m_pqResults.push(*(*sit).second);
        }
    }
}

/** sig1,2,3 are int arrays of lenght NUM_COEFS
    avgl is the average luminance
    thresd is the limit similarity threshold. Only images with score > thresd will be a result
    `sketch' tells which set of haar weights to use
    sigs is the source to query on (map of signatures)
    every search result is removed from sigs. (right now this functn is only used by clusterSim)
*/
HaarIface::long_list HaarIface::queryImgDataForThres(sigMap* tsigs, Haar::Idx* sig1, Haar::Idx* sig2, Haar::Idx* sig3,
                                                     double* avgl, float thresd, int sketch)
{
    int        idx, c;
    int        pn;
    long_list  res;
    Haar::Idx *sig[3] = {sig1, sig2, sig3};

    for (sigIterator sit = (*tsigs).begin(); sit!=(*tsigs).end(); sit++)
    {
        // TODO: do I really need to score every single sig on db?
        (*sit).second->score = 0;
        for ( c = 0;c<3;c++)
        {
            (*sit).second->score += s_haar_weights[sketch][0][c] * fabs((*sit).second->avgl[c]-avgl[c]);
        }
    }

    for (int b = 0; b < NUM_COEFS; b++)
    {
        // for every coef on a sig
        for ( c = 0; c < 3; c++)
        {
            pn  = sig[c][b] <= 0;
            idx = (sig[c][b] - pn) ^ -pn;

            // update the score of every image which has this coef
            long_listIterator end = m_imgbuckets[c][pn][idx].end();
            for (long_listIterator uit = m_imgbuckets[c][pn][idx].begin(); uit != end; uit++)
            {
                if ((*tsigs).count((*uit)))
                    (*tsigs)[(*uit)]->score -= s_haar_weights[sketch][m_imgBin[idx]][c];
            }
        }
    }

    for (sigIterator sit = (*tsigs).begin(); sit != (*tsigs).end(); sit++)
    {
        if ((*sit).second->score < thresd)
        {
            res.push_back((*sit).second->id);
            (*tsigs).erase((*sit).second->id);
        }
    }
    return res;
}

HaarIface::long_list HaarIface::queryImgDataForThresFast(sigMap* tsigs, double* avgl, float thresd, int sketch)
{
    // will only look for average luminance
    long_list res;

    for (sigIterator sit = (*tsigs).begin(); sit != (*tsigs).end(); sit++)
    {
        (*sit).second->score = 0;
        for (int c = 0; c < 3; c++)
        {
            (*sit).second->score += s_haar_weights[sketch][0][c] * fabs((*sit).second->avgl[c]-avgl[c]);
        }

        if ((*sit).second->score < thresd)
        {
            res.push_back((*sit).second->id);
            (*tsigs).erase((*sit).second->id);
        }
    }
    return res;
}

/** Cluster by similarity. Returns list of list of long ints (img ids)
*/
HaarIface::long_list_2 HaarIface::clusterSim(float thresd, int fast)
{
    // will hold a list of lists. ie. a list of clusters
    long_list_2 res;

    // temporary map of sigs, as soon as an image becomes part of a cluster, it's removed from this map
    sigMap wSigs(m_sigs);

    // temporary map of sigs, as soon as an image becomes part of a cluster, it's removed from this map
    sigMap wSigsTrack(m_sigs);

    for (sigIterator sit = wSigs.begin(); sit != wSigs.end(); sit++)
    {
        // for every img on db
        long_list res2;

        if (fast)
        {
            res2 = queryImgDataForThresFast(&wSigs, (*sit).second->avgl, thresd, 1);
        }
        else
        {
            res2 = queryImgDataForThres(&wSigs, 
                                        (*sit).second->sig1, (*sit).second->sig2, (*sit).second->sig3,
                                        (*sit).second->avgl, thresd, 1);
        }
        //    continue;
        long int hid = (*sit).second->id;
        //    if ()
        wSigs.erase(hid);
        if (res2.size() <= 1)
        {
            // everything already added to a cluster sim. Bail out immediately, otherwise next iteration
            // will segfault when incrementing sit
            if (wSigs.size() <= 1)
                break;

            continue;
        }
        res2.push_front(hid);
        res.push_back(res2);
        if (wSigs.size() <= 1)  break;
        // sigIterator sit2 = wSigs.end();
        // sigIterator sit3 = sit++;
    }
    return res;
}

/** get the number of results the last query yielded
*/
int HaarIface::getNumResults()
{
    return m_pqResults.size();
}

/** get the id of the current query result, removing it from the result list
    (you should always call getResultID *before* getResultScore)
*/
long int HaarIface::getResultID()
{
    m_curResult = m_pqResults.top();
    m_pqResults.pop();
    return m_curResult.id;
}

/** get the score for the current query result
*/
double HaarIface::getResultScore()
{
    return m_curResult.score;
}

/** query for images similar to the one that has this id
    numres is the maximum number of results
*/
void HaarIface::queryImgID(long int id, int numres)
{
    while(!m_pqResults.empty())
        m_pqResults.pop();

    if (!m_sigs.count(id))
    {
        DDebug() << "ID not found." << endl;
        return;
    }
    queryImgData(m_sigs[id]->sig1, m_sigs[id]->sig2, m_sigs[id]->sig3,
                 m_sigs[id]->avgl, numres, 0);
}

/** query for images similar to the one on filename
    numres is the maximum number of results
    sketch should be 1 if this image is a drawing
*/
int HaarIface::queryImgFile(char* filename,int numres,int sketch)
{
    while(!m_pqResults.empty())
        m_pqResults.pop();

    double     avgl[3];
    int        cn = 0;
    QImage     image;
    Haar::Idx  sig1[NUM_COEFS];
    Haar::Idx  sig2[NUM_COEFS];
    Haar::Idx  sig3[NUM_COEFS];
    Haar::Unit cdata1[16384];
    Haar::Unit cdata2[16384];
    Haar::Unit cdata3[16384];

    if (!image.load(filename))
        return 0;

    if (image.width() != 128 || image.height() != 128)
        image = image.scaled(128,128);

    for (int i = 0; i < 128; i++)
    {
        // Get a scanline:
        QRgb *line = (QRgb *) image.scanLine(i);

        for (int j = 0; j < 128; j++)
        {
            QRgb pixel = line[j];

            cdata1[cn] = qRed  (pixel);
            cdata2[cn] = qGreen(pixel);
            cdata3[cn] = qBlue (pixel);
            cn++;
        }
    }

    Haar haar;
    haar.transform(cdata1, cdata2, cdata3);
    haar.calcHaar(cdata1, cdata2, cdata3, sig1, sig2, sig3, avgl);
    queryImgData(sig1, sig2, sig3, avgl, numres, sketch);

    return 1;
}

/** remove image with this id from dbase
*/
void HaarIface::removeID(long int id)
{
    if (!m_sigs.count(id))
    {
        // don't remove something which isn't even on db.
        DDebug() << "Attempt to remove invalid id: " << id << endl;
        return;
    }
    delete m_sigs[id];
    m_sigs.erase(id);

    // remove id from each bucket it could be in
    for (int c = 0; c < 3; c++)
    {
        for (int pn=0; pn < 2; pn++)
        {
            for (int i = 0; i < 16384; i++)
            {
                m_imgbuckets[c][pn][i].remove(id);
            }
        }
    }
}

/** return the average luminance difference
*/
double HaarIface::calcAvglDiff(long int id1, long int id2)
{
    if (!m_sigs.count(id1))
        return 0;

    if (!m_sigs.count(id2))
        return 0;

    return fabs(m_sigs[id1]->avgl[0] - m_sigs[id2]->avgl[0]) +
           fabs(m_sigs[id1]->avgl[1] - m_sigs[id2]->avgl[1]) +
           fabs(m_sigs[id1]->avgl[2] - m_sigs[id2]->avgl[2]);
}

/** use it to tell the content-based difference between two images
*/
double HaarIface::calcDiff(long int id1, long int id2)
{
    double diff        = calcAvglDiff(id1, id2);
    Haar::Idx *sig1[3] = {m_sigs[id1]->sig1, m_sigs[id1]->sig2, m_sigs[id1]->sig3};
    Haar::Idx *sig2[3] = {m_sigs[id2]->sig1, m_sigs[id2]->sig2, m_sigs[id2]->sig3};

    for (int b = 0; b < NUM_COEFS; b++)
    {
        for (int c = 0; c < 3; c++)
        {
            for (int b2 = 0; b2 < NUM_COEFS; b2++)
            {
                if (sig2[c][b2] == sig1[c][b])
                {
                    diff -= s_haar_weights[0][m_imgBin[abs(sig1[c][b])]][c];
                }
            }
        }
    }

  return diff;
}

// ----------------------------------------------------------------------------
// TODO: Marcel, these methods can be removed.

void HaarIface::initDbase()
{
    // should be called before adding images
    DDebug() << "Image database initialized." << endl;
    initImgBin();
}

void HaarIface::closeDbase()
{
    // should be called before exiting app
    free_sigs();
    DDebug() << "Image database closed." << endl;
}

int HaarIface::loaddb(char* filename)
{
    std::ifstream f(filename, ios::binary);
    if (!f.is_open()) return 0;

    int      sz;
    long int id;

    // read buckets
    for (int c = 0; c < 3; c++)
    {
        for (int pn = 0; pn < 2; pn++)
        {
            for (int i = 0; i < 16384; i++)
            {
                f.read((char*)&(sz), sizeof(int));
                for (int k = 0; k < sz; k++)
                {
                    f.read((char*)&(id), sizeof(long int));
                    m_imgbuckets[c][pn][i].push_back(id);
                }
            }
        }
    }

    // read sigs
    f.read((char*)&(sz), sizeof(int));
    for (int k = 0; k < sz; k++)
    {
        f.read((char*)&(id), sizeof(long int));
        m_sigs[id] = new sigStruct();
        f.read((char*)m_sigs[id], sizeof(sigStruct));
    }
    f.close();
    return 1;
}

int HaarIface::savedb(char* filename)
{
    /*
    Serialization order:
    for each color {0,1,2}:
        for {positive,negative}:
            for each 128x128 coefficient {0-16384}:
                [int] bucket size (size of list of ids)
                for each id:
                    [long int] image id
    [int] number of images (signatures)
    for each image:
        [long id] image id
        for each sig coef {0-39}:  (the NUM_COEFS greatest coefs)
            for each color {0,1,2}:
                [int] coef index (signed)
        for each color {0,1,2}:
            [double] average luminance
        [int] image width
        [int] image height

    */
    std::ofstream f(filename, ios::binary);
    if (!f.is_open()) return 0;

    int      sz;
    long int id;

    // save buckets
    for (int c = 0; c < 3; c++)
    {
        for (int pn = 0; pn < 2; pn++)
        {
            for (int i = 0; i < 16384; i++)
            {
                sz = m_imgbuckets[c][pn][i].size();
                f.write((char*)&(sz), sizeof(int) );
                long_listIterator end = m_imgbuckets[c][pn][i].end();
                for (long_listIterator it = m_imgbuckets[c][pn][i].begin(); it != end; it++)
                {
                    f.write((char*)&((*it)), sizeof(long int));
                }
            }
        }
    }

    // save sigs
    sz = m_sigs.size();
    f.write((char*)&(sz), sizeof(int));
    for (sigIterator it = m_sigs.begin(); it != m_sigs.end(); it++)
    {
        id = (*it).first;
        f.write((char*)&(id), sizeof(long int));
        f.write((char *)(it->second), sizeof(sigStruct));
    }
    f.close();
    return 1;
}

/** call it to reset all buckets and signatures
*/
int HaarIface::resetdb()
{
    for (int c = 0; c < 3; c++)
    {
        for (int pn = 0; pn < 2; pn++)
        {
            for (int i = 0; i < 16384; i++)
            {
                m_imgbuckets[c][pn][i].clear();
            }
        }
    }

    // delete sigs
    free_sigs();
    m_sigs.clear();
    return 1;
}

int HaarIface::magickThumb(char* f1, char* f2)
{
    QImage image;

    if (isJpegImage(f1))
    {
        if (!loadJPEGScaled(image, f1, 128))
        {
            // error loading image
            // try QT now.
            if (!image.load(f1))
            {
                return 0;
            }
        }
        else
        {
            // use default QT image loading
            if (!image.load(f1))
            {
                return 0;
            }
        }
    }

    image.scaled(128, 128, Qt::KeepAspectRatio).save(f2, "PNG");
    return 1;
}

/** Python Wrappers/Helpers
*/
int HaarIface::getLongListSize(long_list& li)
{
    return li.size();
}

long int HaarIface::popLongList(long_list& li)
{
    long int a = li.front();
    li.pop_front();
    return a;
}

int HaarIface::getLongList2Size(long_list_2& li)
{
    return li.size();
}

HaarIface::long_list HaarIface::popLong2List(long_list_2& li)
{
    long_list a = li.front();
    li.pop_front();
    return a;
}

}  // namespace Digikam
