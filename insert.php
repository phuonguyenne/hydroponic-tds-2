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
// Giờ theo PHP (đã set timezone VN trong config) — không phụ thuộc NOW() của MySQL.
$nowStr = date('Y-m-d H:i:s');

// Luôn cập nhật bản “mới nhất” cho tab Tổng quan (poll nhanh).
$stmtLatest = $conn->prepare(
    'INSERT INTO sensor_latest (id, `time`, temp, tds, status, mode) VALUES (1, ?, ?, ?, ?, ?)
     ON DUPLICATE KEY UPDATE `time` = VALUES(`time`), temp = VALUES(temp), tds = VALUES(tds), status = VALUES(status), mode = VALUES(mode)'
);
$stmtLatest->bind_param('sssss', $nowStr, $tempStr, $tdsStr, $status, $mode);

if (!$stmtLatest->execute()) {
    http_response_code(500);
    echo 'ERR';
    exit;
}

// Lịch sử: đúng 1 dòng / 30s theo “ô” thời gian VN (slot_ts). Cùng ô = cập nhật dòng đó (không chờ POST đúng 30s sau lần trước).
$slot = intdiv((int) time(), 30);
$stmtHist = $conn->prepare(
    'INSERT INTO sensor_data (slot_ts, `time`, temp, tds, status, mode) VALUES (?, ?, ?, ?, ?, ?)
     ON DUPLICATE KEY UPDATE `time` = VALUES(`time`), temp = VALUES(temp), tds = VALUES(tds), status = VALUES(status), mode = VALUES(mode)'
);
$stmtHist->bind_param('isssss', $slot, $nowStr, $tempStr, $tdsStr, $status, $mode);

if (!$stmtHist->execute()) {
    http_response_code(500);
    echo 'ERR';
    exit;
}

echo 'OK';