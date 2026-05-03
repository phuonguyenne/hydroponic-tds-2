<?php
include "config.php";

// SET MODE (MySQL — không dùng mode.txt)
if (isset($_GET['mode'])) {
    $raw = trim((string) $_GET['mode']);
    $m = ($raw === 'truongthanh') ? 'truongthanh' : 'non';
    $st = $conn->prepare('UPDATE app_mode SET mode = ? WHERE id = 1');
    $st->bind_param('s', $m);
    $st->execute();
    echo 'OK';
}

// CLEAR DATA
if(isset($_GET['clear'])){
    $conn->query('TRUNCATE TABLE sensor_data');
    echo 'CLEARED';
}
?>