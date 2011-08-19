<?
    session_start();
?>
<!DOCTYPE html PUBLIC "-//WAPFORUM//DTD XHTML Mobile 1.2//EN" "http://www.openmobilealliance.org/tech/DTD/xhtml-mobile12.dtd">
    <html xmlns="http://www.w3.org/1999/xhtml">
    <head>
        <meta http-equiv="Content-Type" content="application/xhtml+xml; charset=utf-8" />
        <meta http-equiv="cache-control" content="max-age=200" />
                
	<link href="conf1.css" media="handheld, screen" rel="stylesheet" type="text/css" />
        <!-- link rel="stylesheet" type="text/css" href="http://revolunet.github.com/VLCcontrols/src/styles.css" / -->

                
	<title>XVDTH Video</title>
        <style type="text/css">
  		html { height: 100% }
  		body { height: 100%; margin: 0px; padding: 0px; font-family:courier}
  		#map_canvas { height: 100% }
	</style>

        <script type="text/javascript" src="http://maps.google.com/maps/api/js?sensor=true"></script>
        <script type="text/javascript" src="http://ajax.googleapis.com/ajax/libs/jquery/1.6.2/jquery.min.js"></script> 
        <!-- script src="jquery-1.6.2.min.js" type="text/javascript"></script -->
        
        <script type="text/javascript"> 
            //var member = new Array();
            
            function registerVLCEvent(event, handler){
                var vlc = getVLC("vlc1");
                if (vlc) {
                    if (vlc.attachEvent) {
                        // Microsoft
                        vlc.attachEvent (event, handler);
                    } else 
                        if (vlc.addEventListener) {
                            // Mozilla: DOM level 2
                            vlc.addEventListener (event, handler, false);
                        } else {
                            // DOM level 0
                            vlc["on" + event] = handler;
                        }
                    }
            }
            
            // stop listening to event
            function unregisterVLCEvent(event, handler){
                var vlc = getVLC("vlc1");
                if (vlc) {
                    if (vlc.detachEvent) {
                        // Microsoft
                        vlc.detachEvent (event, handler);
                    } else 
                        if (vlc.removeEventListener) {
                            // Mozilla: DOM level 2
                            vlc.removeEventListener (event, handler, false);
                        } else {
                            // DOM level 0
                            vlc["on" + event] = null;
                        }
                    }
            }
            
            // event callback function for testing
            function handleEvents(event){
                if (!event)
                    event = window.event; // IE
                    if (event.target) {
                        // Netscape based browser
                        targ = event.target;
                    } else {
                        if (event.srcElement) {
                            // ActiveX
                            targ = event.srcElement;
                        } else {
                            // No event object, just the value
                            alert("Event value" + event );
                            return;
                        }
                    }
                    
                    if (targ.nodeType == 3){ // defeat Safari bug
                        targ = targ.parentNode;
                    }
                    alert("Event " + event.type + " has fired from " + targ );
            }
            
            // handle mouse grab event from video filter
            function handleMouseGrab(event,X,Y){
                if (!event){
                    event = window.event; // IE
                }
                alert("new position (" + X + "," + Y + ")");
            }
            
            // Register a bunch of callbacks.
            registerVLCEvent('MediaPlayerNothingSpecial', handleEvents);
            registerVLCEvent('MediaPlayerOpening', handleEvents);
            registerVLCEvent('MediaPlayerBuffering', handleEvents);
            registerVLCEvent('MediaPlayerPlaying', handleEvents);
            registerVLCEvent('MediaPlayerPaused', handleEvents);
            registerVLCEvent('MediaPlayerForward', handleEvents);
            registerVLCEvent('MediaPlayerBackward', handleEvents);
            registerVLCEvent('MediaPlayerEncounteredError', handleEvents);
            registerVLCEvent('MediaPlayerEndReached', handleEvents);
            registerVLCEvent('MediaPlayerTimeChanged', handleEvents);
            registerVLCEvent('MediaPlayerPositionChanged', handleEvents);
            registerVLCEvent('MediaPlayerSeekableChanged', handleEvents);
            registerVLCEvent('MediaPlayerPausableChanged', handleEvents);
            
            function check(){
            //alert('check called');
            
                downloadUrl("checkupdate.php", function(data){
                    var xml = data.responseXML;
//                    alert ('checkupdate finished');
                    var puser = xml.documentElement.getElementsByTagName("participant");
                    var member = new Array();
                    //<![CDATA[
                        for (i = 0; i < puser.length ; i++) {
                            var prec = new Array(
	                        puser[i].getAttribute("username"),
                                puser[i].getAttribute("ip"),
                                puser[i].getAttribute("portv"),
                                puser[i].getAttribute("porta")
                            );
                            member.push(prec);
                        }
                    //]]>
                    var a1 = document.getElementById("xfile");
                    var lastlocationurl = a1.value;
                    var changepage = 0;
                    switch (puser.length) {
                        case 1: 
                            if (a1.value != 'conference1.php'){
                                a1.value = 'conference1.php';
                                var p1 = document.getElementById("vlc1");
                                p1.playlist.items.clear();
                                while(p1.playlist.items.count > 0){
                                }
                                var options = new Array(":aspect-ratio=4:3", "--rtsp-tcp");
                                var urival = document.getElementById("uri");
                                urival.value = member[0][1];
                                var id22 = p1.playlist.add(urival.value, "stream one", null);                               
                                //p1.playlist.playItem(id22);
                            }
                        break;
                        
                        case 2: 
                            if (a1.value != 'conference2.php'){
                                a1.value = 'conference2.php';
                                var p1 = document.getElementById("vlc1");
                                p1.playlist.items.clear();
                                while(p1.playlist.items.count > 0){
                                }
                                var options = new Array(":aspect-ratio=4:3", "--rtsp-tcp");
                                var urival = document.getElementById("uri");
                                if(sesslogged == member[0][0]){
                                    urival.value = member[1][1];
                                    var id22 = p1.playlist.add(urival.value, "stream two", null);                               
                                    p1.playlist.playItem(id22);
                                    p1.play();
                                } else {
                                    urival.value = member[0][1];
                                    var id22 = p1.playlist.add(urival.value, "stream two", null);                               
                                    p1.playlist.playItem(id22);
                                }
                                // var id22 = p1.playlist.add(urival.value, "stream two", null);                               
                                // p1.playlist.playItem(id22);
                            }
                        break;
                        
                        case 3:
                            a1.value = '3.html';
                        break;
                        
                        case 4:
                            a1.value = '4.html';
                        break;
                        
                        default:
                            a1.value = 'd.html';
                        break;
                    }
                    // <![CDATA[
                    if ((a1.value != lastlocationurl) && (changepage > 0)){
                        var locationObj = document.location;
                        document.location = a1.value;
                    }else{
                        var t=setTimeout("check()",5000);
                    }
                    // ]]>
                });
            }
        
            function load(){
                //alert ('load called');
            }

            function downloadUrl(url, callback) {
                var request = window.ActiveXObject ?
                new ActiveXObject('Microsoft.XMLHTTP') :
                new XMLHttpRequest;

                request.onreadystatechange = function() {
                    if (request.readyState == 4) {
                        request.onreadystatechange = doNothing;
                        callback(request, request.status);
                    }
                };

                request.open('GET', url, true);
                request.send(null);
            }
            
            $(document.getElementsByTagName('div')[1]).ready(function() {
//                addvlc();
 	    });
            
            function doNothing(){   
            }
            
            google.maps.event.addDomListener(window, 'load', load);

            
        </script> 
        
    </head> 
    
    <body onload="load();">
        <?
            require "check.php";
            echo "<script type='text/javascript'>var sesslogged='$_SESSION[userid]';</script>";
        ?>
        
        <script type="text/javascript"> 
            $(document).ready(function(){
                //alert("ready called");
            });
            //alert(sesslogged);
            check();
        </script>
        
        <div class="mainwrapper">
            <div id="header">
                <div id="header2">Neuron Light 2-Way Demo
                </div>
            </div> 
            
            <div id="content" >        
        <div id='vlce1'>
            <embed type="application/x-vlc-plugin" pluginspage="http://www.videolan.org" version="VideoLAN.VLCPlugin.2"
                width="320"
                height="240"
                id="vlc1"/>
                <!-- 
                autoplay="yes" loop="no"
                target="rtsp://184.72.239.149/vod/mp4:BigBuckBunny_175k.mov"            
                / -->        
   
            <!-- input id='uri' value='http://www.revolunet.com/static/download/labo/VLCcontrols/bunny.mp4' size=70 type='text' -->
            <!-- input id='uri' value='http://download.blender.org/peach/bigbuckbunny_movies/big_buck_bunny_480p_surround-fix.avi' size=70 type='text' -->
            <!--input id='uri' value='rtsp://184.72.239.149/vod/mp4:BigBuckBunny_175k.mov' size=70 type='text'-->
            <!--input id='uri' value='rtsp://192.168.46.81:8554/serenity.ts' size=70 type='text'-->
            <!--input id='uri' value='rtsp://192.168.46.81:8554/stream1.sdp' size=70 type='text'-->                    <!--input id='uri' value='file:///Users/xvdthuser1xvdth/media/vlc-output.ts' size='70' type='text'/-->
            <!--input id='uri' value="rpurl" size='70' type='text' name="vevo"/-->
            <input id='uri' value='emptyuri' type='text' name="vevo2"/>        
            <input id='xfile' value='emptyfile' type='text' name="vevo"/>
        </div>
            </div>
            
            <div id="footer">
            </div>
        </div>         
    </body> 
</html> 