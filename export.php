<?php
include "config.php";

header("Content-Type: application/vnd.ms-excel");
header("Content-Disposition: attachment; filename=data.xls");

echo "Time\tTDS\tTemp\tStatus\n";

$result = $conn->query("SELECT * FROM sensor_data");

while($row = $result->fetch_assoc()){
    echo "{$row['time']}\t{$row['tds']}\t{$row['temp']}\t{$row['status']}\n";
}
?>