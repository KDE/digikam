/* ============================================================
 *
 * Date        : 2010-02-05
 * Description : JavaScript part of the GoogleMaps-backend for WorldMapWidget2
 *
 * Copyright (C) 2010, 2011, 2014 by Michael G. Hansen <mike at mghansen dot de>
 * Copyright (C) 2014 by Justus Schwartz <justus at gmx dot li>

 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

var mapDiv;
var map;
var eventBuffer = new Array();
var markerList = new Object();
var clusterList = new Object();
var clusterDataList = new Object();
var trackList = new Array();
var isInEditMode = false;
var dragMarker;
var dragSnappingToMid = -1;
var dragSnappingToId = -1;
var selectionRectangle;
var temporarySelectionRectangle;
var markers = new Array();
var markerCount = 0;

// ProjectionHelp: http://taapps-javalibs.blogspot.com/2009/10/google-map-v3how-to-use-overlayviews.html
function ProjectionHelper(overlayMap)
{
    google.maps.OverlayView.call(this);
    this.setMap(overlayMap);
}
ProjectionHelper.prototype = new google.maps.OverlayView();
ProjectionHelper.prototype.draw = function() { }
var projectionHelper = null;

function kgeomapPostEventString(eventString)
{
    eventBuffer.push(eventString);
    window.status = '(event)';
}

function kgeomapReadEventStrings()
{
    var eventBufferString = eventBuffer.join('|');
    eventBuffer = new Array();
    // let the application know that there are no more events waiting:
    window.status = '()';
    return eventBufferString;
}

function kgeomapDebugOut(someString)
{
    kgeomapPostEventString('do'+someString);
}

function kgeomapSetZoom(zoomvalue)
{
    map.setZoom(zoomvalue);
}

function kgeomapGetZoom()
{
    return map.getZoom();
}

function kgeomapGetMaxZoom()
{
    return map.mapTypes[map.getMapTypeId()].maxZoom;
}

function kgeomapGetMinZoom()
{
    var minZoom = map.mapTypes[map.getMapTypeId()].minZoom;
    if (minZoom==null)
    {
        minZoom = 1;
    }
    return minZoom;
}

function kgeomapZoomIn()
{
    map.setZoom(map.getZoom()+1);
}

function kgeomapZoomOut()
{
    map.setZoom(map.getZoom()-1);
}

function kgeomapSetCenter(lat, lon)
{
    var latlng = new google.maps.LatLng(lat, lon);
    map.setCenter(latlng);
}

function kgeomapGetCenter()
{
    var latlngString = map.getCenter().toUrlValue(12);
    return latlngString;
}

function kgeomapGetBounds()
{
    return map.getBounds().toString();
}

function kgeomapSetIsInEditMode(state)
{
    isInEditMode = state;
}

function kgeomapLatLngToPoint(latLng)
{
    //      There is an offset in fromLatLngToDivPixel once the map has been panned
    var myPoint = projectionHelper.getProjection().fromLatLngToDivPixel(latLng);
    var centerPoint = projectionHelper.getProjection().fromLatLngToDivPixel(map.getCenter());
    var centerOffsetX = Math.floor(mapDiv.offsetWidth / 2);
    var centerOffsetY = Math.floor(mapDiv.offsetHeight / 2);
    var pointX = myPoint.x-centerPoint.x+centerOffsetX;
    var pointY = myPoint.y-centerPoint.y+centerOffsetY;
    return new google.maps.Point(pointX, pointY);
}

function kgeomapLatLngToPixel(lat, lon)
{
    //      There is an offset in fromLatLngToDivPixel once the map has been panned
    var latlng = new google.maps.LatLng(lat, lon);
    var myPoint = projectionHelper.getProjection().fromLatLngToDivPixel(latlng);
    var centerPoint = projectionHelper.getProjection().fromLatLngToDivPixel(map.getCenter());
    var centerOffsetX = Math.floor(mapDiv.offsetWidth / 2);
    var centerOffsetY = Math.floor(mapDiv.offsetHeight / 2);
    var pointX = myPoint.x-centerPoint.x+centerOffsetX;
    var pointY = myPoint.y-centerPoint.y+centerOffsetY;
    return new google.maps.Point(pointX, pointY).toString();
//         return projectionHelper.getProjection().fromLatLngToDivPixel(latlng).toString();
}

function kgeomapPixelToLatLngObject(x, y)
{
    //      There is an offset in fromDivPixelToLatLng once the map has been panned
    var centerPoint = projectionHelper.getProjection().fromLatLngToDivPixel(map.getCenter());
    var centerOffsetX = mapDiv.offsetWidth / 2;
    var centerOffsetY = mapDiv.offsetHeight / 2;
    var pointX = x+centerPoint.x-centerOffsetX;
    var pointY = y+centerPoint.y-centerOffsetY;
    var point = new google.maps.Point(pointX, pointY);
    return projectionHelper.getProjection().fromDivPixelToLatLng(point);
}

function kgeomapPixelToLatLng(x, y)
{
    return kgeomapPixelToLatLngObject(x, y).toUrlValue(12);
}

// parameter: "SATELLITE"/"ROADMAP"/"HYBRID"/"TERRAIN"
function kgeomapSetMapType(newMapType)
{
    if (newMapType == "SATELLITE") { map.setMapTypeId(google.maps.MapTypeId.SATELLITE); }
    if (newMapType == "ROADMAP")   { map.setMapTypeId(google.maps.MapTypeId.ROADMAP); }
    if (newMapType == "HYBRID")    { map.setMapTypeId(google.maps.MapTypeId.HYBRID); }
    if (newMapType == "TERRAIN")   { map.setMapTypeId(google.maps.MapTypeId.TERRAIN); }
}

function kgeomapGetMapType()
{
    var myMapType = map.getMapTypeId();
    if (myMapType == google.maps.MapTypeId.SATELLITE) { return "SATELLITE"; }
    if (myMapType == google.maps.MapTypeId.ROADMAP )  { return "ROADMAP"; }
    if (myMapType == google.maps.MapTypeId.HYBRID )   { return "HYBRID"; }
    if (myMapType == google.maps.MapTypeId.TERRAIN )  { return "TERRAIN"; }
    return "";
}

function kgeomapSetShowMapTypeControl(state)
{
    var myOptions = {
            mapTypeControl: state
        }
    map.setOptions(myOptions);
}

function kgeomapSetShowNavigationControl(state)
{
    var myOptions = {
            navigationControl: state
        }
    map.setOptions(myOptions);
}

function kgeomapSetShowScaleControl(state)
{
    var myOptions = {
            scaleControl: state
        }
    map.setOptions(myOptions);
}

function kgeomapClearMarkers(mid)
{
    for (var i in markerList[mid])
    {
        markerList[mid][i].marker.setMap(null);
    }
    markerList[mid] = new Object();
}

function kgeomapClearTracks() 
{
    for (var i in trackList) {
        trackList[i].track.setMap(null);
    }
    trackList = new Array();

    return true;
}

function kgeomapGetTrackIndex(tid)
{
    for (var i=0; i<trackList.length; ++i)
    {
        if (trackList[i].id==tid)
        {
            return i;
        }
    }

    return -1;
}

function kgeomapRemoveTrack(tid)
{
    var idx = kgeomapGetTrackIndex(tid);
    if (idx<0)
    {
        return;
    }

    trackList[idx].track.setMap(null);
    trackList.splice(idx, 1);
}

function kgeomapCreateTrack(tid, trackColor)
{
    // trackColor has to have the form '#FF0000'
    var oldIndex = kgeomapGetTrackIndex(tid);
    if (oldIndex>=0)
    {
        trackList[oldIndex].track.setMap(null);
        trackList.splice(oldIndex, 1);
    }

    var trackEntry = new Object();
    trackEntry.id = tid;
    trackEntry.track = new google.maps.Polyline({
            geodesic: true,
            strokeColor: trackColor,
            strokeOpacity: 1.0,
            strokeWeight: 2
        });

    trackList.push(trackEntry);

    return trackList.length-1;
}

function kgeomapAddToTrack(tid, coordString)
{
    var trackIndex = kgeomapGetTrackIndex(tid);
    if (trackIndex<0)
    {
        return false;
    }

    var track = trackList[trackIndex].track;
    /// @TODO Does setting and unsetting the map take long? Maybe it is better
    ///       to create, add points, then add map instead.
//    track.setMap(null); // See bug #342427

    var trackCoordinates = track.getPath();
//     for (var i = 0; i < coordString.length; i+=2)
//     {
//         var cLat = coordString[i];
//         var cLon = coordString[i+1];
//         trackCoordinates.push(new google.maps.LatLng(cLat,cLon));
//     }
    var coordArray = JSON.parse(coordString);
    for (var i = 0; i < coordArray.length; ++i)
    {
        var coord = coordArray[i];
        trackCoordinates.push(new google.maps.LatLng(coord.lat,coord.lon));
    }
    track.setPath(trackCoordinates);
    track.setMap(map);
    trackList[trackIndex].track = track;

    return true;
}

function kgeomapSetMarkerPixmap(mid, id, pixmapWidth, pixmapHeight, xOffset, yOffset, pixmapurl)
{
    var pixmapSize = new google.maps.Size(pixmapWidth, pixmapHeight);
    var pixmapOrigin = new google.maps.Point(0, 0);
    var anchorPoint = new google.maps.Point(xOffset, yOffset);
    var markerImage = new google.maps.MarkerImage(pixmapurl, pixmapSize, pixmapOrigin, anchorPoint);
    markerList[mid][id].marker.setIcon(markerImage);
}

function kgeomapAddMarker(mid, id, lat, lon, setDraggable, setSnaps)
{
    var latlng = new google.maps.LatLng(lat, lon);
    var marker = new google.maps.Marker({
            position: latlng,
            map: map,
            draggable: setDraggable,
            icon: new google.maps.MarkerImage('marker-green.png', new google.maps.Size(20, 32)),
            zIndex: 10
        });

    google.maps.event.addListener(marker, 'dragend', function()
        {
            kgeomapPostEventString('mm'+id.toString());
        });
    if (!markerList[mid])
    {
        markerList[mid] = new Object();
    }
    markerList[mid][id] = {
            marker: marker,
            snaps: setSnaps
        };
}

function kgeomapGetMarkerPosition(mid,id)
{
    var latlngString;
    if (markerList[mid.toString()][id.toString()])
    {
        latlngString = markerList[mid.toString()][id.toString()].marker.getPosition().toUrlValue(12);
    }
    return latlngString;
}

function kgeomapClearClusters()
{
    for (var i in clusterList)
    {
        clusterList[i].setMap(null);
    }
    clusterList = new Object();
    clusterDataList = new Object();
}

function kgeomapGetPixmapName(markerCount, markerSelectedCount)
{
    var colorCode;
    if (markerCount>=100)
    {
        colorCode="ff0000";
    }
    else if (markerCount>=50)
    {
        colorCode="ff7f00";
    }
    else if (markerCount>=10)
    {
        colorCode="ffff00";
    }
    else if (markerCount>=2)
    {
        colorCode="00ff00";
    }
    else
    {
        colorCode="00ffff";
    }
    if (markerSelectedCount==markerCount)
    {
        colorCode+="-selected";
    }
    else if (markerSelectedCount>0)
    {
        colorCode+="-someselected";
    }
    return colorCode;
}

function kgeomapSetClusterPixmap(id, pixmapWidth, pixmapHeight, xOffset, yOffset, pixmapurl)
{
    var pixmapSize = new google.maps.Size(pixmapWidth, pixmapHeight);
    var pixmapOrigin = new google.maps.Point(0, 0);
    var anchorPoint = new google.maps.Point(xOffset, yOffset);
    var markerImage = new google.maps.MarkerImage(pixmapurl, pixmapSize, pixmapOrigin, anchorPoint);
    clusterList[id].setIcon(markerImage);
}

function kgeomapAddCluster(id, lat, lon, setDraggable, markerCount, markerSelectedCount)
{
    var latlng = new google.maps.LatLng(lat, lon);
    var clusterIcon;
    var colorCode = kgeomapGetPixmapName(markerCount, markerSelectedCount);
    if (isInEditMode)
    {
        clusterIcon = new google.maps.MarkerImage('marker-'+colorCode+'.png', new google.maps.Size(20, 32));
    } else
    {
        clusterIcon = new google.maps.MarkerImage('cluster-circle-'+colorCode+'.png', new google.maps.Size(30, 30), new google.maps.Point(0,0), new google.maps.Point(15, 15));
    }
    var marker = new google.maps.Marker({
            position: latlng,
            map: map,
            draggable: isInEditMode,
            icon: clusterIcon,
            zIndex: 10
        });

/*        for (var mid in markerList) {
            for (var id in markerList[mid]) {
                markerList[mid][id].marker.setClickable(false);
            }
        }
*/

    google.maps.event.addListener(marker, 'dragstart', function()
        {
            var movingClusterData = clusterDataList[id];
            if (movingClusterData.MarkerSelectedCount==0)
            {
                // no need to change the cluster in any way
                return;
            }
            // at least some items in the cluster are selected. we have to scan all clusters and
            // take their selected markers:
            var newSelectedCount = 0;
            for (var i in clusterList)
            {
                var clusterData = clusterDataList[i];
                if (clusterData.MarkerSelectedCount>0)
                {
                    newSelectedCount+=clusterData.MarkerSelectedCount;
                    var newMarkerCount = clusterData.MarkerCount-clusterData.MarkerSelectedCount;

                    if (i!=id)
                    {
                        var colorCode = kgeomapGetPixmapName(newMarkerCount, 0);
                        var clusterIcon = new google.maps.MarkerImage('marker-'+colorCode+'.png', new google.maps.Size(20, 32));
                        clusterList[i].setOptions({ icon: clusterIcon, title: newMarkerCount.toString() });
                    }
                }
            }
            // adjust the moving marker
            var colorCode = kgeomapGetPixmapName(newSelectedCount, newSelectedCount);
            var clusterIcon = new google.maps.MarkerImage('marker-'+colorCode+'.png', new google.maps.Size(20, 32));
            clusterList[id].setOptions({ icon: clusterIcon, title: newSelectedCount.toString()});

            // create a leftover-marker:
            var leftOverMarkerCount=movingClusterData.MarkerCount-movingClusterData.MarkerSelectedCount;
            if (leftOverMarkerCount>0)
            {
                var colorCode = kgeomapGetPixmapName(leftOverMarkerCount, 0);
                var clusterIcon = new google.maps.MarkerImage('marker-'+colorCode+'.png', new google.maps.Size(20, 32));
                var leftOverMarker = new google.maps.Marker({
                        position: latlng,
                        map: map,
                        icon: clusterIcon,
                        title: leftOverMarkerCount.toString(),
                        zIndex: 10
                    });
                clusterList[-1]=leftOverMarker;
            }
        });
    google.maps.event.addListener(marker, 'drag', function(e)
        {
            // get the pixel position:
            var clusterPoint = kgeomapLatLngToPoint(e.latLng);
            // now iterate through all markers to which we can snap
            var minDistSquared=-1;
            var minMid;
            var minId;
            kgeomapDebugOut('drag');
            for (var mid in markerList)
            {
                for (var id in markerList[mid])
                {
                    if (!markerList[mid][id].snaps)
                    {
                        continue;
                    }

                    var markerPoint = kgeomapLatLngToPoint(markerList[mid][id].marker.getPosition());
                    var distanceSquared = (clusterPoint.x-markerPoint.x)*(clusterPoint.x-markerPoint.x) + (clusterPoint.y-markerPoint.y)*(clusterPoint.y-markerPoint.y);
                    if ((distanceSquared<=100)&&((minDistSquared<0)||(distanceSquared<minDistSquared)))
                    {
                        minDistSquared = distanceSquared;
                        minMid = mid;
                        minId = id;
                    }
                }
            }
            if (minDistSquared>=0)
            {
                // TODO: emit proper snap signal
                marker.setPosition(markerList[minMid][minId].marker.getPosition());
                dragSnappingToId = minId;
                dragSnappingToMid = minMid;
            }
            else
            {
                dragSnappingToId = -1;
                dragSnappingToMid = -1;
            }
        });
    google.maps.event.addListener(marker, 'dragend', function()
        {
            if (dragSnappingToMid>=0)
            {
                kgeomapPostEventString('cs'+id.toString()+'/'+dragSnappingToMid.toString()+'/'+dragSnappingToId.toString());
            }
            else
            {
                kgeomapPostEventString('cm'+id.toString());
            }
        });
    google.maps.event.addListener(marker, 'click', function()
        {
            kgeomapPostEventString('cc'+id.toString());
        });
    
    clusterList[id] = marker;
    var clusterData = new Object();
    clusterData["MarkerCount"]=markerCount;
    clusterData["MarkerSelectedCount"]=markerSelectedCount;
    clusterDataList[id]=clusterData;
}

