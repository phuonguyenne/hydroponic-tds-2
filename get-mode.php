<?php
include 'config.php';
header('Content-Type: text/plain; charset=utf-8');

$mode = 'non';
$r = $conn->query('SELECT mode FROM app_mode WHERE id = 1 LIMIT 1');
if ($r && ($row = $r->fetch_assoc())) {
    $m = trim((string) $row['mode']);
    if ($m === 'truongthanh' || $m === 'non') {
        $mode = $m;
    }
}
echo $mode;
