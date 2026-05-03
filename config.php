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

// Giờ Việt Nam
$appTz = getenv('APP_TIMEZONE') ?: 'Asia/Ho_Chi_Minh';
date_default_timezone_set($appTz);
$conn->query("SET time_zone = '+07:00'");

$conn->query(
    'CREATE TABLE IF NOT EXISTS sensor_data (
  id INT AUTO_INCREMENT PRIMARY KEY,
  `time` DATETIME NOT NULL,
  temp VARCHAR(32) NOT NULL,
  tds VARCHAR(32) NOT NULL,
  status VARCHAR(16) NOT NULL,
  mode VARCHAR(16) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4'
);