function kgeomapGetClusterPosition(id)
{
    var latlngString;
    if (clusterList[id.toString()])
    {
        latlngString = clusterList[id.toString()].getPosition().toUrlValue(12);
    }
    return latlngString;
}

function kgeomapWidgetResized(newWidth, newHeight)
{
    document.getElementById('map_canvas').style.height=newHeight.toString()+'px';
}

function kgeomapRemoveDragMarker()
{
    if (dragMarker)
    {
        dragMarker.setMap(null);
        dragMarker = null;
    }
}

function kgeomapMoveDragMarker(x, y)
{
    if (dragMarker)
    {
        dragMarker.setPosition(kgeomapPixelToLatLngObject(x ,y));
    }
}

function kgeomapSetDragMarker(x, y, markerCount, markerSelectedCount)
{
    kgeomapRemoveDragMarker();
    var latlng = kgeomapPixelToLatLngObject(x, y);
    var colorCode = kgeomapGetPixmapName(markerCount, markerSelectedCount);
    var clusterIcon = new google.maps.MarkerImage('marker-'+colorCode+'.png', new google.maps.Size(20, 32));
    dragMarker = new google.maps.Marker({
            position: latlng,
            map: map,
            icon: clusterIcon
        });
}

function kgeomapUpdateSelectionRectangleColor()
{
    if (selectionRectangle == null)
    {
        return;
    }

    if (temporarySelectionRectangle == null)
    {
        selectionRectangle.setOptions({
                    fillOpacity : 0.0,
                    strokeColor : "#0000ff",
                    strokeWeight: 1
                });
    }
    else
    {
        selectionRectangle.setOptions({
                    fillOpacity : 0.0,
                    strokeColor : "#ff0000",
                    strokeWeight: 1
                });
    }
}

