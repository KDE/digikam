/* ============================================================
 *
 * This file is a part of digiKam
 *
 * Date        : 02-02-2012
 * Description : Face database interface to train identities.
 *
 * Copyright (C) 2012-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2010-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef FACE_DATABASE_TRAINING_INTERFACE_H
#define FACE_DATABASE_TRAINING_INTERFACE_H

// Qt includes

#include <QString>
#include <QFile>
#include <QDataStream>
#include <QStandardPaths>

//c++ includes
#include <vector>

// Local includes

#include "identity.h"
#include "facedbbackend.h"
#include "opencvmatdata.h"
#include "dnn_face.h"
#include "shapepredictor.h"
#include "fullobjectdetection.h"

namespace Digikam
{

class LBPHFaceModel;
class EigenFaceModel;
class FisherFaceModel;
class DNNFaceModel;

#ifndef DNN_NETWORK
#define DNN_NETWORK
template <template <int,template<typename>class,int,typename> class block, int N, template<typename>class BN, typename SUBNET>
using residual = add_prev1<block<N,BN,1,tag1<SUBNET>>>;

template <template <int,template<typename>class,int,typename> class block, int N, template<typename>class BN, typename SUBNET>
using residual_down = add_prev2<avg_pool<2,2,2,2,skip1<tag2<block<N,BN,2,tag1<SUBNET>>>>>>;

template <int N, template <typename> class BN, int stride, typename SUBNET>
using block  = BN<con<N,3,3,1,1,relu<BN<con<N,3,3,stride,stride,SUBNET>>>>>;

template <int N, typename SUBNET> using ares      = relu<residual<block,N,affine,SUBNET>>;
template <int N, typename SUBNET> using ares_down = relu<residual_down<block,N,affine,SUBNET>>;
template <typename SUBNET> using alevel0 = ares_down<256,SUBNET>;
template <typename SUBNET> using alevel1 = ares<256,ares<256,ares_down<256,SUBNET>>>;
template <typename SUBNET> using alevel2 = ares<128,ares<128,ares_down<128,SUBNET>>>;
template <typename SUBNET> using alevel3 = ares<64,ares<64,ares<64,ares_down<64,SUBNET>>>>;
template <typename SUBNET> using alevel4 = ares<32,ares<32,ares<32,SUBNET>>>;

using anet_type = loss_metric<fc_no_bias<128,avg_pool_everything<
                                alevel0<
                                alevel1<
                                alevel2<
                                alevel3<
                                alevel4<
                                max_pool<3,3,2,2,relu<affine<con<32,7,7,2,2,
                                input_rgb_image_sized<150>
                                >>>>>>>>>>>>;
#endif
                                
class FaceDb
{
public:

    FaceDb(FaceDbBackend* const db);
    ~FaceDb();

    BdEngineBackend::QueryState setSetting(const QString& keyword, const QString& value);
    QString setting(const QString& keyword) const;

    int  addIdentity() const;
    void updateIdentity(const Identity& p);
    void deleteIdentity(int id);
    void deleteIdentity(const QString & uuid);
    QList<Identity> identities()  const;
    QList<int>      identityIds() const;

    /// OpenCV LBPH

    void updateLBPHFaceModel(LBPHFaceModel& model);
    LBPHFaceModel lbphFaceModel() const;
    void clearLBPHTraining(const QString& context = QString());
    void clearLBPHTraining(const QList<int>& identities, const QString& context = QString());

    /// OpenCV EIGEN

    void getFaceVector(OpenCVMatData data, std::vector<float>& vecdata);
    void updateEIGENFaceModel(EigenFaceModel& model);
    EigenFaceModel eigenFaceModel() const;
    void clearEIGENTraining(const QString& context = QString());
    void clearEIGENTraining(const QList<int>& identities, const QString& context = QString());

    /// OpenCV FISHER
    void updateFISHERFaceModel(FisherFaceModel& model);
    FisherFaceModel fisherFaceModel() const;

    /// DNN
    DNNFaceModel dnnFaceModel();// const;

    // ----------- Database shrinking methods ----------

    /**
     * Returns true if the integrity of the database is preserved.
     */
    bool integrityCheck();

    /**
     * Shrinks the database.
     */
    void vacuum();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // FACE_DATABASE_TRAINING_INTERFACE_H
