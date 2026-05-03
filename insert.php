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

$mode = 'non';
$r = $conn->query('SELECT mode FROM app_mode WHERE id = 1 LIMIT 1');
if ($r && ($row = $r->fetch_assoc())) {
    $t = trim((string) $row['mode']);
    if ($t === 'truongthanh' || $t === 'non') {
        $mode = $t;
    }
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
