<?php
include "config.php";

$sql = "SELECT * FROM sensor_data ORDER BY id DESC LIMIT 200";

$result = $conn->query($sql);

$data = [];

while($row = $result->fetch_assoc()){
    $data[] = $row;
}

echo json_encode($data);
?>