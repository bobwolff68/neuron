<?
    session_start();
    require "check.php";
    
    // Opens a connection to a MySQL server
    $mysqli= new mysqli ("127.0.0.1", "xvdth", "12345", "xvdth" );
    
    if (!$mysqli) {  
        printf("failed");
        die('Not connected : ' . mysqli_error());
    } else {
        $sql = "delete from sessionrecord where username='$_SESSION[userid]'";
        $result = mysqli_query($mysqli, $sql);
        
        $sql = "select count from sessioncount";
        $result = mysqli_query($mysqli, $sql);
        if (!$result) {  
            die('Invalid query: ' . mysqli_error());
        } else {
            $row = @mysqli_fetch_assoc($result);
            $count = $row['count'];
//            if ($_SESSION['count'] > $count){
            if ($count > 0){
                $count--;
            
                $sql = "update sessioncount set count=$count";
                $result = mysqli_query($mysqli, $sql);
            }
        }
        mysqli_close($mysqli);
     }
?>