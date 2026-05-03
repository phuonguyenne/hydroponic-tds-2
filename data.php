<?php
include "config.php";

// SET MODE
if(isset($_GET['mode'])){
    file_put_contents("mode.txt", $_GET['mode']);
    echo "OK";
}

// CLEAR DATA
if(isset($_GET['clear'])){
    $conn->query("TRUNCATE TABLE sensor_data");
    echo "CLEARED";
}
?>