<?
    session_start();
?>
<!DOCTYPE html PUBLIC "-//WAPFORUM//DTD XHTML Mobile 1.2//EN" "http://www.openmobilealliance.org/tech/DTD/xhtml-mobile12.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
    <head>
        <meta http-equiv="Content-Type" content="application/xhtml+xml; charset=utf-8" />
        <meta http-equiv="cache-control" content="max-age=200" />
        <meta name="viewport" content="initial-scale=1.0, user-scalable=no" />
                
	<link href="startsession.css" media="handheld, screen" rel="stylesheet" type="text/css" />
        <!-- link rel="stylesheet" type="text/css" href="http://revolunet.github.com/VLCcontrols/src/styles.css" / -->
                
	<title>XVDTH Video</title>
                        
        <style type="text/css">
  		html { height: 100% }
  		body { height: 100%; margin: 0px; padding: 0px; font-family:courier}
  		#map_canvas { height: 100% }
	</style>

        <script type="text/javascript" src="http://ajax.googleapis.com/ajax/libs/jquery/1.6.2/jquery.min.js"></script> 
        <script type="text/javascript" src="http://maps.google.com/maps/api/js?sensor=true"></script>

        <script type="text/javascript"> 
            var member = new Array();
        
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
        
            function load(){
                //alert('load called');
                //check ();
            }
        
            $(document).ready(function() {  
                //alert('doc ready called');
                //downloadUrl("redirect.php", function(data) {
                //alert('ready you are redirected');
                //});
            });        
        
            $(document.getElementsByTagName('div')[1]).ready(function() {
                
 	    });
        
        function doNothing(){   
        }

        google.maps.event.addDomListener(window, 'load', load);
        
            
    </script>
    
    </head> 
    
<body onload="load();">
    <?
        require "check.php"; 
    ?>
    
<div class="mainwrapper">
	    <div id="header">
                <div id="header2">Neuron Light 2-Way Demo
                </div>
            </div>  

	<div id="content" class="content">
            <div id="join" style="width: 200px; padding-left: 400px; padding-top:100px; float: left">
                <form name="input" action="startjoin.php" method="POST">
		    Your stream URL: <!-- input type="text" maxlength="90" size ="60" name="cURL" value="rtsp://192.168.46.100:8554/stream.sdp"/ -->
                                     <!-- input type="text" maxlength="90" size ="60" name="cURL" value="rtsp://184.72.239.149/vod/mp4:BigBuckBunny_175k.mov"/><br/ -->
                                     <input type="text" maxlength="90" size ="60" name="cURL" value="rtsp://192.168.46.81:8554/stream0"/><br/>
		    <input size ="60" type="submit" value="Submit"/>
                    <a href="landing.php"><input type="button" name="cancel" value="Cancel" /></a>
                </form>
            </div>		
	</div>

	<div id="footer">            
	</div>
		
</div>

</body>
</html>
