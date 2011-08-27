<?
    if ((!isset($_POST["username"])) || (!isset($_POST["password"]))){
        header("Location: index.html");
        exit;
    }

    $MYSESSION1 = "";
    $MYSESSION2 = "";
    // Opens a connection to a MySQL server
    $mysqli = new mysqli("127.0.0.1", "xvdth", "12345", "xvdth");
    if (!$mysqli) {  
        die('Not connected : ' . mysqli_error($mysqli));
        header("Location: index.html");
        exit;
    } else {
        // Set the active MySQL database 
        // Select all the rows from table
        $query = "SELECT * FROM user where username = '".$_POST["username"]."' and password = '".$_POST["password"]."'";
        $result = mysqli_query($mysqli, $query);
        if (!$result) {  
            die('Invalid query: ' . mysqli_error($mysqli));
            header("Location: index.html");
            exit;
        } else {
            if (mysqli_num_rows($result) == 0){
                $result->close();
                header("Location: index.html");
                exit;
            } else{
                $row = @mysqli_fetch_assoc($result);
                $MYSESSION1 = $row['username']; // Starting a session. 
                $MYSESSION2 = $row['first'];    // Storing name in another session variable.  
                $result->close();
            }
        }
    }
    session_start();
?>
<!DOCTYPE html PUBLIC "-//WAPFORUM//DTD XHTML Mobile 1.2//EN" "http://www.openmobilealliance.org/tech/DTD/xhtml-mobile12.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
    <head>
		<meta http-equiv="Content-Type" content="application/xhtml+xml; charset=utf-8" />
		<meta http-equiv="cache-control" content="max-age=200" />
                <meta name="viewport" content="initial-scale=1.0, user-scalable=no" />
                
		<link href="landing.css" media="handheld, screen" rel="stylesheet" type="text/css" />
		<title>XVDTHLanding</title>
        <style type="text/css">
  		html { height: 100% }
  		body { height: 100%; margin: 0px; padding: 0px; font-family:courier}
  		#map_canvas { height: 100% }
	</style>

        <script type="text/javascript" src="http://maps.google.com/maps/api/js?sensor=true"></script>
        <script type="text/javascript" src="http://ajax.googleapis.com/ajax/libs/jquery/1.6.2/jquery.min.js"></script> 
        <!-- script src="jquery-1.6.2.min.js" type="text/javascript"></script -->
        
        <script type="text/javascript"> 
            
            function check(){
//            alert('check called');
            
                downloadUrl("checkupdate.php", function(data){
                    var xml = data.responseXML;
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
                        
                        var mybody  = document.getElementsByTagName("body")[0];
                        var mytable = mybody.getElementsByTagName("table")[0];
                        var mytbody = mytable.getElementsByTagName("tbody")[0];
                        var myrow;
                        var mycel;
                        var myelem;
                        var myelemimg;
                        var found;                        
                         
                        //alert('row.length ='+mytbody.rows.length)
                        for (j = 0; j < mytbody.rows.length; j++){
                            myrow = mytbody.getElementsByTagName("tr")[j];
                            mycel1 = myrow.getElementsByTagName("td")[1];                           
                            mycel = myrow.getElementsByTagName("td")[0];                                
                            myelem = mycel.childNodes[0]; 
                            myelemimg = myelem.childNodes[0];                           
                            found = 0;                            
                            for (i = 0; i < puser.length; i++) {
                                if (mycel1.innerText == member[i][0]) {
                                    found = 1;                                    
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
                                    break;
                                } 
                            }
                            
                            if (found == 0){
                                if (myelem.getAttribute("href") != null){
                                    myelem.removeAttribute("href");
                                }
                                if (myelemimg.getAttribute("src") != null){
                                    if (myelemimg.getAttribute("src") != "myonline2.gif"){
                                        myelemimg.setAttribute("src", "myonline2.gif");
                                    }
                                }
                            }
                        }
                    //]]>
      
                    var t = setTimeout("check()", 5000);
                });                 
            }
        
            function load(){
//                alert ('load called');
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
    
    <body onload="check();">  
        <?
            $_SESSION['userid'] = $MYSESSION1;
            $_SESSION['name'] = $MYSESSION2;
            $_SESSION['count'] = 0;
            $_SESSION['udec'] = 1;
            require "check.php"; 
        ?>
        
        <script type="text/javascript"> 
            $(document).ready(function(){
            });
                        
            window.onbeforeunload = function (e) {
                var uremove = document.getElementById("removeuser");                
                if (uremove.value == '1'){
                    
                } 
            };
        </script>
        
        <div class="mainwrapper">
	    <div id="header">
                <div id="header2">Neuron Light 2-Way Demo
                </div>
            </div>   

	    <div id="content" class="content">
		
                    <div id="contacts" style="width: 150px; padding-top:22px; float: right">
                            <hr/>
                            <p style="margin: 0px; text-align: center">
                            <p>Active Sessions</p>
                            <table>
                                <tbody>
                                    <tr>
                                        <td><a><img style="padding-top:4px" alt=""></img></a></td>
                                        <td>Tim</td>
                                    </tr>
                                    <tr>
                                        <td><a><img style="padding-top:4px" alt=""></img></a></td>
                                        <td>Joe</td>
                                    </tr>
                                    <tr>
                                        <td><a><img style="padding-top:4px" alt=""></img></a></td>
                                        <td>Sue</td>
                                    </tr>
                                    <tr>
                                        <td><a><img style="padding-top:4px" alt=""></img></a></td>
                                        <td>Eve</td>
                                    </tr>
                                </tbody>
                            </table>
                            </p>
                            
                                <hr/>
                                <a href="readysession.php"><button>Start Session</button></a>
		   	</div>
	    </div>

        <div  id="footer" style="float: right">
            <a href="http://localhost:8080"><button><b>Session Test</b></button></a>
            <a href="restartall.php"><button><b>Restart All</b></button></a>
            <a href="index.php"><button><b>Exit</b></button></a>
        </div>	
        </div>
    </body>
</html>
