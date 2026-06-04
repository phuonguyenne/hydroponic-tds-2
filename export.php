<?php
include 'config.php';

header('Content-Type: text/tab-separated-values; charset=UTF-8');
header('Cache-Control: no-store');

$date = isset($_GET['date']) ? trim((string) $_GET['date']) : '';
$filename = ($date !== '' && preg_match('/^\d{4}-\d{2}-\d{2}$/', $date))
    ? "data_{$date}.xls"
    : 'data.xls';
header("Content-Disposition: attachment; filename={$filename}");

function export_second_of_day(string $time): ?int
{
    if (!preg_match('/(\d{1,2}):(\d{2}):(\d{2})/', $time, $m)) {
        return null;
    }
    return (int) $m[1] * 3600 + (int) $m[2] * 60 + (int) $m[3];
}

function export_cycle_phase(string $time): string
{
    $sec = export_second_of_day($time);
    if ($sec === null) {
        return 'night';
    }
    $start = 6 * 3600;
    $end = 16 * 3600 + 10 * 60;
    if ($sec < $start || $sec >= $end) {
        return 'night';
    }
    $cycle = ($sec - $start) % 2400;
    return $cycle < 600 ? 'dose' : 'rest';
}

function export_table_bucket_key(string $time, string $phase): string
{
    $dateHour = strlen($time) >= 13 ? substr($time, 0, 13) : '';
    $min = strlen($time) >= 16 ? (int) substr($time, 14, 2) : 0;
    if ($phase === 'night') {
        $slot = (int) (floor($min / 30) * 30);
        return $dateHour . ':' . str_pad((string) $slot, 2, '0', STR_PAD_LEFT);
    }
    if ($phase === 'rest') {
        $slot = (int) (floor($min / 5) * 5);
        return $dateHour . ':' . str_pad((string) $slot, 2, '0', STR_PAD_LEFT);
    }
    return substr($time, 0, 16);
}

function export_normalize_mode(?string $raw): string
{
    $m = strtolower(trim((string) $raw));
    return str_contains($m, 'truong') ? 'truongthanh' : 'non';
}

/** Chi chu ASCII cho Excel (khong icon, khong dau, tranh loi font). */
function export_display_status(string $phase, float $tds, float $temp, string $plantMode): string
{
    if ($phase === 'rest') {
        return 'Dang nghi 30 phut';
    }
    if ($phase === 'night') {
        return 'Nghi dem - tranh ung re';
    }
    $norm = export_normalize_mode($plantMode);
    $min = $norm === 'truongthanh' ? 700.0 : 500.0;
    $max = $norm === 'truongthanh' ? 900.0 : 700.0;
    if ($tds < $min || $temp < 18) {
        return 'LOW (canh bao)';
    }
    if ($tds > $max) {
        return 'HIGH (canh bao)';
    }
    if ($temp > 30) {
        return 'WARM (canh bao nhe)';
    }
    return 'SAFE (TDS OK + TEMP OK)';
}

function export_mode_label(?string $raw): string
{
    return export_normalize_mode($raw) === 'truongthanh' ? 'Cay truong thanh' : 'Cay non';
}

// BOM UTF-8 giup Excel doc tieng Viet; status/mode dung chu thuong ASCII.
echo "\xEF\xBB\xBF";
echo "Time\tTDS\tTemp\tStatus\tMode\n";

if ($date !== '' && preg_match('/^\d{4}-\d{2}-\d{2}$/', $date)) {
    $stmt = $conn->prepare(
        'SELECT * FROM sensor_data WHERE DATE(`time`) = ? ORDER BY id DESC'
    );
    $stmt->bind_param('s', $date);
    $stmt->execute();
    $result = $stmt->get_result();
} else {
    $result = $conn->query('SELECT * FROM sensor_data ORDER BY id DESC LIMIT 200');
}

$bucketSeen = [];

if ($result) {
    while ($row = $result->fetch_assoc()) {
        $time = (string) $row['time'];
        $phase = export_cycle_phase($time);
        $key = export_table_bucket_key($time, $phase);
        if (isset($bucketSeen[$key])) {
            continue;
        }
        $bucketSeen[$key] = true;

        $tds = (float) $row['tds'];
        $temp = (float) $row['temp'];
        $mode = (string) ($row['mode'] ?? 'non');
        $status = export_display_status($phase, $tds, $temp, $mode);
        $modeLabel = export_mode_label($mode);

        echo "{$time}\t{$row['tds']}\t{$row['temp']}\t{$status}\t{$modeLabel}\n";
    }
}
