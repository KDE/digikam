import QtQuick 1.0


Rectangle {

    id:rect
    focus: true;
	color:"black"


    signal nextClicked;
    signal prevClicked;
    signal play;
    signal pause;
    signal gridChanged (int index);
	signal loadMetaData();

    property bool bool_pp: true;
	property bool editbox_opened: false;
    property real source_scale: 1.0;
	property int imagewidth: 0
	property int imageheight: 0
	property string text: ""

    Keys.onLeftPressed:
    { rect.prevClicked(); rect.source_scale=1.0 }
    Keys.onRightPressed:
    { rect.nextClicked(); rect.source_scale=1.0}
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
			height:parent.height/5
            width:parent.width/5
            x:5
            y:5 
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
			x: next.x - 2/3*prev.width
            y: next.height + 6/5*zoom_in.height
            width: parent.height / 5
            height: parent.height / 5
            source: "Def/grid.svg"
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
/*    }*/

	Image{
        id:edit_icon;
        source:"blue_pencil.png";
        x:gridviewicon.x-gridviewicon.width;
        y:gridviewicon.y-gridviewicon.height;
		z:10;
        height:gridviewicon.height;
        width:height;
		MouseArea
		{
			id:open_metadata_editbox
			anchors.fill:parent;
			onClicked:
			{
				if(rect.editbox_opened==false) {rect.open_metadata_editbox();rect.editbox_opened=true;}
				else {editbox.visible=false;rect.editbox_opened=false;rect.focus=true;}
			}
		}
    }

    }

	Rectangle
	{
    	id:editbox
    	x:parent.width/4
    	y:parent.height*3/8
		z:10
    	width: parent.width/2
    	height: parent.height/4;
    	color: "black"
    	opacity: 0.7
    	radius:10
    	property string name: "";
		property string data_time: "";
		property int pick_label: 0;
		property int color_label: 0;
		property int rating: 0;
		visible: false;
		objectName:"editbox";
		Text
		{
			id:name_text;
			text:"Image Name";
			color: "white";
			x:5;
			y:5;
			font.family : "Helvetica"
            font.pixelSize: 20;
		}
		Rectangle
		{
		id:image_name_rect;
		color:"white";
		x:name_text.x+name_text.width+5;
		y:name_text.y;
		width:parent.width-name_text.width-15;
		height:image_name.height+5;
		radius:5;
    	TextInput
	    {
	        id: image_name;
		    text:editbox.name;
			anchors.centerIn: parent;
			font.family : "Helvetica"
			font.pixelSize: 20;
			font.bold: true;
			x: parent.x;
	        y: parent.y;
	        width:parent.width-10;
			color:"black"
	    }
		}
		Text
        {
            id:dt_text;
            text:"Date & Time";
			color:"gray"
            x:name_text.x;
            y:image_name_rect.y+image_name_rect.height+5;
            font.family : "Helvetica"
            font.pixelSize: 20;
        }
		Text
	    {
	        id: image_dt;
			color: "white"
		    text:editbox.data_time;
			font.family : "Helvetica"
			font.pixelSize: 20;
			font.bold: true;
			x:image_name_rect.x;
			y:dt_text.y;
	        width:parent.width-10;
	    }
		Text
		{
			id: picklabel;
/*			text: {if(editbox.pick_label==0) return "NoPickLabel"; else if(editbox.pick_label==1) return "RejectedLabel"; else if(editbox.pick_label==2) return "PendingLabel"; else if(editbox.pick_label==3) return "AcceptedLabel"; else return "InvalidLabel"}*/
			text: "Pick Label";
			color:"gray";
			font.family : "Helvetica"
            font.pixelSize: 20;
            x:5;
            y:image_dt.y+30;
		}
		/*Rectangle
        {
			id:no_pick_label_rect;
            x:image_name_rect.x;
            y:picklabel.y;
            height:picklabel.height;
            width:height;
            color:"gray";
            radius:5;
        }*/
		Grid
		{
			x:image_name_rect.x;
			y:picklabel.y;
			height:picklabel.height;
			columns:4
			rows:1
			spacing: 10
			Rectangle
			{
				id:no_pick_rect;
				color:"gray";
				radius:5;
				height:parent.height;
				width:height;
				
/*				ToolTip {id:tooltip;text:"NoPickLabel";target:no_pick_rect;visible:mouseArea.onEntered;}
				MouseArea {id:mouseArea;anchors.fill:parent;}*/
			}
			Rectangle
			{
				id:rejected_label_rect;
				color:"gray";
				radius:5;
				height:parent.height;
				width:height;
				Image
				{
					id:rejected_label_image;
					source:"flag2.png";
					anchors.centerIn:parent;
					height:2*parent.height/3;
					width:height;
				}
			}
			Rectangle
			{
				id:pending_label_rect;
				color:"gray";
				radius:5;
				height:parent.height;
				width:height;
				Image
				{
					id:pending_label_image;
					source:"flag2.png";
					anchors.centerIn:parent;
					height:2*parent.height/3;
					width:height;
				}
			}
			Rectangle
			{
				id:accepted_label_rect;
				color:"gray";
				radius:5;
				height:parent.height;
				width:height;
				Image
				{
					id:accepted_label_image;
					source:"flag2.png";
					anchors.centerIn:parent;
					height:2*parent.height/3;
					width:height;
				}
			}
		}
		Image
		{
			id:flag_1;
		}
		Text
		{
			id: colorlabel;
			/*            text: {if(editbox.color_label==0) return "NoColorLabel"; else if(editbox.color_label==1) return "RedLabel"; else if(editbox.color_label==2) return "OrangeLabel"; else if(editbox.color_label==3) return     "YellowLabel"; else if(editbox.color_label==4) return "GreenLabel"; else if(editbox.color_label==5) return "BlueLabel"; else if(editbox.color_label==6) return "MagentaLabel"; else if(editbox.color_label==7) return "GrayLabel"; else if(editbox.color_label==8) return "BlackLabel"; else if(editbox.color_label==9) return "WhiteLabel"; else return "InvalidLabel";}*/
			text:"Color Label";
			color:"gray";
			font.family : "Helvetica"
			font.pixelSize: 20;
			x:5;
			y:picklabel.y+30;
		}
	Grid
        {
            x:image_name_rect.x;
            y:colorlabel.y;
            height:picklabel.height;
            columns:10
            rows:1
            spacing: 10
			Rectangle
			{
				id:no_color_rect;
				color:"gray";
				radius:5;
				height:parent.height;
				width:height;
				Image
				{
					id:no_color_image;
				 	source:"cross.png";
					anchors.centerIn:parent;
					height:2*parent.height/3;
					width:height;
				}
			}
			Rectangle
			{
				id:red_color_rect;
				color:"gray";
				radius:5;
				height:parent.height;
				width:height;
				Rectangle
				{
					id:red_color_image;
					color:"red";
					anchors.centerIn:parent;
					height:2*parent.height/3;
					width:height;
				}
			}
			Rectangle
			{
				id:orange_color_rect;
				color:"gray";
				radius:5;
				height:parent.height;
				width:height;
				Rectangle
				{
					id:orange_color_image;
					color:"orange";
					anchors.centerIn:parent;
					height:2*parent.height/3;
					width:height;
				}
			}
			Rectangle
			{
				id:yellow_color_rect;
				color:"gray";
				radius:5;
				height:parent.height;
				width:height;
				Rectangle
				{
					id:yellow_color_image;
					color:"yellow";
					anchors.centerIn:parent;
					height:2*parent.height/3;
					width:height;
				}
			}
			Rectangle
			{
				id:green_color_rect;
				color:"gray";
				radius:5;
				height:parent.height;
				width:height;
				Rectangle
				{
					id:green_color_image;
					color:"green";
					anchors.centerIn:parent;
					height:2*parent.height/3;
					width:height;
				}
			}
			Rectangle
			{
				id:blue_color_rect;
				color:"gray";
				radius:5;
				height:parent.height;
				width:height;
				Rectangle
				{
					id:blue_color_image;
					color:"blue";
					anchors.centerIn:parent;
					height:2*parent.height/3;
					width:height;
				}
			}
			Rectangle
			{
				id:magenta_color_rect;
				color:"gray";
				radius:5;
				height:parent.height;
				width:height;
				Rectangle
				{
					id:magenta_color_image;
					color:"magenta";
					anchors.centerIn:parent;
					height:2*parent.height/3;
					width:height;
				}
			}
			Rectangle
			{
				id:gray_color_rect;
				color:"gray";
				radius:5;
				height:parent.height;
				width:height;
				Rectangle
				{
					id:gray_color_image;
					color:"gray";
					anchors.centerIn:parent;
					height:2*parent.height/3;
					width:height;
				}
			}
			Rectangle
			{
				id:black_color_rect;
				color:"gray";
				radius:5;
				height:parent.height;
				width:height;
				Rectangle
				{
					id:black_color_image;
					color:"black";
					anchors.centerIn:parent;
					height:2*parent.height/3;
					width:height;
				}
			}
			Rectangle
			{
				id:white_color_rect;
				color:"gray";
				radius:5;
				height:parent.height;
				width:height;
				Rectangle
				{
					id:white_color_image;
					color:"white";
					anchors.centerIn:parent;
					height:2*parent.height/3;
					width:height;
				}
			}
		}
		Text
		{
			id: rating_text;
            color: "gray"
//            text:editbox.rating;
			text:"Rating";
            font.family : "Helvetica"
            font.pixelSize: 20;
            x:5;
            y:colorlabel.y+30;
            width:parent.width-10;
		}
		Grid
        {
            x:image_name_rect.x;
            y:rating_text.y;
            height:picklabel.height;
            columns:5
            rows:1
            spacing: 10
			Image
			{
				id:rating_1;
				source:"empty_star.png";
				height:parent.height;
				width:height;
			}
			Image
			{
				id:rating_2;
				source:"empty_star.png";
				height:parent.height;
				width:height;
			}
			Image
			{
				id:rating_3;
				source:"empty_star.png";
				height:parent.height;
				width:height;
			}
			Image
			{
				id:rating_4;
				source:"empty_star.png";
				height:parent.height;
				width:height;
			}
			Image
			{
				id:rating_5;
				source:"empty_star.png";
				height:parent.height;
				width:height;
			}
		}
	}

	function open_metadata_editbox()
	{
		editbox.visible=true;
		rect.loadMetaData();
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
        contentWidth: (rect.width<source.width)?source.width:rect.width;
        contentHeight: (rect.height<source.height)?source.height:rect.height;
//        x: (rect.width - source.width) / 2;
//        y: (rect.height - source.height) / 2;
        boundsBehavior: Flickable.StopAtBounds;
    Image {
        id: source
        anchors.centerIn: parent
        width: rect.imagewidth*rect.source_scale;
        height: rect.imageheight*rect.source_scale;
        source: rect.text
        visible:true

        Timer
        {
            id:timer
            interval: 6000; running:rect.bool_pp;
            repeat: rect.bool_pp;
            onTriggered: rect.nextClicked();
        }

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
                opacity:0.7;
                border.width: 10;
                border.color: "yellow";
                clip: true
                height: grid.cellHeight;
                width: grid.cellWidth;
				onXChanged: {
								rect.gridChanged(grid.currentIndex);
							}
				onYChanged: {
								rect.gridChanged(grid.currentIndex);
							}
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

	Text
	{
		text: imagedata
		font.family: "Terminal"
		font.pointSize: 8
		color: "white"
		x:30
		y:rect.height-70
		z:500
	}
}

