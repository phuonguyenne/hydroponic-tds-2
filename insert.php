<?php
include 'config.php';

$apiKey = getenv('UPLOAD_API_KEY');
if ($apiKey !== false && $apiKey !== '') {
    $sent = $_SERVER['HTTP_X_API_KEY'] ?? '';
    if (!hash_equals($apiKey, $sent)) {
        http_response_code(401);
        header('Content-Type: text/plain; charset=utf-8');
        echo 'Unauthorized';
        exit;
    }
}

$temp = isset($_POST['temp']) ? (float) $_POST['temp'] : 0;
$tds  = isset($_POST['tds']) ? (float) $_POST['tds'] : 0;

if (!file_exists('mode.txt')) {
    file_put_contents('mode.txt', 'non');
}

$mode = trim(file_get_contents('mode.txt'));
if ($mode === '') {
    $mode = 'non';
}

$status = 'OK';
if ($mode === 'non') {
    if ($tds < 500) {
        $status = 'LOW';
    }
    if ($tds > 700) {
        $status = 'HIGH';
    }
} else {
    if ($tds < 700) {
        $status = 'LOW';
    }
    if ($tds > 900) {
        $status = 'HIGH';
    }
}

$tempStr = (string) $temp;
$tdsStr = (string) $tds;
$nowStr = date('Y-m-d H:i:s');

// Mỗi lần ESP POST → một dòng mới (web + bảng đồng bộ với Serial, không gom 30s).
$stmt = $conn->prepare(
    'INSERT INTO sensor_data (`time`, temp, tds, status, mode) VALUES (?, ?, ?, ?, ?)'
);
$stmt->bind_param('sssss', $nowStr, $tempStr, $tdsStr, $status, $mode);

if (!$stmt->execute()) {
    http_response_code(500);
    echo 'ERR';
    exit;
}

echo 'OK';
