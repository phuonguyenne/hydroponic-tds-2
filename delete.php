<?php
include "config.php";
$conn->query('TRUNCATE TABLE sensor_data');
$conn->query('DELETE FROM sensor_latest WHERE id = 1');
?>