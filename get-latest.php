<?php
include 'config.php';

header('Content-Type: application/json; charset=utf-8');

$result = $conn->query('SELECT * FROM sensor_latest WHERE id = 1 LIMIT 1');
$data = [];

if ($result && ($row = $result->fetch_assoc())) {
    $data[] = $row;
}

echo json_encode($data);
