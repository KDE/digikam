import QtQuick 1.0
import QtWebKit 1.0


Rectangle {
	id:rect
    width: 500
    height: 30     
property string text: "matrix.jpg"
signal nextClicked;
signal prevClicked;
signal play;
signal pause;
property bool bool_pp: true;
    Image {
        id: circle_image
        x: 0
        y: 0
        width: parent.height / 3
        height: parent.height / 3
        visible: false
        rotation: 0
        opacity: 0.7
        source: "circle.png"
        z: 5

        Image {
            id: play_pause
            x: 0
            y: 0
            width: parent.height / 3
            height: parent.height / 3
            z: 12
            property string src: "pause.png"
            source: src
            MouseArea
            {
                id: flip_icon
                anchors.fill: parent
                onClicked: { if (parent.src == "pause.png") {parent.src = "play.png"; rect.pause();}
                    else {parent.src= "pause.png";rect.play();};
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
            source: "next.png"
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
            source: "previous.png"
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
            source: "zoom-out.png"
        }

        Image {
            id: zoom_in
            x: parent.width / 2
            y: 0
            width: parent.width / 5
            height: parent.height / 5
            rotation: 0
            z: 12
            source: "zoom-in.png"
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
    }

    Image {
        id: close
        x: parent.width - width
        y: 0
        width: parent.height/8
        height: parent.height/8
        visible: false
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
            onPositionChanged: { circle_image.visible = true; close.visible = true ; remove_icon.running = true;}
        }

        Timer
            {
                id: remove_icon
                interval: 4000; running: false;
                onTriggered: {circle_image.visible = false; close.visible = false; }
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

}
