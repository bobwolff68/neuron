<?
    session_start();
    require "check.php";
    $location = "Location: readyjoin.php";    

    // Opens a connection to a MySQL server
    $mysqli= new mysqli ("127.0.0.1", "xvdth", "12345", "xvdth" );
    
    if (!$mysqli) {  
        printf("failed");
        die('Not connected : ' . mysqli_error());
    } else {
        $sql = "insert into sessionrecord (username, ip, portv, porta) values ('$_SESSION[userid]', '".$_POST["cURL"]."', 4000, 4001)";

        $result = mysqli_query($mysqli, $sql);
        
        $sql = "select count from sessioncount";
        $result = mysqli_query($mysqli, $sql);
        if (!$result) {  
            die('Invalid query: ' . mysqli_error());
        } else {
            $row = @mysqli_fetch_assoc($result);
            $count = $row['count'];
            $count++;
            
            $sql = "update sessioncount set count=$count";
            $result = mysqli_query($mysqli, $sql);

            switch ($count){
                case 0:
                break;
            
                case 1:
                   $location = "Location: conference1.php";
                break;
            
                case 2:
                   $location = "Location: conference1.php";
                break;
            
                case 3:
                    $location = "Location: conference2.php";
                break;            
            
                default:
                break;
            }
        }
        mysqli_close($mysqli);
     }
     header( $location );
?>