function kgeomapSetSelectionRectangle(west, north, east, south)
{
    var firstSelectionPoint = new google.maps.LatLng(south,west,true);
    var secondSelectionPoint = new google.maps.LatLng(north,east,true);

    latLngBounds = new google.maps.LatLngBounds(
                    firstSelectionPoint,
                    secondSelectionPoint
                );

    if (selectionRectangle == null)
    {
        selectionRectangle = new google.maps.Rectangle({
                    bounds: latLngBounds,
                    clickable: false,
                    fillOpacity: 0.0,
                    map: map,
                    strokeColor: "#0000FF",
                    strokeWeight: 1,
                    zIndex: 100
                });
    }
    else
    {
        selectionRectangle.setOptions({
                    bounds: latLngBounds,
                    fillOpacity : 0.0,
                    strokeColor : "#FF0000",
                    strokeWeight: 1
                });
    }

    kgeomapUpdateSelectionRectangleColor();
}


function kgeomapSetTemporarySelectionRectangle(west, north, east, south)
{
    var firstPoint = new google.maps.LatLng(south,west,true);
    var secondPoint = new google.maps.LatLng(north,east,true);

    var latLngBounds = new google.maps.LatLngBounds(
            firstPoint,
            secondPoint
        );

    if (temporarySelectionRectangle == null)
    {
        temporarySelectionRectangle = new google.maps.Rectangle({
                    bounds: latLngBounds,
                    clickable: false,
                    fillOpacity: 0.0,
                    map: map,
                    strokeColor: "#0000FF",
                    strokeWeight: 1,
                    zIndex: 100
                });
    }
    else
    {
       temporarySelectionRectangle.setOptions({
                    bounds: latLngBounds,
                    fillOpacity : 0.0,
                    strokeColor : "#0000FF",
                    strokeWeight: 1
                });
    }

    kgeomapUpdateSelectionRectangleColor();
}

