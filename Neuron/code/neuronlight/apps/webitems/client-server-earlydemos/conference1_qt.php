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

        <script type="text/javascript" src="AC_QuickTime.js" language="javascript"></script>
        <script type="text/javascript" src="http://maps.google.com/maps/api/js?sensor=true"></script>
        <script type="text/javascript" src="http://ajax.googleapis.com/ajax/libs/jquery/1.6.2/jquery.min.js"></script> 
        <!-- script src="jquery-1.6.2.min.js" type="text/javascript"></script -->
        
        <script type="text/javascript"> 
            // quicktime start
            var togglepause = 1;
            var togglemute = true;
            
            /* define function that shows percentage of movie 1 loaded */
            function showProgress()
            {   var percentLoaded = 0 ;
                percentLoaded = parseInt((document.movie1.GetMaxTimeLoaded() / document.movie1.GetDuration()) * 100);
                document.getElementById("loadStatus1").innerHTML = 'Movie loading: ' + percentLoaded + '% complete...';
            }
            /* define function that executes when movie1 loading is complete */
            function movieLoaded()
            {   document.getElementById("loadStatus1").innerHTML = "Movie Loaded" ;
            }
            /* define function that shows percentage of movie 0 loaded */
            function showProgress0()
            {   var percentLoaded = 0 ;
                percentLoaded = parseInt((document.movie0.GetMaxTimeLoaded() / document.movie0.GetDuration()) * 100);
                document.getElementById("loadStatus0").innerHTML = 'Movie loading: ' + percentLoaded + '% complete...';
            }
            /* define function that executes when movie0 loading is complete */
            function movieLoaded0()
            {   document.getElementById("loadStatus0").innerHTML = "Movie Loaded" ;
            }
            /* define function that adds another function as a listener for a DOM event */
            function myAddListener(obj, evt, handler, captures)
            {   if ( document.addEventListener )
                    obj.addEventListener(evt, handler, captures);
                else
                    // IE
                    obj.attachEvent('on' + evt, handler);
            }
 
            /* define functions that register each listener */
            function RegisterListener(eventName, objID, embedID, listenerFcn)
            {   var obj = document.getElementById(objID);
                if ( !obj )
                    obj = document.getElementById(embedID);
                if ( obj )
                    myAddListener(obj, eventName, listenerFcn, false);
            }
 
            /* define a single function that registers all listeners to call onload */
            function RegisterListeners()
            {   RegisterListener('qt_progress', 'movie1', 'qtmovie_embed1', showProgress);
                RegisterListener('qt_load', 'movie1', 'qtmovie_embed1', movieLoaded);
                
                RegisterListener('qt_progress', 'movie0', 'qtmovie_embed0', showProgress0);
                RegisterListener('qt_load', 'movie0', 'qtmovie_embed0', movieLoaded0);
            }

        // quicktime end
                        
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
                                puser[i].getAttribute("insession"),
                                puser[i].getAttribute("width"),
                                puser[i].getAttribute("height")
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
                    var curl = 'conference1_qt.php';
                    // alert('aratio'+aspectratio);
                    // var options = new Array(aspectratio, "--rtsp-tcp");
                    switch (numinsession) {
                        case 0:
                            curl = 'landing.php';
                            changepage = 1;
                        break;
                        
                        case 1: 
                            if (a1.value != 'zero'){
                                a1.value = 'zero';
                                var p0 = document.getElementById("vlc0");                                
                                var p1 = document.getElementById("vlc1");
                                
                                var urival = document.getElementById("uri");
                                urival.value = ''; // 'rtsp://184.72.239.149/vod/mp4:BigBuckBunny_175k.mov'; // ''; // member[0][1];
                                //var options = new Array(aspectratio, "--rtsp-tcp");
                                //var id22 = p1.playlist.add(urival.value, "stream one", options);
                                //document.movie1.
                                //p1.playlist.stop();
                                
                                var urival0 = document.getElementById("uri0");
                                urival0.value = member[0][1]; // member[0][1]; // rtsp://184.72.239.149/vod/mp4:BigBuckBunny_175k.mov
                                //document.movie0.SetHREF(urival0.value);
                                //var matrix = document.movie1.GetMatrix();
                                //alert('matrix = ' + matrix.value);
                            //var mymat  = document.getElementById("matrix");
                            //mymat.value = 'stuff'; //document.movie1.GetMatrix();

                                document.movie0.play();
                            }
                        break;
                        
                        case 2: 
                            curl = 'conference1a_qt.php';
                            changepage = 1;
                        break;
                        
                        default:
                            if (numinsession > 2){
                                //  $_SESSION['udec'] = 0; 
                                changepage = 1;
                                curl = 'conference2_qt.php';
                            }
                        break;
                    }

                    if (changepage > 0){
                        var uremove = document.getElementById("removeuser");
                        uremove.value = '0';
                        document.location = curl;
                    }
