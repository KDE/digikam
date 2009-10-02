/* ============================================================
 *
 * This file is a part of markerclusterholder, developed
 * for digikam and trippy
 *
 * Date        : 2009-09-03
 * Description : clustering of markers support for worldmapwidget
 *
 * Copyright (C) 2009 by Michael G. Hansen <mike at mghansen dot de>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef MARKERCLUSTERHOLDER_H
#define MARKERCLUSTERHOLDER_H

// standard library includes
#include <memory>

// Marble includes
#include <marble/MarbleWidget.h>
#include <marble/GeoDataPoint.h>

class MarkerClusterHolderPrivate;

class MarkerClusterHolder : public QObject
{
    Q_OBJECT
  
public:
    
    typedef QList<int> QIntList;
    
    /**
     * @brief Information about a marker
     */
    class MarkerInfo
    {
    private:
        //! latitude of this marker
        qreal m_lat;
        //! longitude of this marker
        qreal m_lon;
        //! custom data stored in the marker
        QVariant m_data;
        //! is the marker selected
        bool m_selected;
        //! is the marker 'solo'
        bool m_solo;
        
    public:
        /**
         * @brief Constructs a MarkerInfo with given latitude and longitude
         * @param lon Longitude of marker in degrees
         * @param lat Latitude of marker in degrees
         */
        MarkerInfo(const qreal lon, const qreal lat)
        : m_lat(lat), m_lon(lon), m_data(), m_selected(false), m_solo(false)
        {
        }
        
        /**
         * @brief Constructs a MarkerInfo with given latitude and longitude
         * @param lon Longitude of marker in degrees
         * @param lat Latitude of marker in degrees
         * @param yourdata QVariant holding user data associated with this marker
         */
        MarkerInfo(const qreal lon, const qreal lat, const QVariant& yourdata )
        : m_lat(lat), m_lon(lon), m_data(yourdata), m_selected(false), m_solo(false)
        {
        }
        
        MarkerInfo()
        : m_lat(0), m_lon(0), m_data(), m_selected(false), m_solo(false)
        {
        }
        
        /**
         * @brief Constructs a marker from the applications user data
         *
         * This function has to be implemented by the client application.
         * The latitude and longitude for the marker have to be extracted
         * from the user data and then a marker can be created.
         *
         * @param yourdata User data from the application
         * @return A marker created from the user data
         */
        template<class yourtype> static MarkerInfo fromData(const yourtype& yourdata);
        
        /**
         * @brief Returns the data associated with this marker
         *
         * This function has to be implemented by the client application
         *
         * @return The user data which was stored in @m_data
         */
        template<class yourtype> yourtype data() const
        {
            return m_data.value<yourtype>();
        }
        
        /**
         * @brief Returns the longitude of this marker
         * @return Longitude of this marker in degrees
         */
        qreal lon() const
        {
            return m_lon;
        }
        
        /**
         * @brief Returns the latitude of this marker
         * @return Latitude of this marker in degrees
         */
        qreal lat() const
        {
            return m_lat;
        }
        
        /**
         * @brief Sets the latitude of this marker
         * @param lat Latitude in degrees
         */
        void setLat(const qreal lat)
        {
            m_lat = lat;
        }
        
        /**
         * @brief Sets the longitude of this marker
         * @param lon Longitude in degrees
         */
        void setLon(const qreal lon)
        {
            m_lon = lon;
        }
        
        /**
         * @brief Sets the selected state of this marker
         * @param newState Whether this marker should be selected
         */
        void setSelected(const bool newState)
        {
            m_selected = newState;
        }
        
        /**
         * @brief Returns the selected state of this marker
         * @return Whether this marker is selected
         */
        bool isSelected() const
        {
            return m_selected;
        }
        
        /**
         * @brief Sets the solo state of this marker
         * @param newState Whether the marker should be solo
         */
        void setSolo(const bool newState)
        {
            m_solo = newState;
        }
        
        /**
         * @brief Returns the solo state of this marker
         * @return Whether this marker is solo
         */
        bool isSolo() const
        {
            return m_solo;
        }
        
        typedef QList<MarkerInfo> List;
        
        friend class MarkerClusterHolder;
    };
    
    typedef QList<MarkerInfo> MarkerInfoList;
    
    /**
     * @brief Information about a cluster
     */
    class ClusterInfo
    {
      public:
        //! latitude of the center of this cluster
        qreal lat;
        //! longitude of the center of this cluster
        qreal lon;
        //! whether the center has been set yet
        bool centerValid;
        //! position of the cluster on the screen
        QPoint pixelPos;
        //! indices of the markers in this cluster
        QIntList markerIndices;
        //! maximum size on the map
        QSize maxSize;
        //! last size on the map (needed for mouse interaction)
        QSize lastSize;
        //! userdata stored in the cluster
        QVariant userData;
        
        //! Description for a property which can apply for a part of the markers of this cluster
        enum PartialState
        {
          //! Property does not apply for any markers in this cluster
          PartialNone = 0,
          //! Property applies for some of the markers in this cluster
          PartialSome = 1,
          //! Property applies for all markers in the cluster
          PartialAll = 2
        };

        //! Selection state of this cluster
        PartialState selected;
        //! Solo state of this cluter
        PartialState solo;
        
        ClusterInfo()
        : lat(), lon(), centerValid(false), pixelPos(), markerIndices(), maxSize(), lastSize(), userData(), selected(PartialNone), solo(PartialNone)
        {
        }

        /**
         * @brief Returns the number of markers in this cluster
         * @return The number of markers
         */
        int markerCount() const
        {
            return markerIndices.count();
        }
        
        /**
         * @brief Adds a marker to this cluster
         * @param markerIndex Index of the marker to be added
         */
        void addMarkerIndex(const int markerIndex)
        {
            markerIndices << markerIndex;
        }
        
        /**
         * @brief Adds markers to this cluster
         * @param markerIndexList List of indices of the markers to be added
         */
        void addMarkerIndices(const QIntList& markerIndexList)
        {
            markerIndices << markerIndexList;
        }
        
        void getColorInfos(const bool haveAnySolo, QColor *fillColor, QColor *strokeColor, Qt::PenStyle *strokeStyle, QString *labelText, QColor *labelColor) const;
        
        QString getLabelText() const;
        
        /**
         * @brief Sets the center of the cluster
         * @param newLat Latitude of the center of the cluster
         * @param newLon Longitude of the center of the cluster
         */
        void setCenter(const qreal newLat, const qreal newLon)
        {
            lat=newLat;
            lon=newLon;
            centerValid=true;
        }
        
        /**
         * @brief Sets the center of the cluster
         * @param marker Marker whose coordinates are to be used as the new center
         */
        void setCenter(const MarkerInfo& marker)
        {
            lat = marker.lat();
            lon = marker.lon();
            centerValid = true;
        }
        
        typedef QList<ClusterInfo> List;
    };
    
    typedef QList<ClusterInfo> ClusterInfoList;
    
    /**
     * @brief Comparison function for the user data of markers
     *
     * This function has to be implemented by the client application. It is
     * only needed if the user data stored in the markers is a user defined type.
     *
     * @param yourdata User data for the compare function
     * @return true if the data in the two QVariants is equal
     */
    typedef bool (*MarkerDataEqualFunction)(const QVariant& one, const QVariant& two, void* const yourdata);
    
    /**
     * @brief Returns the tooltip text for a cluster
     *
     * This function has to be implemented by the client application. It is
     * only needed if tooltips are to be displayed for markers.
     * The application can decide whether a tooltip is useful depending on the
     * number of markers in the cluster. If no tooltip should be shown, it should return
     * an empty string.
     *
     * @param clusterIndex Index of the cluster whose tooltip is requested
     * @param mch Pointer to the MarkerClusterHolder requesting the tooltip
     * @param yourdata User data for the tooltip function
     * @return The text for the tooltip
     */
    typedef QString (*TooltipFunction)(const int clusterIndex, MarkerClusterHolder* const mch, void* const yourdata);
    
    enum PixmapOperations {
        PixmapInvalid = 0,
        PixmapValid = 1,
        PixmapNoAddNumber = 2,
        PixmapNoSoloModify = 4,
        PixmapNoSelectedModify = 8
    };
    
    /**
     * @brief Creates the pixmap for a cluster
     *
     * This function has to be implemented by the client application. It is
     * only needed if pixmaps are to be displayed instead of circles.
     *
     * @param clusterIndex Index of the cluster whose pixmap is requested
     * @param mch Pointer to the MarkerClusterHolder requesting the pixmap
     * @param maxSize Maximum size of the pixmap
     * @param yourdata User data for the pixmap function
     * @param clusterPixmap Pointer to a QPixmap where the pixmap is to be stored
     * @return PixmapValid if a pixmap was returned
     */
    typedef PixmapOperations (*ClusterPixmapFunction)(const int clusterIndex, MarkerClusterHolder* const mch, const QSize& maxSize, void* const yourdata, QPixmap* const clusterPixmap);
    
    /**
     * @brief Custom painting before/after clusters
     */
    typedef void (*CustomPaintFunction)(Marble::GeoPainter* const geoPainter, const bool isBefore, void* const yourdata);
    
