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

// Luôn cập nhật bản “mới nhất” cho tab Tổng quan (poll 3–5s).
$stmtLatest = $conn->prepare(
    'INSERT INTO sensor_latest (id, `time`, temp, tds, status, mode) VALUES (1, NOW(), ?, ?, ?, ?)
     ON DUPLICATE KEY UPDATE `time` = VALUES(`time`), temp = VALUES(temp), tds = VALUES(tds), status = VALUES(status), mode = VALUES(mode)'
);
$stmtLatest->bind_param('ssss', $tempStr, $tdsStr, $status, $mode);

if (!$stmtLatest->execute()) {
    http_response_code(500);
    echo 'ERR';
    exit;
}

// Lịch sử: tối đa 1 dòng / 30 giây để bảng/biểu đồ theo ngày không quá dày.
$insertHistory = true;
$resMax = $conn->query('SELECT MAX(`time`) AS last_t FROM sensor_data');
if ($resMax && ($rowMax = $resMax->fetch_assoc()) && !empty($rowMax['last_t'])) {
    $lastTs = strtotime($rowMax['last_t']);
    if ($lastTs !== false && (time() - $lastTs) < 30) {
        $insertHistory = false;
    }
}

if ($insertHistory) {
    $stmt = $conn->prepare(
        'INSERT INTO sensor_data (`time`, temp, tds, status, mode) VALUES (NOW(), ?, ?, ?, ?)'
    );
    $stmt->bind_param('ssss', $tempStr, $tdsStr, $status, $mode);

    if (!$stmt->execute()) {
        http_response_code(500);
        echo 'ERR';
        exit;
    }
}

echo 'OK';