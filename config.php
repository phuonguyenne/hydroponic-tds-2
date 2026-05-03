<?php
$host = getenv('MYSQLHOST') ?: 'localhost';
$user = getenv('MYSQLUSER') ?: 'root';
$pass = getenv('MYSQLPASSWORD');
if ($pass === false) {
    $pass = '';
}
$db   = getenv('MYSQLDATABASE') ?: 'tds_system';
$port = (int) (getenv('MYSQLPORT') ?: 3306);

$conn = new mysqli($host, $user, $pass, $db, $port);

if ($conn->connect_error) {
    die('Kết nối lỗi: ' . $conn->connect_error);
}

$conn->set_charset('utf8mb4');

// Giờ Việt Nam: PHP + MySQL NOW() thống nhất (tránh lệch UTC trên Railway).
$appTz = getenv('APP_TIMEZONE') ?: 'Asia/Ho_Chi_Minh';
date_default_timezone_set($appTz);
$conn->query("SET time_zone = '+07:00'");

// Bảng 1 dòng: luôn cập nhật mỗi lần ESP POST (tổng quan poll nhanh). Lịch sử vẫn ở sensor_data (throttle 30s trong insert.php).
$conn->query(
    'CREATE TABLE IF NOT EXISTS sensor_latest (
  id INT NOT NULL PRIMARY KEY,
  `time` DATETIME NOT NULL,
  temp VARCHAR(32) NOT NULL,
  tds VARCHAR(32) NOT NULL,
  status VARCHAR(16) NOT NULL,
  mode VARCHAR(16) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4'
);

// Cột slot_ts: 1 ô 30 giây theo giờ VN (intdiv(time,30)) — insert.php dùng UPSERT để bảng không bị lệch 35–39s do chu kỳ ESP.
$tbl = $conn->query("SHOW TABLES LIKE 'sensor_data'");
if ($tbl && $tbl->num_rows > 0) {
    $col = $conn->query("SHOW COLUMNS FROM sensor_data LIKE 'slot_ts'");
    if ($col && $col->num_rows === 0) {
        $conn->query('ALTER TABLE sensor_data ADD COLUMN slot_ts INT UNSIGNED NULL DEFAULT NULL AFTER id');
    }
    $idx = $conn->query("SHOW INDEX FROM sensor_data WHERE Key_name = 'idx_sensor_data_slot'");
    if ($idx && $idx->num_rows === 0) {
        $conn->query('ALTER TABLE sensor_data ADD UNIQUE INDEX idx_sensor_data_slot (slot_ts)');
    }
}