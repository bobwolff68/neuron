<?php
    // Start XML file, create parent node
    $dom = new DOMDocument("1.0");
    $node = $dom->createElement("participant");
    $parnode = $dom->appendChild($node); 

    header("Content-type: text/xml"); 
    // ADD TO XML DOCUMENT NODE  
    $node = $dom->createElement("capture");  
    $newnode = $parnode->appendChild($node);  
    $newnode->setAttribute("captured", "You've been captured!");            
   
    echo $dom->saveXML();   
?>
