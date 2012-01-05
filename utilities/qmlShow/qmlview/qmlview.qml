import QtQuick 1.0
import QtWebKit 1.0


Rectangle {

    id:rect
    width: 500
    height: 300
    property string text: " "

    signal nextClicked;
    signal prevClicked;
    signal play;
    signal pause;
    signal gridChanged (int index);
    property bool bool_pp: true;
    property real source_scale: 1;

    Keys.onLeftPressed:
    { rect.prevClicked() }
    Keys.onRightPressed:
    { rect.nextClicked() }
    Keys.onEscapePressed:
    { Qt.quit() }
    Keys.onSpacePressed:
    {
        if (play_pause.src == "Def/pause.svg")
        {
            play_pause.src = "Def/play.svg"; rect.pause();
        }
        else
        {
            play_pause.src= "Def/pause.svg";rect.play();
        };
    }

    Image {
        id: circle_image
        x: 0
        y: 0
        width: parent.height / 3
        height: parent.height / 3
        visible: true
        rotation: 0
        opacity: 0.7
        source: "Menu.svg"
        z: 5

        states: [
          State {
            name: "Enable"
            PropertyChanges { target: circle_image; opacity: 1; }
          },
          State {
            name: "Disable"
            PropertyChanges { target: circle_image; opacity: 0; }
          }
        ]

        transitions: [
            Transition {
                from: "*"; to: "*"
                PropertyAnimation {
                    target: circle_image
                    properties: "opacity"; duration: 500 ;
                }
            }
        ]

        Image {
            id: play_pause
            x: next.x - 2/3*prev.width
            y: next.height + 6/5*zoom_in.height
            width: parent.height / 5
            height: parent.height / 5
            z: 12
            property string src: "Def/pause.svg"
            source: src
            MouseArea
            {
                id: flip_icon
                anchors.fill: parent
                onClicked: { if (parent.src == "Def/pause.svg") {parent.src = "Def/play.svg"; rect.pause();}
                else {parent.src= "Def/pause.svg";rect.play();};
                }

            }
        }

        Image {
            id: next
            x: zoom_in.x - (width / 2)
            y: zoom_in.height
            width: parent.width / 5
            height: parent.height / 5
            rotation: 0
            z: 12
            source: "Def/next.svg"
            MouseArea
            {
                anchors.fill: parent;
                onClicked: rect.nextClicked();
                onPressed: parent.source = "Onpress/next1.svg";
                onReleased: parent.source = "Def/next.svg";
            }
        }

        Image {
            id: prev
            x: zoom_out.x + 4/3*zoom_out.width
            y: parent.height * 3/4 - 1/2 *zoom_out.height
            width: parent.width / 5
            height: parent.height / 5
            rotation: 0
            z: 12
            source: "Def/back.svg"
            MouseArea
            {
                anchors.fill:parent;
                onClicked: rect.prevClicked();
                onPressed: parent.source = "Onpress/back1.svg";
                onReleased: parent.source = "Def/back.svg";
            }
        }

        Image {
            id: zoom_out
            x: 0
            y: parent.height*3 / 4
            width: parent.width / 5
            height: parent.height / 5
            rotation: 0
            z: 12
            source: "Def/Zoom-.svg"
                MouseArea
                {
                        id:zoomout
                        anchors.fill: parent
                        onClicked: {rect.source_scale = rect.source_scale / 2;}
                        onPressed: parent.source = "Onpress/Zoom-1.svg";
                        onReleased: parent.source = "Def/Zoom-.svg";
                }
        }

        Image {
            id: zoom_in
            x: 3/4 * parent.width
            y: 0
            width: parent.width / 5
            height: parent.height / 5
            rotation: 0
            z: 12
            source: "Def/Zoom+.svg"
                MouseArea
                {
                        id:zoomin
                        anchors.fill: parent
                        onClicked: {rect.source_scale = rect.source_scale * 2;}
                        onPressed: parent.source = "Onpress/Zoom+1.svg";
                        onReleased: parent.source = "Def/Zoom+.svg";
                }
        }

        /*Image {
            id: map_view
            x: 0
            y: parent.height*3 / 4
            width: parent.width / 5
            height: parent.height / 5
            source: "map_view.png"

            MouseArea {
                id: show_map
                anchors.fill: parent
                onClicked: { if (openstreetmap.visible == false) openstreetmap.visible = true
                else openstreetmap.visible=false;}
            }
        }
*/

        Image {
            id: gridviewicon
            height:parent.height/5
            width:parent.width/5
            x:5
            source: "Def/grid.svg"
            y:5
            MouseArea {
                id:show_gridview
                anchors.fill: parent
                onClicked:
                {
                if(rect2.visible==false){
                    rect2.visible=true;
                    grid.focus= true;
                    play_pause.src = "Def/play.svg";
                    rect.pause();}
                else
                { rect2.visible = false;
                        rect.focus = true;
                }
                }

                onPressed: parent.source = "Onpress/grid1.svg";
                onReleased: parent.source = "Def/grid.svg";
            }
        }
    }

    Image {
        id: close
        x: parent.width - width
        y: 0
        width: parent.height/8
        height: parent.height/8
        visible: true
        z: 5
        opacity: 0.5
        rotation: 0
        source: "Def/close.svg"
        MouseArea
        {
            id:quit
            hoverEnabled: true
            anchors.fill: parent
            onClicked:{Qt.quit()}
            onPressed: parent.source = "Onpress/close1.svg";
            onReleased: parent.source = "Def/close.svg";
        }
    }

        Flickable
        {
        id:flickable
        anchors.fill: parent
        contentWidth: source.width
        contentHeight: source.height;
        contentX: (source.width - rect.width) / 2;
        contentY: (source.height - rect.height) / 2;
        boundsBehavior: Flickable.StopAtBounds;
    Image {
        id: source
        anchors.centerIn: rect
        width: rect.width * rect.source_scale
        height: rect.height * rect.source_scale
        source: rect.text
        visible:true

        Timer
        {
            id:timer
            interval: 6000; running:rect.bool_pp;
            repeat: rect.bool_pp;
            onTriggered: rect.nextClicked();
        }

        MouseArea {
            id: view_icons
            hoverEnabled: true
            anchors.fill: parent
            z: 10
            onPositionChanged:
            {
                circle_image.state = "Enable";
                close.visible = true ;
                remove_icon.running = true;
            }
        }

        Timer
        {
            id: remove_icon
            interval: 4000; running: false;
            onTriggered:
            {
                circle_image.state = "Disable";
                close.visible = false;
            }
         }

    }
}

    WebView {
        id: openstreetmap
        x: 0
        y: circle_image.height
        height: circle_image.height
        width: circle_image.width
        visible: false
        url: "./mapview.html"
    }

    Rectangle
    {
        id:rect2
        visible: false
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
        height:(parent.height/3)*2
        width:(parent.width/3)*2
        radius: 15
        opacity:0.7
        color: "black"
        GridView
        {
            id: grid;
            visible: true
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
            height: rect2.height -50
            width: rect2.width - 40;
            cellHeight: grid.height/3
            cellWidth: grid.width/3
            clip: true
            model: myModel
            cacheBuffer: 0
            highlightFollowsCurrentItem: true
            highlight:	Rectangle{
                id: highlight_rect;
                color: "white";
                border.width: 10;
                border.color: "white";
                clip: true
                height: grid.cellHeight;
                width: grid.cellWidth;
		onXChanged: rect.gridChanged(grid.currentIndex);
		onYChanged: rect.gridChanged(grid.currentIndex);
            }
            focus: parent.visible

            Keys.onEscapePressed:
            {
                rect2.visible=false; rect.focus=true
            }

            delegate: Component {
                Rectangle {
                    id:image;
                    width:grid.cellWidth-20;
                    height:grid.cellHeight-20;
                    radius: 10;
                    clip: true
                    Image{anchors.fill:parent;clip:true;source: modelData; }
                    MouseArea
                    {
                        anchors.fill:parent;
                        clip: true;
                        onClicked: {
                            grid.currentIndex = grid.indexAt(parent.x,parent.y);
                        }
                    }
                }
            }
        }
    }
}

