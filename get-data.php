<?php
include 'config.php';

header('Content-Type: application/json; charset=utf-8');
header('Cache-Control: no-store, no-cache, must-revalidate, max-age=0');
header('Pragma: no-cache');

$date = isset($_GET['date']) ? trim((string) $_GET['date']) : '';

if ($date !== '' && preg_match('/^\d{4}-\d{2}-\d{2}$/', $date)) {
    $stmt = $conn->prepare(
        'SELECT * FROM sensor_data WHERE DATE(`time`) = ? ORDER BY id DESC'
    );
    $stmt->bind_param('s', $date);
    $stmt->execute();
    $result = $stmt->get_result();
} else {
    $result = $conn->query('SELECT * FROM sensor_data ORDER BY id DESC LIMIT 200');
}

$data = [];

if ($result) {
    while ($row = $result->fetch_assoc()) {
        $data[] = $row;
    }
}

echo json_encode($data);
