<?
    session_start();
    require "check.php";
    $location = "Location: readysession.php";    

    // Opens a connection to a MySQL server
    $mysqli= new mysqli ("127.0.0.1", "xvdth", "12345", "xvdth" );
    
    if (!$mysqli) {  
        printf("failed");
        die('Not connected : ' . mysqli_error());
    } else {
        $sql = "insert into sessionrecord (username, ip, portv, porta) values ('$_SESSION[userid]', '".$_POST["cURL"]."', 4000, 4001)";
        $result = mysqli_query($mysqli, $sql);
        
        $sql = "update user set insession=1,width='$_SESSION[cwidth]',height='$_SESSION[cheight]' where username='$_SESSION[userid]'";
        $result = mysqli_query($mysqli, $sql);
        
        $_SESSION[aratio0] = $_SESSION[cheight] / $_SESSION[cwidth];
        $_SESSION[dheight0] = round($_SESSION[dwidth0] * $_SESSION[aratio0]);
                
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
            $_SESSION['count'] = $count;
            
            switch ($count){
                case 0:
                break;
            
                case 1:
                   $location = "Location: conference1.php";
                break;
            
                case 2:
                   $location = "Location: conference1a.php";
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