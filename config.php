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