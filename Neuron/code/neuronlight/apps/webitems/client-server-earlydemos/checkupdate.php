<?php
   // header( 'Location: http://192.168.1.105/join.html' );
   //header( "Location: join.html" );
   
//   header("Content-type: text/xml"); 
//   $node = $dom->createElement("update");  
//   $newnode = $parnode->appendChild($node);  
//   $newnode->setAttribute("newurl", "http://www.mozilla.org");
    session_start();
    require "check.php";
   
    // Start XML file, create parent node
    $dom = new DOMDocument("1.0");
    $node = $dom->createElement("participant");
    $parnode = $dom->appendChild($node); 

    header("Content-type: text/xml"); 
    // Opens a connection to a MySQL server
    //$mysqli= new mysqli ("192.168.46.107', $username, $password, database );
    $mysqli = new mysqli("127.0.0.1", "xvdth", "12345", "xvdth");
    //$mysqli = new mysqli("192.168.46.67", "xvdth", "12345", "xvdth");
    //$mysqli = new mysqli("localhost", "xvdth", "12345", "xvdth");
    //$mysqli = new mysqli("localhost", "xvdth", "12345", "xvdth");
    if (!$mysqli) {  
        die('Not connected : ' . mysqli_error());
    } else{
        // Set the active MySQL database 
        // Select all the rows from table
        $query = "SELECT * FROM user";
        $result = mysqli_query($mysqli, $query);
        if (!$result) {  
            die('Invalid query: ' . mysqli_error());
        } 

        // Iterate through the rows, adding XML nodes for each
        while ($row = @mysqli_fetch_assoc($result)){  
            // ADD TO XML DOCUMENT NODE  
            $node = $dom->createElement("participant");  
            $newnode = $parnode->appendChild($node);  
            $newnode->setAttribute("username", $row['username']);
            $newnode->setAttribute("ip", $row['ip']);  
            $newnode->setAttribute("online", $row['online']);
            $newnode->setAttribute("insession", $row['insession']);
            $newnode->setAttribute("width", $row['width']);
            $newnode->setAttribute("height", $row['height']);            
        } 
        $result->close();
    }
        //$node = $dom->createElement("participant");  
        //$newnode = $parnode->appendChild($node);  
        //$newnode->setAttribute("username", "Tom");
        //$newnode->setAttribute("ip", "192.168.46.100");  
        //$newnode->setAttribute("portv", "7000");
        //$newnode->setAttribute("porta", "7001");
    
    echo $dom->saveXML();   
?>
