<?php  
    // Opens a connection to a MySQL server
    $mysqli= new mysqli ("127.0.0.1", "xvdth", "12345", "xvdth" );
    
    if (!$mysqli) {  
        printf("failed");
        die('Not connected : ' . mysqli_error());
    } else {
        $sql = "insert into sessionrecord (username, ip, portv, porta) values ('Tom', '".$_POST["cURL"]."', 4000, 4001)";
        $result = mysqli_query($mysqli, $sql);
        
//        if($result == TRUE){
//            echo "A record has been inserted";
//        } else {
//            printf ("insert did not work");
//        }
        
        mysqli_close($mysqli);
    }
    header( "Location: conference1.html" );
?>