function kgeomapRemoveSelectionRectangle()
{
    if (!selectionRectangle)
    {
        return;
    }

    selectionRectangle.setMap(null);
    selectionRectangle = null;
}

function kgeomapRemoveTemporarySelectionRectangle()
{
    if (!temporarySelectionRectangle)
    {
        return;
    }

    temporarySelectionRectangle.setMap(null);
    temporarySelectionRectangle = null;

    kgeomapUpdateSelectionRectangleColor();
}

function kgeomapSelectionModeStatus(state)
{
    map.draggable = !state;

    if (!state)
    {
        kgeomapRemoveTemporarySelectionRectangle();
    }
}

function kgeomapSetMapBoundaries(west, north, east, south, useSaneZoomLevel)
{
    firstPoint = new google.maps.LatLng(south, west, true);
    secondPoint = new google.maps.LatLng(north, east, true);

    newBounds = new google.maps.LatLngBounds(firstPoint, secondPoint);
    map.fitBounds(newBounds);

    if (useSaneZoomLevel && (map.getZoom()>17))
    {
        map.setZoom(17);
    }
}

function kgeomapInitialize()
{
    var latlng = new google.maps.LatLng(52.0, 6.0);
    var myOptions = {
            zoom: 8,
            center: latlng,
            mapTypeId: google.maps.MapTypeId.ROADMAP
        };
    mapDiv = document.getElementById("map_canvas");
    //       mapDiv.style.height="100%"
    map = new google.maps.Map(mapDiv, myOptions);
    google.maps.event.addListener(map, 'maptypeid_changed', function()
        {
            kgeomapPostEventString('MT'+kgeomapGetMapType());
        });

    //google.maps.event.clearListeners(map, 'dragstart');
    //google.maps.event.clearListeners(map, 'drag');
    //google.maps.event.clearListeners(map, 'dragend');
    //google.maps.event.clearInstanceListeners(map);

    //  these are too heavy on the performance. monitor 'idle' event only for now:
    //       google.maps.event.addListener(map, 'bounds_changed', function() {
    //           kgeomapPostEventString('MB');
    //       });
    //       google.maps.event.addListener(map, 'zoom_changed', function() {
    //           kgeomapPostEventString('ZC');
    //       });
    google.maps.event.addListener(map, 'idle', function()
        {
            kgeomapPostEventString('id');
        });
    // source: http://taapps-javalibs.blogspot.com/2009/10/google-map-v3how-to-use-overlayviews.html
    projectionHelper = new ProjectionHelper(map);
}
