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
                                puser[i].getAttribute("online"),
                                puser[i].getAttribute("insession")
                            );
                            member.push(prec);
                        }
                        
                        var mybody  = document.getElementsByTagName("body")[0];
                        var mytable = mybody.getElementsByTagName("table")[0];
                        var mytbody = mytable.getElementsByTagName("tbody")[0];
                        var myrow;
                        var mycel;
                        var myelem;
                        var myelemimg;
                        var numinsession = 0;
                         
                        //alert('row.length ='+mytbody.rows.length)
                        for (j = 0; j < mytbody.rows.length; j++){
                            myrow = mytbody.getElementsByTagName("tr")[j];
                            mycel1 = myrow.getElementsByTagName("td")[1];                           
                            mycel = myrow.getElementsByTagName("td")[0];                                
                            myelem = mycel.childNodes[0]; 
                            myelemimg = myelem.childNodes[0];                           
                            for (i = 0; i < puser.length; i++) {
                                if (mycel1.innerText == member[i][0]) {
                                    found = 1;
                                    if (member[i][2] == 0){
                                        if (myelem.getAttribute("href") != null){
                                            myelem.removeAttribute("href");
                                        } 
                                        if (myelemimg.getAttribute("src") == null){
                                            myelemimg.setAttribute("src", "myoffline2.gif");
                                        }else {
                                            if (myelemimg.getAttribute("src") != "myoffline2.gif"){
                                                myelemimg.setAttribute("src", "myoffline2.gif");
                                            }
                                        }
                                    }
                                    
                                    if ((member[i][2] == 1) && (member[i][3] == 0)){
                                        if (myelem.getAttribute("href") != null){
                                            myelem.removeAttribute("href");
                                        } 
                                        if (myelemimg.getAttribute("src") == null){
                                            myelemimg.setAttribute("src", "myonline2.gif");
                                        }else {
                                            if (myelemimg.getAttribute("src") != "myonline2.gif"){
                                                myelemimg.setAttribute("src", "myonline2.gif");
                                            }
                                        }
                                    }
                                    
                                    if ((member[i][2] == 1) && (member[i][3] == 1)){
                                        numinsession++;
                                        if (myelem.getAttribute("href") == null){
                                            myelem.setAttribute("href", "readyjoin.php");
                                        } else {
                                            if (myelem.getAttribute("href") != "readyjoin.php"){
                                                myelem.setAttribute("href", "readyjoin.php");
                                            }
                                        }
                                        if (myelemimg.getAttribute("src") == null){
                                            myelemimg.setAttribute("src", "mysession.gif");
                                        }else {
                                            if (myelemimg.getAttribute("src") != "mysession.gif"){
                                                myelemimg.setAttribute("src", "mysession.gif");
                                            }
                                        }
                                    }                                                           
                                    
                                    break;
                                } 
                            }
                        }
                    //]]>
                    
                    var a1 = document.getElementById("xfile");
                    var lastlocationurl = a1.value;
                    var changepage = 0;
                    var curl = 'conference1.php';
                    switch (numinsession) {
                        case 0:
                            curl = 'landing.php';
                            changepage = 1;
                        break;
                        
                        case 1: 
                            if (a1.value != 'zero'){
                                a1.value = 'zero';
                                var options = new Array(":aspect-ratio=4:3", "--rtsp-tcp");
                                
                                var p0 = document.getElementById("vlc0");
                                p0.playlist.items.clear();
                                while(p0.playlist.items.count > 0){
                                }
                                
                                var p1 = document.getElementById("vlc1");
                                p1.playlist.items.clear();
                                while(p1.playlist.items.count > 0){
                                }
                                
                                var urival = document.getElementById("uri");
                                urival.value = ''; // member[0][1];
                                var id22 = p1.playlist.add(urival.value, "stream one", options);
                                p1.playlist.stop();
                                
                                var urival0 = document.getElementById("uri0");
                                urival0.value = member[0][1];
                                var id21 = p0.playlist.add(urival0.value, "stream zero", options);
                                p0.playlist.play();
                            }
                        break;
                        
                        case 2: 
                            if (a1.value != 'one'){
                                a1.value = 'one';
                                var options = new Array(":aspect-ratio=4:3", "--rtsp-tcp");

                                var p0 = document.getElementById("vlc0");
                                p0.playlist.items.clear();
                                while(p0.playlist.items.count > 0){
                                }
                                
                                var p1 = document.getElementById("vlc1");
                                p1.playlist.items.clear();
                                while(p1.playlist.items.count > 0){
                                }
                                
                                var urival = document.getElementById("uri");
                                var urival0 = document.getElementById("uri0");
                                var uname = document.getElementById("uname1");
                                if(sesslogged == member[0][0]){
                                    uname.value = member[1][0];
                                    urival0.value = member[0][1];
                                    urival.value = member[1][1];
                                    urival.value = urival.value.replace(/stream0/i, "stream1");
                                } else {
                                    uname.value = member[0][0];
                                    urival0.value = member[1][1];
                                    urival.value = member[0][1];
                                    urival.value = urival.value.replace(/stream0/i, "stream1");
                                }
                                
                                var id22 = p1.playlist.add(urival.value, "stream two", null);                               
                                p1.playlist.playItem(id22);
                                p1.playlist.play();
                                
                                var id21 = p0.playlist.add(urival0.value, "stream zero", null);                               
                                p0.playlist.playItem(id21);
                                p0.playlist.play();
                            }
                        break;
                        
                        default:
                            if (numinsession > 2){
                                //  $_SESSION['udec'] = 0; 
                                changepage = 1;
                                curl = 'conference2.php';
                            }
                        break;
                    }

                    if (changepage > 0){
                        var uremove = document.getElementById("removeuser");
                        uremove.value = '0';
                        document.location = curl;
                    }
                    var t = setTimeout("check()", 5000);
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
            
            function downloadUrl2(url, callback) {
                var request = window.ActiveXObject ?
                new ActiveXObject('Microsoft.XMLHTTP') :
                new XMLHttpRequest;

                request.onreadystatechange = function() {
                    if (request.readyState == 4) {
                        request.onreadystatechange = doNothing;
                        callback(request, request.status);
                    }
                };

                request.open('GET', url, false);
                request.send(null);
            }
            
            $(document.getElementsByTagName('div')[1]).ready(function() {
//                addvlc();
 	    });
            
            function doNothing(){   
            }
            
            google.maps.event.addDomListener(window, 'load', load);
            
            function vplay(vlcid){
                var p1 = document.getElementById(vlcid);
                p1.playlist.play();
            }
            
            function vpause(vlcid){
                var p1 = document.getElementById(vlcid);
                p1.playlist.togglePause();
            }
            
            function vstop(vlcid){
                var p1 = document.getElementById(vlcid);
                p1.playlist.stop();
            }
            
            function vmute(vlcid){
                var p1 = document.getElementById(vlcid);
                p1.audio.toggleMute();
            }
            
            function doalert(){
                alert('doalert');
            }
            
        </script> 
        
    </head> 
    
    <body onload="load();" style="font-family: Tahoma, Geneva, sans-serif;">
        <?
            require "check.php";
            echo "<script type='text/javascript'>var sesslogged='$_SESSION[userid]';</script>";
            $_SESSION['udec'] = 1;
        ?>
        
        <script type="text/javascript"> 
            $(document).ready(function(){
            });
            
            check();
            
            window.onbeforeunload = function (e) {
               // alert('onbeforeunload');
                var uremove = document.getElementById("removeuser");                
                if (uremove.value == '1'){
   //                 alert('doing remove');                    
                    downloadUrl2("removeuser.php", function(data){               
                    });
                    document.location = "landing.php";
                } 
            };
        </script>
        
        <div class="mainwrapper">
            
            <div id="content" class="content"> 
                
                <div id="contacts" style="width: 150px; padding-top:22px; float: right">
                            <hr/>
                            <p style="margin: 0px; text-align: center">
                            <p>Active Sessions</p>
                            <table>
                                <tbody>
                                    <tr>
                                        <td><a><img style="padding-top:4px" alt=""></img></a></td>
                                        <td>Sue</td>
                                    </tr>
                                    <tr>
                                        <td><a><img style="padding-top:4px" alt=""></img></a></td>
                                        <td>Eve</td>
                                    </tr>
                                    <tr>
                                        <td><a><img style="padding-top:4px" alt=""></img></a></td>
                                        <td>Tim</td>
                                    </tr>
                                    <tr>
                                        <td><a><img style="padding-top:4px" alt=""></img></a></td>
                                        <td>Joe</td>
                                    </tr>
                                </tbody>
                            </table>
                            </p>
                            
                                <hr/>
                                <a href="readysession.php"><button>Start Session</button></a>
		</div>                
                
                <div id='vlce1'>
                    <ul>
                        <li><embed type="application/x-vlc-plugin" pluginspage="http://www.videolan.org" version="VideoLAN.VLCPlugin.2"
                        width="320"
                        height="240"
                        id="vlc1"/>
                        <!-- 
                        autoplay="yes" loop="no"
                        target="rtsp://184.72.239.149/vod/mp4:BigBuckBunny_175k.mov"            
                        / -->
                        </li>
                        <!-- input id='uri' value='http://www.revolunet.com/static/download/labo/VLCcontrols/bunny.mp4' size=70 type='text' -->
                        <!-- input id='uri' value='http://download.blender.org/peach/bigbuckbunny_movies/big_buck_bunny_480p_surround-fix.avi' size=70 type='text' -->
                        <!--input id='uri' value='rtsp://184.72.239.149/vod/mp4:BigBuckBunny_175k.mov' size=70 type='text'-->
                        <!--input id='uri' value='rtsp://192.168.46.81:8554/serenity.ts' size=70 type='text'-->
                        <!--input id='uri' value='rtsp://192.168.46.81:8554/stream1.sdp' size=70 type='text'-->                    <!--input id='uri' value='file:///Users/xvdthuser1xvdth/media/vlc-output.ts' size='70' type='text'/-->
                        <!--input id='uri' value="rpurl" size='70' type='text' name="vevo"/-->
                        <li><a href="javascript:;" onclick='return vplay("vlc1")'>Play </a>
                            <a href="javascript:;" onclick='return vpause("vlc1")'>Pause </a>
                            <a href="javascript:;" onclick='return vstop("vlc1")'>Stop </a>
                            <a href="javascript:;" onclick='return vmute("vlc1")'>Mute</a>
                        </li>
                        <li><b><input id='uname1' value='no joiners' size='25' type='text' name="vevo2" style="border: none; font-weight:bold"/></b>
                        </li>
                        <li><input id='uri' value='emptyuri' size='70' type='text' name="vevo2"/></li>        
                        <li><input id='xfile' value='emptyfile' type='hidden' name="vevo"/></li>
                    </ul>
                </div>
                

            </div>
            
            <div id="tslider">
                <div id="vlce0" style="float: left">
                    <ul>
                        <li><embed type="application/x-vlc-plugin" pluginspage="http://www.videolan.org" version="VideoLAN.VLCPlugin.2"
                            width="160"
                            height="120"
                            id="vlc0"/></li>
                        <li>
                            <a href="javascript:;" onclick='return vplay("vlc0")'>Play </a>
                            <a href="javascript:;" onclick='return vpause("vlc0")'>Pause </a>
                            <a href="javascript:;" onclick='return vstop("vlc0")'>Stop </a>
                            <a href="javascript:;" onclick='return vmute("vlc0")'>Mute</a>
                        </li>
                        <li><b><? echo ($_SESSION[userid]); ?></b></li>
                        <li><input id='uri0' value='emptyuri' size='30' type='text' name="vevo2"/></li>        
                        <li><input id='xfile0' value='emptyfile' type='hidden' name="vevo"/></li>
                    </ul>
                </div>
            </div>
        </div>
        
        <div  id="footer" style="float: right">
            <a href="http://localhost:8080"><button><b>Session Test</b></button></a>
            <a href="restartall.php"><button><b>Restart All</b></button></a>
            <a href="landing.php"><button><b>Exit</b></button></a>
        </div>
        
        <input id='removeuser' value='1' type='hidden' name="vevo"/>
    </body> 
</html> 