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