//                var p0 = document.getElementById("vlc0");
//                var swidth = p0.aspectRatio;
//                alert ('swidth ='+ swidth);
                    var t = setTimeout("check()", 5000);
                });
                
                

            }
        
            function load(){
                //alert ('load called');
            }
            
            function land(ref, target)
            {   lowtarget = target.toLowerCase();
                if (lowtarget == "_self") {
                    window.location=loc;
                } else {
                    if (lowtarget == "_top") {
                        top.location = loc;
                    } else {
                        if (lowtarget == "_blank") {
                            window.open(loc);
                        }else {
                            if (lowtarget == "_parent") {
                                parent.location = loc;
                            }else {
                                parent.frames[target].location = loc;
                            };
                        }
                    }
                }
            }
            
            function jump(menu)
            {   ref = menu.choice.options[menu.choice.selectedIndex].value;
                splitc = ref.lastIndexOf("*");
                target = "";
                if (splitc != -1)
                {   loc = ref.substring(0,splitc);
                    target = ref.substring(splitc+1,1000);
                }
                else {
                    loc = ref; 
                    target="_self";
                };

                if (ref != "") {
                    land(loc,target);
                }
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
                document.movie1.Play();
            }
            
            function vpause(vlcid){
                if (togglepause == 1){
                    document.movie1.SetRate(0);
                    togglepause = 0;
                } else{
                    document.movie1.Play();
                    togglepause = 1;
                }
            }
            
            function vstop(vlcid){
                document.movie1.Stop();
            }
            
            function vmute(vlcid){
                if (togglemute == true){
                    document.movie1.SetMute(togglemute);
                    togglemute = false;
                } else{
                    document.movie1.SetMute(togglemute);
                    togglemute = true;
                }
            }
            
            function test()
            {   alert("JavaScript function called from QuickTime!");
            }
 
            
            function doalert(){
                alert('doalert');
            }
            
        </script> 
        
    </head> 
    
    <body onload="onload();" style="font-family: Tahoma, Geneva, sans-serif;">
        <?
            require "check.php";
            echo "<script type='text/javascript'>var sesslogged='$_SESSION[userid]';</script>";
            $_SESSION['udec'] = 1;
            
            // Opens a connection to a MySQL server
            $mysqli= new mysqli ("127.0.0.1", "xvdth", "12345", "xvdth" );    
            if (!$mysqli) {  
                printf("failed");
                die('Not connected : ' . mysqli_error());
            } else {
        
                $sql = "select * from user where insession=1";
                $result = mysqli_query($mysqli, $sql);
                
                if (!$result) {  
                    die('Invalid query: ' . mysqli_error());
                } else {
                    $count = 0;
                    while ($row = @mysqli_fetch_assoc($result)){ 
                            switch ($count){
                                case 0:
                                    if ($row['username'] == $_SESSION[userid]){  
                                        $_SESSION[aratio0] = $_SESSION[cheight] / $_SESSION[cwidth];
                                        $_SESSION[dheight0] = round($_SESSION[dwidth0] * $_SESSION[aratio0]);                                        
                                        if ($_SESSION[dheight0] > $_SESSION[d0constheight]){
                                            $_SESSION[dheight0] = $_SESSION[d0constheight];
                                            $_SESSION[dwidth0] = round($_SESSION[dheight0] * (1 / $_SESSION[aratio0]));
                                        }
                                        $count--;
                                    } else {
                                        $_SESSION[aratio1] = $row['height'] / $row['width'];
                                        //$_SESSION[dwidth1] = $row['width'];
                                        $_SESSION[dheight1] = round($_SESSION[dwidth1] * $_SESSION[aratio1]);
                                        if ($_SESSION[dheight1] > $_SESSION[d1constheight]){
                                            $_SESSION[dheight1] = $_SESSION[d1constheight];
                                            $_SESSION[dwidth1] = round($_SESSION[dheight1] * (1 / $_SESSION[aratio1]));
                                        }
                                    }
                                break;
                            
                                case 1:
                                    if ($row['username'] == $_SESSION[userid]){  
                                        $_SESSION[aratio0] = $_SESSION[cheight] / $_SESSION[cwidth];
                                        $_SESSION[dheight0] = round($_SESSION[dwidth0] * $_SESSION[aratio0]); 
                                        if ($_SESSION[dheight0] > $_SESSION[d0constheight]){
                                            $_SESSION[dheight0] = $_SESSION[d0constheight];
                                            $_SESSION[dwidth0] = round($_SESSION[dheight0] * (1 / $_SESSION[aratio0]));
                                        }                                        
                                        $count--;
                                    } else {
                                        $_SESSION[aratio2] = $row['height'] / $row['width'];
                                        // $_SESSION[dwidth2] = $row['width'];
                                        $_SESSION[dheight2] = round($_SESSION[dwidth2] * $_SESSION[aratio2]);
                                        if ($_SESSION[dheight2] > $_SESSION[d1constheight]){
                                            $_SESSION[dheight2] = $_SESSION[d1constheight];
                                            $_SESSION[dwidth2] = round($_SESSION[dheight2] * (1 / $_SESSION[aratio2]));
                                        }                                        
                                    }
                                break;
                            
                                default:
                                break;
                            }
                        $count++;                        
                    }
                    $result->close();
                }
            }            
        ?>
        
        <script type="text/javascript"> 
            var aspectratio = ":aspect-ratio=";
            var aratio = "4:3";
            aspectratio = aspectratio + aratio;
            
            var mywidth = "160";
            
            function dowidth(){
                return "160";
            }
            
            RegisterListeners();
            
            $(document).ready(function(){
                //p0.aspectRatio = "1:1";
                //var swidth = p0.aspectRatio;
                //alert ('swith ='+ swidth);
                var p0 = document.getElementById("vlc0");
                var ewidth = p0.width;
                var eheight = p0.height;
                var esize = ewidth + 'x' + eheight;
                var dsize = document.getElementById("size0");
                dsize.value = esize;
                
                p0 = document.getElementById("vlc1");
                ewidth = p0.width;
                eheight = p0.height;
                esize = ewidth + 'x' + eheight;
                dsize = document.getElementById("size1");
                dsize.value = esize;                
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
                                <form action="dummy" method="post">
                                    <div align="center">
                                        <p align="center">
                                            <select name="choice" size="1">
                                                 <option value="readysession_qt.php">qt</option>
                                                 <option value="readysession.php">vlc</option>
                                             </select><br/>
                                             <input TYPE="button" VALUE="Start Session" onClick="jump(this.form)"></input>
                                        </p>

                                    </div>
                                    
                                </form>
                                
                                <!-- a href="readysession.php"><button>Start Session</button></a -->
                                <!-- INPUT TYPE="BUTTON" OnClick="checkplugin();" VALUE="Start Session"></INPUT -->
		</div>                
                
                <div id='vlce1'>
                    <script language="javascript" type="text/javascript">
                        var width = "<? echo $_SESSION[dwidth1] ?>";
                        var height = "<? echo $_SESSION[dheight1] ?>";                                            

                        QT_WriteOBJECT('myoffline2.gif' , width, height, '', 'HREF', 'rtsp://184.72.239.149/vod/mp4:BigBuckBunny_175k.mov','autohref' ,'true', 'scale','tofit','target', 'myself','obj#id', 'movie1', 'emb#id', 'qtmovie_embed1', 'emb#name', 'movie1', 'postdomevents', 'true', 'enablejavascript', 'true');
                                //var matrix = document.movie1.GetMatrix();
                                //alert('matrix = ' + matrix);
                        // QT_WriteOBJECT('myoffline2.gif' , width, height, '', 'HREF', 'rtsp://192.168.46.67:8554/stream0','autohref' ,'true', 'target', 'myself','obj#id', 'movie1', 'emb#id', 'qtmovie_embed1', 'emb#name', 'movie1', 'postdomevents', 'true', 'enablejavascript', 'true');
                        // QT_WriteOBJECT('rtsp://184.72.239.149/vod/mp4:BigBuckBunny_175k.mov', width, height, '', 'obj#id', 'movie1', 'emb#id', 'qtmovie_embed1', 'emb#name', 'movie1', 'postdomevents', 'true', 'enablejavascript', 'true');
                        // QT_WriteOBJECT('rtsp://192.168.46.67:8554/stream0', width, height, '', 'obj#id', 'movie1', 'emb#id', 'qtmovie_embed1', 'emb#name', 'movie1', 'postdomevents', 'true', 'enablejavascript', 'true');
                        // QT_WriteOBJECT('http://www.revolunet.com/static/download/labo/VLCcontrols/bunny.mp4', width, height, '', 'obj#id', 'movie1', 'emb#id', 'qtmovie_embed1', 'emb#name', 'movie1', 'postdomevents', 'true', 'enablejavascript', 'true');
                        // QT_WriteOBJECT('rtsp://184.72.239.149/vod/mp4:BigBuckBunny_175k.mov', '50%','25%', '', 'AUTOPLAY', 'true', 'scale', 'aspect'); //, 'emb#name', 'movie1', 'postdomevents', 'true', 'enablejavascript', 'true');                      
                    </script>

                    <!-- iframe  id="Iframe1"  name="Iframe1" frameborder="0"  vspace="0"  hspace="0"  marginwidth="0"  marginheight="0" width="2"  height="2"  src="blank.html">
                    </iframe -->
 
                    <!-- script type="text/javascript">
                        //<![CDATA[                                               
                            // QT_WriteOBJECT('http://www.revolunet.com/static/download/labo/VLCcontrols/bunny.mp4' , '640', '480', '', 'HREF', '<Calltest.html> T<Iframe1>');
                            // QT_WriteOBJECT('rtsp://184.72.239.149/vod/mp4:BigBuckBunny_175k.mov' , '320', '240', '', 'HREF', '<Calltest.html> T<Iframe1>');
                            //var width = "< ? echo $_SESSION[dwidth1] ?>";
                            //var height = "< ? echo $_SESSION[dheight1] ?>";           
                            //QT_WriteOBJECT('myoffline2.gif' , width, height, '', 'HREF', 'rtsp://184.72.239.149/vod/mp4:BigBuckBunny_175k.mov','autohref' ,'true', 'target', 'myself'); 
                            
                            // QT_WriteOBJECT('myoffline2.gif' , width, height, '', 'HREF', 'rtsp://192.168.46.67:8554/stream0','autohref' ,'true', 'target', 'myself'); 
                            // QT_WriteOBJECT('rtsp://184.72.239.149/vod/mp4:BigBuckBunny_175k.mov' , '320', '240', '', 'HREF', '<Calltest.html> T<Iframe1>');
                        //]]>
                    </script -->
                    &nbsp;&nbsp;
                    <p ID="loadStatus1">MOVIE LOADING...</p>
                    <ul>
                        <li><b><input id='uname1' value='no joiners' size='25' type='text' name="vevo2" style="border: none; font-weight:bold"/></b>
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
                        <li>matrix<input id='matrix' value='emptyuri' size='70' type='text' name="vevo2"/></li>
                        <script language="javascript" type="text/javascript">
                            // alert ('here');
                            var mymat  = document.getElementById("matrix");
                            //document.movie1.SetMatrix("1,0,0, 0,1,0, 0,0,1");
                            var gmat = "7";
                            //gmat = document.movie1.GetHREF();
                            //gmat = document.movie1.GetMovieMatrix();
                            gmat = document.movie1.GetTrackCount();
                            alert('mat = ' + gmat); // + document.movie1.GetRectangle() );
                            // mymat.value = document.movie1.GetMatrix();
                        </script>
 
                        <li>size: <input id='size1' value='empty size' size='11' type='text' name="vevo2"/></li>
                        <li><input id='uri' value='emptyuri' size='70' type='text' name="vevo2"/></li>        
                        <li><input id='xfile' value='emptyfile' type='hidden' name="vevo"/></li>
                    </ul>
                </div>
            </div>
            
            <div id="tslider" class="tslider">
                <div id="vlce0" style="float: left">
                    <ul>
                        <li><b><? echo ($_SESSION[userid]); ?></b></li>
                        <li>
                            <script language="javascript" type="text/javascript">
                                var width = "<? echo $_SESSION[dwidth0] ?>";
                                var height = "<? echo $_SESSION[dheight0] ?>";
                                QT_WriteOBJECT('myoffline2.gif' , '100%', '50%', '', 'HREF', 'rtsp://184.72.239.149/vod/mp4:BigBuckBunny_175k.mov','autohref' ,'true', 'SCALE','TOFIT','target', 'myself','obj#id', 'movie0', 'emb#id', 'qtmovie_embed0', 'emb#name', 'movie0', 'postdomevents', 'true', 'enablejavascript', 'true');
                                // QT_WriteOBJECT('rtsp://184.72.239.149/vod/mp4:BigBuckBunny_175k.mov', width,height, '', 'obj#id', 'movie0', 'emb#id', 'qtmovie_embed0', 'emb#name', 'movie0', 'postdomevents', 'true', 'enablejavascript', 'true');
                            </script>
                        </li>
                        <p ID="loadStatus0">MOVIE LOADING...</p>
                        <li>
                            <a href="javascript:;" onclick='return vplay("vlc0")'>Play </a> 
                            <a href="javascript:;" onclick='return vpause("vlc0")'>Pause </a>
                            <a href="javascript:;" onclick='return vstop("vlc0")'>Stop </a>
                            <a href="javascript:;" onclick='return vmute("vlc0")'>Mute</a>                            
                        </li>                       
                        <li>size: <input id='size0' value='empty size' size='11' type='text' name="vevo2"/></li>
                        <li><input id='uri0' value='emptyuri' size='50' type='text' name="vevo2"/></li>                          
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
