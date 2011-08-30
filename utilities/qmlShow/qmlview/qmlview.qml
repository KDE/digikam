import QtQuick 1.0
import QtWebKit 1.0


Rectangle {

    id:rect
    width: 500
    height: 300
    property string text: "matrix.jpg"

    signal nextClicked;
    signal prevClicked;
    signal play;
    signal pause;
    signal gridItem;
    property bool bool_pp: true;

    Keys.onLeftPressed:
    { rect.prevClicked() }
    Keys.onRightPressed:
    { rect.nextClicked() }
    Keys.onEscapePressed:
    { Qt.quit() }
    Keys.onSpacePressed:
    {
        if (play_pause.src == "pause.svg")
        {
            play_pause.src = "play.svg"; rect.pause();
        }
        else
        {
            play_pause.src= "pause.svg";rect.play();
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

        Image {
            id: play_pause
            x: 0
            y: 0
            width: parent.height / 3
            height: parent.height / 3
            z: 12
            property string src: "pause.svg"
            source: src
            MouseArea
            {
                id: flip_icon
                anchors.fill: parent
                onClicked: { if (parent.src == "pause.svg") {parent.src = "play.svg"; rect.pause();}
                else {parent.src= "pause.svg";rect.play();};
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
            source: "Next.svg"
            MouseArea
            {
                anchors.fill: parent;
                onClicked: rect.nextClicked();
            }
        }

        Image {
            id: prev
            x: next.x - prev.width
            y: next.height + zoom_in.height
            width: parent.width / 5
            height: parent.height / 5
            rotation: 0
            z: 12
            source: "previous.svg"
            MouseArea
            {
                anchors.fill:parent;
                onClicked: rect.prevClicked();
            }
        }

        Image {
            id: zoom_out
            x: 0
            y: prev.y + prev.height/2
            width: parent.width / 5
            height: parent.height / 5
            rotation: 0
            z: 12
            source: "ZoomOut.svg"
        }

        Image {
            id: zoom_in
            x: parent.width / 2
            y: 0
            width: parent.width / 5
            height: parent.height / 5
            rotation: 0
            z: 12
            source: "ZoomIn.svg"
        }

        Image {
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

        Image
        {
            id: gridviewicon
            height:parent.height/5
            width:parent.width/5
            x: parent.width*3/4
            source: "./gridview.png"
            y:0
            MouseArea
            {
                id:show_gridview
                anchors.fill: parent
                onClicked:
                {
                    rect2.visible=true;
                    grid.focus= true;
                    play_pause.src = "play.svg";
                    rect.pause();
                }
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
        source: "close.png"
        MouseArea
        {
            id:quit
            hoverEnabled: true
            anchors.fill: parent
            onClicked:{Qt.quit()}
        }
    }

    Image {
        id: source
        x: 0
        y: 0
        width: parent.width
        height: parent.height
        source: parent.text
        focus: true
        visible:true

        Timer
        {
            id:timer
            interval: 6000; running:rect.bool_pp;
            repeat: rect.bool_pp;
            onTriggered: rect.nextClicked();
        }


        MouseArea
        {
            id: view_icons
            hoverEnabled: true
            anchors.fill: parent
            z: 10
            onPositionChanged:
            {
                circle_image.visible = true;
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
                circle_image.visible = false;
                close.visible = false;
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
