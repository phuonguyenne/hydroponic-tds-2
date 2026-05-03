<?php
include 'config.php';

header('Content-Type: application/json; charset=utf-8');
header('Cache-Control: no-store, no-cache, must-revalidate, max-age=0');
header('Pragma: no-cache');

$result = $conn->query('SELECT * FROM sensor_data ORDER BY id DESC LIMIT 1');
$data = [];

if ($result && ($row = $result->fetch_assoc())) {
    $data[] = $row;
}

echo json_encode($data);