public:
    MarkerClusterHolder(Marble::MarbleWidget* const marbleWidget);
    ~MarkerClusterHolder();
    void addMarker(const MarkerInfo& marker);
    void addMarkers(const QList<MarkerInfo>& markerList);
    void removeMarkers(const QList<MarkerInfo>& markerList);
    void removeMarkers(const QIntList& markerIndices);
    void removeMarkers(const int start, const int end);
    void paintOnMarble(Marble::GeoPainter* const painter);
    void clear();
    void reorderClusters();
    bool autoRedrawOnMarkerAdd() const;
    MarkerInfo::List selectedMarkers() const;
    MarkerInfo::List soloMarkers() const;
    MarkerInfo::List indicesToMarkers(const QIntList indicesList) const;
    void setMarkerDataEqualFunction(const MarkerDataEqualFunction compareFunction, void* const yourdata);
    void setClusterPixmapFunction(const ClusterPixmapFunction clusterPixmapFunction, void* const yourdata);
    int findClusterAt(const QPoint pos) const;
    void setCustomPaintFunction(const CustomPaintFunction customPaintFunction, void* const yourdata);
    ClusterInfo& cluster(const int clusterIndex);
    MarkerInfo& marker(const int markerIndex);
    
protected:
     bool eventFilter(QObject *obj, QEvent *event);
     bool markersEqual(const MarkerInfo& one, const MarkerInfo& two);
     
