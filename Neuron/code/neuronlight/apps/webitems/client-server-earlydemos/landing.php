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
		<link href="landing.css" media="handheld, screen" rel="stylesheet" type="text/css" />
		<title>XVDTHLanding</title>
		<meta name="viewport" content="initial-scale=1.0, user-scalable=no" />
		<style type="text/css">
  			html { height: 100% }
  			body { height: 100%; margin: 0px; padding: 0px }
  			#map_canvas { height: 100% }
		</style>	
    </head>
    
    <body>  
        <?
            $_SESSION['userid'] = $MYSESSION1;
            $_SESSION['name'] = $MYSESSION2;
            require "check.php"; 
        ?>
        
        <div class="mainwrapper">
	    <div id="header">
                <div id="header2">Neuron Light 2-Way Demo
                </div>
            </div>   

	    <div id="content" class="content">
		
                    <div id="contacts" style="width: 150px; padding-top:22px; float: right">
                            <hr/>
                            <p style="margin:0px; text-align: center">
                            <p>Active Sessions</p>
                            <table>
                                <tr>
                                    <td><a href="join.html"><button>Active</button></a></td>
                                    <td><a href="join.html">Tom</a></td>
                                </tr>
                                <tr>
                                    <td></td>
                                    <td>Joe</td>
                                </tr>
                                <tr>
                                    <td></td>
                                    <td>Sue</td>
                                </tr>
                                <tr>
                                    <td></td>
                                    <td>Ann</td>
                                </tr>
                            </table>
                            </p>
                            
                                <hr/>
                                <a href="startsession.html"><button>Start Session</button></a>
		   	</div>
		
	    </div>

	    <div id="footer">
            </div>	
        </div>
    </body>
</html>
