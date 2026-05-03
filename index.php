<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<title>Hydroponic System</title>
<script src="https://cdn.jsdelivr.net/npm/chart.js"></script>

<style>
body{margin:0;font-family:Segoe UI;background:#eef2f7}

/* HEADER */
.header{
display:flex;align-items:center;justify-content:space-between;
padding:10px 40px;background:#fff;
box-shadow:0 2px 5px rgba(0,0,0,0.1)
}
.header img{height:80px;width:80px;object-fit:contain}

.title{text-align:center;flex:1}

/* FIX SIZE NHỎ LẠI */
.title h2{font-size:24px;font-weight:bold;margin:5px}
.title h3{margin:5px}
.title p{
margin:5px;
font-weight:bold;
font-size:24px;
text-transform:uppercase;
}

/* TAB */
.tabs{display:flex;background:#2c3e50}
.tabs button{
flex:1;padding:15px;border:none;
color:#fff;background:none;font-weight:bold;cursor:pointer
}
.tabs button:hover{background:#34495e}

.tab{display:none;padding:20px}
.active{display:block}

/* GRID */
.grid2{
display:grid;
grid-template-columns:1fr 1fr;
gap:20px;
max-width:900px;
margin:20px auto;
}

/* CARD */
.card{
background:#fff;
border-radius:12px;
padding:20px;
text-align:center;
box-shadow:0 3px 8px rgba(0,0,0,0.1);
min-height:160px;
display:flex;
flex-direction:column;
justify-content:center;
}

/* TITLE */
.card-title{
font-size:24px;
font-weight:bold;
color:#2c3e50;
margin-bottom:10px;
}

.card-title.temp{color:#e74c3c}

/* VALUE */
.value-big{
font-size:42px;
font-weight:bold;
color:#2980b9;
}

.unit{font-size:16px;color:#555}

/* WARNING */
.warn-title{
font-size:24px;
font-weight:bold;
margin-bottom:10px;
}

.warn-text{
font-size:15px;
line-height:1.6;
text-align:left;
}

.ok{
border:3px solid #2ecc71;
color:#2ecc71;
}

.low{
border:3px solid #f1c40f;
color:#f39c12;
animation:blink 1s infinite;
}

.high{
border:3px solid #e74c3c;
color:#e74c3c;
animation:blink 1s infinite;
}

@keyframes blink{
0%{box-shadow:0 0 5px}
50%{box-shadow:0 0 20px}
100%{box-shadow:0 0 5px}
}

/* BUTTON */
.btn{
padding:10px 20px;border:none;border-radius:6px;
cursor:pointer;font-weight:bold;margin:5px
}
.green{background:#27ae60;color:white}
.red{background:#e74c3c;color:white}
.active-btn{background:#f39c12 !important}

/* TABLE */
table{
width:95%;margin:auto;border-collapse:collapse;
background:white;
}
th{
background:#2980b9;color:white;padding:10px
}
td{
padding:10px;border:1px solid #ddd;text-align:center
}

.low-row{background:#fff3cd;}
.high-row{background:#f8d7da;}
.safe-row{background:#d4edda;}

/* CHART */
.chart-box{
background:white;
margin:20px auto;
padding:15px;
border-radius:10px;
box-shadow:0 2px 6px rgba(0,0,0,0.1);
width:90%;
text-align:center;
}

.chart-title{
font-weight:bold;
font-size:18px;
margin-bottom:10px;
}

/* STATUS */
.status{
text-align:center;
font-weight:bold;
color:#e67e22;
margin-top:10px;
font-size:18px;
}
</style>
</head>

<body>

<div class="header">
<img src="logo.svg" alt="Logo trường">
<div class="title">
<h2>TRƯỜNG ĐẠI HỌC NÔNG LÂM THÀNH PHỐ HỒ CHÍ MINH</h2>
<h3>KHOA CƠ KHÍ CÔNG NGHỆ</h3>
<p>GIÁM SÁT NHIỆT ĐỘ DUNG DỊCH VÀ NỒNG ĐỘ DINH DƯỠNG TRÊN MÔ HÌNH TRỒNG RAU THỦY CANH</p>
</div>
<img src="logo_khoacokhi.svg" alt="Logo khoa">
</div>

<div class="tabs">
<button onclick="openTab(0)">🏠 Tổng quan</button>
<button onclick="openTab(1)">📊 Biểu đồ</button>
<button onclick="openTab(2)">📋 Dữ liệu</button>
</div>

<!-- TAB 1 -->
<div class="tab active">

<div style="text-align:center">
<button id="btnNon" class="btn green btn-mode active-btn" onclick="setMode('non')">🌱 Cây non</button>
<button id="btnTruong" class="btn green btn-mode" onclick="setMode('truongthanh')">🌿 Cây trưởng thành</button>
</div>

<div class="status" id="systemStatus"></div>

<div class="grid2">
<div class="card">
<div class="card-title">💧 DINH DƯỠNG</div>
<div><span id="tds" class="value-big">--</span> <span class="unit">ppm</span></div>
</div>

<div class="card">
<div class="card-title temp">🌡 NHIỆT ĐỘ</div>
<div><span id="temp" class="value-big">--</span> <span class="unit">°C</span></div>
</div>
</div>

<div class="grid2">
<div id="warnTDS" class="card ok"></div>
<div id="warnTemp" class="card ok"></div>
</div>

</div>

<!-- TAB 2 -->
<div class="tab">
<div class="chart-box">
<div class="chart-title">📈 Biểu đồ Nhiệt độ theo Thời gian</div>
<canvas id="c1"></canvas>
</div>

<div class="chart-box">
<div class="chart-title">📊 Biểu đồ Nồng độ dinh dưỡng theo thời gian</div>
<canvas id="c2"></canvas>
</div>

<div class="chart-box">
<div class="chart-title">📊 Biểu đồ Nồng độ dinh dưỡng theo Nhiệt độ</div>
<canvas id="c3"></canvas>
</div>
</div>

<!-- TAB 3 -->
<div class="tab">

<div style="text-align:center">
<button class="btn red" onclick="clearData()">🗑 Xóa dữ liệu</button>
<button class="btn green" onclick="exportExcel()">📥 Xuất Excel</button>
</div>

<table>
<thead>
<tr>
<th>Time</th>
<th>TDS</th>
<th>Temp</th>
<th>Status</th>
</tr>
</thead>
<tbody id="tableData"></tbody>
</table>

</div>

<script>

let chart1,chart2,chart3;
let mode="non";

function openTab(i){
document.querySelectorAll(".tab").forEach(t=>t.classList.remove("active"));
document.querySelectorAll(".tab")[i].classList.add("active");
}

function setMode(m){
mode=m;
fetch("data.php?mode="+m);

document.querySelectorAll(".btn-mode").forEach(b=>b.classList.remove("active-btn"));
if(m=="non") btnNon.classList.add("active-btn");
else btnTruong.classList.add("active-btn");
}

function clearData(){
if(confirm("Bạn có chắc muốn xóa toàn bộ dữ liệu?")){
fetch("data.php?clear=1").then(()=>alert("Đã xóa dữ liệu"));
}
}

function exportExcel(){window.location="export.php";}

/* ===== STATUS 3 TRẠNG THÁI ===== */
function updateStatus(){
let now=new Date();
let h=now.getHours();
let m=now.getMinutes();
let s=now.getSeconds();

let total=h*3600+m*60+s;
let offset=total-6*3600;

if(offset<0){
systemStatus.innerHTML="🌙 ĐANG NGHỈ BAN ĐÊM (tránh úng rễ)";
return;
}

let cycle=offset%2400;

if(cycle<600){
systemStatus.innerHTML="💧 ĐANG TƯỚI (10 phút)";
}else{
systemStatus.innerHTML="🛑 ĐANG NGHỈ (30 phút)";
}
}

function loadData(){
fetch("get-data.php")
.then(r=>r.json())
.then(data=>{

if(!data.length) return;

let d=data[0];

tds.innerHTML=d.tds;
temp.innerHTML=d.temp;

let min=(mode=="non")?500:700;
let max=(mode=="non")?700:900;

/* ===== CẢNH BÁO CHUẨN ===== */
if(d.tds<min){
warnTDS.className="card low";
warnTDS.innerHTML="<div class='warn-title'>⚠️ TDS THẤP</div><div class='warn-text'>❌  Thiếu dinh dưỡng<br>📉 Ảnh hưởng => cây chậm lớn, lá nhỏ, rễ yếu<br>✅ Khuyến nghị => thêm dung dịch A+B từ từ</div>";
}
else if(d.tds>max){
warnTDS.className="card high";
warnTDS.innerHTML="<div class='warn-title'>⚠️ TDS CAO</div><div class='warn-text'>❌  Dung dịch quá đậm<br>📉 Ảnh hưởng => cháy rễ, cây sốc dinh dưỡng<br>✅ Khuyến nghị => thêm nước để pha loãng</div>";
}
else{
warnTDS.className="card ok";
warnTDS.innerHTML="<div class='warn-title'>✅ TDS PHÙ HỢP</div><div class='warn-text'>Hệ thống đang cân bằng dinh dưỡng</div>";
}

if(d.temp>30){
warnTemp.className="card high";
warnTemp.innerHTML="<div class='warn-title'>⚠️ NHIỆT CAO</div><div class='warn-text'>❌  Nhiệt dung dịch cao<br>📉 Ảnh hưởng => giảm oxy, hại rễ<br>✅ Khuyến nghị => làm mát dung dịch</div>";
}
else if(d.temp<18){
warnTemp.className="card low";
warnTemp.innerHTML="<div class='warn-title'>⚠️ NHIỆT THẤP</div><div class='warn-text'>❌  Nhiệt thấp<br>📉 Ảnh hưởng => cây hấp thụ kém<br>✅ Khuyến nghị => tăng nhiệt</div>";
}
else{
warnTemp.className="card ok";
warnTemp.innerHTML="<div class='warn-title'>✅ NHIỆT ĐỘ PHÙ HỢP</div><div class='warn-text'>Nhiệt độ phù hợp cho cây</div>";
}

/* TABLE */
let html="",labels=[],tdsArr=[],tempArr=[];

data.forEach(x=>{

labels.push(x.time);
tdsArr.push(x.tds);
tempArr.push(x.temp);

let cls="safe-row";
let status="SAFE (TDS OK + TEMP OK)";

if(x.tds<min || x.temp<18){
cls="low-row";
status="LOW (CẢNH BÁO)";
}
else if(x.tds>max || x.temp>30){
cls="high-row";
status="HIGH (CẢNH BÁO)";
}

html+=`<tr class="${cls}">
<td>${x.time}</td>
<td>${x.tds}</td>
<td>${x.temp}</td>
<td>${status}</td>
</tr>`;
});

tableData.innerHTML=html;

/* CHART */
if(chart1){chart1.destroy();chart2.destroy();chart3.destroy();}

chart1=new Chart(c1,{type:'line',data:{labels:labels,datasets:[{data:tempArr,borderColor:'red'}]},options:{plugins:{legend:{display:false}}}});
chart2=new Chart(c2,{type:'line',data:{labels:labels,datasets:[{data:tdsArr,borderColor:'blue'}]},options:{plugins:{legend:{display:false}}}});
chart3=new Chart(c3,{type:'line',data:{labels:labels,datasets:[{label:'Temp',data:tempArr,borderColor:'red'},{label:'TDS',data:tdsArr,borderColor:'blue'}]}});

});
}

setInterval(loadData,2000);
setInterval(updateStatus,1000);

</script>

</body>
</html>