private:
    std::auto_ptr<MarkerClusterHolderPrivate> d;
    void reorderClustersPixelGrid();
    void redrawIfNecessary(const bool force = false);
    void updateClusterStates();
    void paintOnMarbleInternal(Marble::GeoPainter* const painter);
    static void ExternalDrawCallback(Marble::GeoPainter *painter, void* yourdata);
    void computeClusterDistances();
  
signals:
    void signalSelectionChanged();
    void signalSoloChanged();
    
public slots:
    void setAutoRedrowOnMarkerAdd(const bool doRedraw);
    void clearSelection();
    void clearFiltering();
    void setSoloMarkers(const QIntList &markerIndicesList, const bool setAsSolo = true, const bool resetOthers = true);
    void setSoloMarkers(const MarkerClusterHolder::MarkerInfoList &markerList, const bool setAsSolo = true, const bool resetOthers = true);
    void setSelectedMarkers(const QIntList &markerIndicesList, const bool setAsSelected = true, const bool resetOthers = true);
    void setSelectedMarkers(const MarkerClusterHolder::MarkerInfoList &markerList, const bool setAsSelected = true, const
    bool resetOthers = true);
    void setAllowFiltering(const bool allow);
    void setAllowSelection(const bool allow);
    void setTooltipFunction(TooltipFunction newTooltipFunction, void* const yourdata);
    void zoomIntoCluster(ClusterInfo cluster);
    
private:
    Q_DISABLE_COPY(MarkerClusterHolder)
};

Q_DECLARE_METATYPE(MarkerClusterHolder::MarkerInfo)
Q_DECLARE_METATYPE(MarkerClusterHolder::MarkerInfoList)
Q_DECLARE_METATYPE(MarkerClusterHolder::ClusterInfo)
Q_DECLARE_METATYPE(MarkerClusterHolder::ClusterInfoList)

namespace Marble
{
    class GeoPainter;
}

inline MarkerClusterHolder::PixmapOperations& operator|=(MarkerClusterHolder::PixmapOperations& target, const MarkerClusterHolder::PixmapOperations& source)
{
    target = MarkerClusterHolder::PixmapOperations(target | source);
    return target;
}

/**
 * @brief Simple sub-class of Marble::MarbleWidget for custom painting
 *
 * Use this class instead of Marble::MarbleWidget if you do not want
 * to subclass it yourself
 */
class MarbleSubClassWidget : public Marble::MarbleWidget
{
    Q_OBJECT
    
public:
    MarkerClusterHolder* const m_markerClusterHolder;
    
    MarbleSubClassWidget(QWidget* parent)
    : MarbleWidget(parent), m_markerClusterHolder(new MarkerClusterHolder(this))
    {
    }
  
protected:
    virtual void customPaint(Marble::GeoPainter* painter);

private:
    Q_DISABLE_COPY(MarbleSubClassWidget)
};

#endif // MARKERCLUSTERHOLDER_H

