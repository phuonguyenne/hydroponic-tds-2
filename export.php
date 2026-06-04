<?php
include 'config.php';

header('Content-Type: application/vnd.ms-excel');
header('Cache-Control: no-store');

$date = isset($_GET['date']) ? trim((string) $_GET['date']) : '';
$filename = ($date !== '' && preg_match('/^\d{4}-\d{2}-\d{2}$/', $date))
    ? "data_{$date}.xls"
    : 'data.xls';
header("Content-Disposition: attachment; filename={$filename}");

echo "Time\tTDS\tTemp\tStatus\tMode\n";

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

if ($result) {
    while ($row = $result->fetch_assoc()) {
        echo "{$row['time']}\t{$row['tds']}\t{$row['temp']}\t{$row['status']}\t{$row['mode']}\n";
    }
}
