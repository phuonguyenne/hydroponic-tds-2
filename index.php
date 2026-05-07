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
.header img{
height:80px;width:80px;object-fit:contain;
flex-shrink:0;min-width:80px;
}

.title{text-align:center;flex:1;min-width:0}

/* FIX SIZE NHỎ LẠI */
.title h2{font-size:24px;font-weight:bold;margin:5px}
.title h3{margin:5px}
.title p{
margin:5px;
font-weight:bold;
font-size:24px;
text-transform:uppercase;
}
.student-info{
display:inline-block;
margin:8px auto 0;
padding:10px 18px;
border:2px solid #2980b9;
border-radius:12px;
background:#f8fbff;
box-shadow:0 2px 6px rgba(41,128,185,0.15);
font-size:15px;
font-weight:600;
color:#2c3e50;
text-transform:none;
line-height:1.5;
text-align:left;
min-width:360px;
}
.student-info-title{
text-align:center;
font-weight:700;
color:#2980b9;
margin-bottom:4px;
}
.student-info-row{
display:flex;
gap:8px;
}
.student-info-label{
min-width:130px;
font-weight:700;
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

#dateToolbar{
display:none;
padding:12px;
text-align:center;
background:#dfe6e9;
border-bottom:1px solid #bdc3c7;
font-weight:600;
}
#dateToolbar input[type=date]{
padding:6px 10px;
border:1px solid #95a5a6;
border-radius:6px;
font-size:15px;
}
</style>
</head>

<body>

<div class="header">
<img src="/logo.png?v=3" width="80" height="80" alt="Logo trường">
<div class="title">
<h2>TRƯỜNG ĐẠI HỌC NÔNG LÂM THÀNH PHỐ HỒ CHÍ MINH</h2>
<h3>KHOA CƠ KHÍ CÔNG NGHỆ</h3>
<p>GIÁM SÁT NHIỆT ĐỘ DUNG DỊCH VÀ NỒNG ĐỘ DINH DƯỠNG TRÊN MÔ HÌNH TRỒNG RAU THỦY CANH</p>
<div class="student-info">
<div class="student-info-title">THÔNG TIN SINH VIÊN</div>
<div class="student-info-row"><span class="student-info-label">Họ và tên:</span><span>Phạm Thị Phương Uyên</span></div>
<div class="student-info-row"><span class="student-info-label">MSSV:</span><span>22138117</span></div>
<div class="student-info-row"><span class="student-info-label">Lớp:</span><span>DH22TD</span></div>
<div class="student-info-row"><span class="student-info-label">Email:</span><span>22138117@st.hcmuaf.edu.vn</span></div>
</div>
</div>
<img src="/logo_khoacokhi.png?v=3" width="80" height="80" alt="Logo khoa">
</div>

<div class="tabs">
<button onclick="openTab(0)">🏠 Tổng quan</button>
<button onclick="openTab(1)">📊 Biểu đồ</button>
<button onclick="openTab(2)">📋 Dữ liệu</button>
</div>

<div id="dateToolbar">
<label for="historyDate">Ngày xem dữ liệu &amp; biểu đồ: </label>
<input type="date" id="historyDate" title="Lọc theo ngày">
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
<th>Mode cây</th>
</tr>
</thead>
<tbody id="tableData"></tbody>
</table>

</div>

<script>

let chart1,chart2,chart3;
let mode="non";
let currentTab=0;
let overviewInFlight=false;
let historyInFlight=false;

function todayStr(){
return new Date().toISOString().slice(0,10);
}

function applyModeUI(m){
mode=(m==="truongthanh")?"truongthanh":"non";
document.querySelectorAll(".btn-mode").forEach(b=>b.classList.remove("active-btn"));
if(mode==="non") btnNon.classList.add("active-btn");
else btnTruong.classList.add("active-btn");
}

function syncModeFromServer(){
return fetch("get-mode.php").then(r=>r.text()).then(t=>{
t=t.trim();
if(t==="truongthanh"||t==="non") applyModeUI(t);
else applyModeUI("non");
}).catch(()=>applyModeUI("non"));
}

function openTab(i){
currentTab=i;
document.querySelectorAll(".tab").forEach(t=>t.classList.remove("active"));
document.querySelectorAll(".tab")[i].classList.add("active");
const bar=document.getElementById("dateToolbar");
if(bar) bar.style.display=(i===1||i===2)?"block":"none";
if(i===0) loadOverview();
if(i===1||i===2) loadHistory();
}

function setMode(m){
fetch("data.php?mode="+encodeURIComponent(m)).then(r=>{
if(!r.ok) throw new Error("mode");
return r.text();
}).then(()=>{
applyModeUI(m);
loadOverview();
if(currentTab===1||currentTab===2) loadHistory();
}).catch(()=>alert("Không lưu được chế độ. Kiểm tra máy chủ."));
}

function clearData(){
if(!confirm("Bạn có chắc muốn xóa toàn bộ dữ liệu?")) return;
fetch("data.php?clear=1").then(r=>{
if(!r.ok) throw new Error("clear");
return r.text();
}).then(()=>{
loadOverview();
loadHistory();
alert("Đã xóa dữ liệu");
}).catch(()=>alert("Xóa thất bại."));
}

function exportExcel(){window.location="export.php";}

function updateStatus(){
const now=new Date();
const sec=now.getHours()*3600+now.getMinutes()*60+now.getSeconds();
const START=6*3600;
const END_DAY=16*3600;
if(sec<START||sec>=END_DAY){
systemStatus.innerHTML="🌙 NGHỈ ĐÊM — tránh úng rễ";
return;
}
const offset=sec-START;
const cycle=offset%2400;
if(cycle<600){
systemStatus.innerHTML="💧 ĐANG TƯỚI (10 phút)";
}else{
systemStatus.innerHTML="🛑 ĐANG NGHỈ 30 PHÚT";
}
}

function applyWarnings(tdsVal,tempVal){
let min=(mode==="non")?500:700;
let max=(mode==="non")?700:900;
let t=parseFloat(tdsVal);
let tmp=parseFloat(tempVal);
if(isNaN(t)) t=0;
if(isNaN(tmp)) tmp=0;

if(t<min){
warnTDS.className="card low";
warnTDS.innerHTML="<div class='warn-title'>⚠️ TDS THẤP</div><div class='warn-text'>❌  Thiếu dinh dưỡng<br>📉 Ảnh hưởng => cây chậm lớn, lá nhỏ, rễ yếu<br>✅ Khuyến nghị => thêm dung dịch A+B từ từ</div>";
}else if(t>max){
warnTDS.className="card high";
warnTDS.innerHTML="<div class='warn-title'>⚠️ TDS CAO</div><div class='warn-text'>❌  Dung dịch quá đậm<br>📉 Ảnh hưởng => cháy rễ, cây sốc dinh dưỡng<br>✅ Khuyến nghị => thêm nước để pha loãng</div>";
}else{
warnTDS.className="card ok";
warnTDS.innerHTML="<div class='warn-title'>✅ TDS PHÙ HỢP</div><div class='warn-text'>Hệ thống đang cân bằng dinh dưỡng</div>";
}

if(tmp>30){
warnTemp.className="card high";
warnTemp.innerHTML="<div class='warn-title'>⚠️ NHIỆT CAO</div><div class='warn-text'>❌  Nhiệt dung dịch cao<br>📉 Ảnh hưởng => giảm oxy, hại rễ<br>✅ Khuyến nghị => làm mát dung dịch</div>";
}else if(tmp<18){
warnTemp.className="card low";
warnTemp.innerHTML="<div class='warn-title'>⚠️ NHIỆT THẤP</div><div class='warn-text'>❌  Nhiệt thấp<br>📉 Ảnh hưởng => cây hấp thụ kém<br>✅ Khuyến nghị => tăng nhiệt</div>";
}else{
warnTemp.className="card ok";
warnTemp.innerHTML="<div class='warn-title'>✅ NHIỆT ĐỘ PHÙ HỢP</div><div class='warn-text'>Nhiệt độ phù hợp cho cây</div>";
}
}

function loadOverview(){
if(currentTab!==0) return;
if(overviewInFlight) return;
overviewInFlight=true;
const url="get-latest.php?t="+Date.now();
fetch(url,{cache:"no-store",headers:{"Cache-Control":"no-cache"}})
.then(r=>{if(!r.ok) throw new Error("latest");return r.json();})
.then(data=>{
if(!data.length){
tds.innerHTML="--";
temp.innerHTML="--";
warnTDS.className="card ok";
warnTDS.innerHTML="<div class='warn-title'>Chưa có dữ liệu</div><div class='warn-text'>Đợi thiết bị gửi lần đầu.</div>";
warnTemp.className="card ok";
warnTemp.innerHTML="<div class='warn-title'>—</div><div class='warn-text'>—</div>";
return;
}
let d=data[0];
tds.innerHTML=d.tds;
temp.innerHTML=d.temp;
applyWarnings(d.tds,d.temp);
})
.catch(()=>{})
.finally(()=>{overviewInFlight=false;});
}

function loadHistory(){
if(currentTab!==1&&currentTab!==2) return;
if(historyInFlight) return;
historyInFlight=true;
let day=document.getElementById("historyDate");
let q=day&&day.value?("?date="+encodeURIComponent(day.value)):"?date="+encodeURIComponent(todayStr());
fetch("get-data.php"+q+"&t="+Date.now(),{cache:"no-store",headers:{"Cache-Control":"no-cache"}})
.then(r=>{if(!r.ok) throw new Error("hist");return r.json();})
.then(data=>{
let min=(mode==="non")?500:700;
let max=(mode==="non")?700:900;
let html="",labels=[],tdsArr=[],tempArr=[];

if(!data.length){
tableData.innerHTML="<tr><td colspan=\"5\">Chưa có dữ liệu cho ngày đã chọn.</td></tr>";
if(chart1){chart1.destroy();chart1=null;}
if(chart2){chart2.destroy();chart2=null;}
if(chart3){chart3.destroy();chart3=null;}
return;
}

/* Bảng: API trả id DESC — mới nhất trên cùng. Biểu đồ: cần thời gian tăng dần → đảo bản sao. */
data.forEach(x=>{
let cls="safe-row";
let status="SAFE (TDS OK + TEMP OK)";
let xt=parseFloat(x.tds)||0;
let xtmp=parseFloat(x.temp)||0;
if(xt<min||xtmp<18){cls="low-row";status="LOW (CẢNH BÁO)";}
else if(xt>max||xtmp>30){cls="high-row";status="HIGH (CẢNH BÁO)";}
let rowMode=(x.mode==="truongthanh")?"🌿 Trưởng thành":"🌱 Cây non";
html+=`<tr class="${cls}"><td>${x.time}</td><td>${x.tds}</td><td>${x.temp}</td><td>${status}</td><td>${rowMode}</td></tr>`;
});

data.slice().reverse().forEach(x=>{
labels.push(x.time);
tdsArr.push(parseFloat(x.tds)||0);
tempArr.push(parseFloat(x.temp)||0);
});

tableData.innerHTML=html;

if(chart1){chart1.destroy();chart2.destroy();chart3.destroy();}
chart1=new Chart(c1,{type:"line",data:{labels:labels,datasets:[{data:tempArr,borderColor:"red"}]},options:{plugins:{legend:{display:false}},responsive:true}});
chart2=new Chart(c2,{type:"line",data:{labels:labels,datasets:[{data:tdsArr,borderColor:"blue"}]},options:{plugins:{legend:{display:false}},responsive:true}});
chart3=new Chart(c3,{type:"line",data:{labels:labels,datasets:[{label:"Temp",data:tempArr,borderColor:"red"},{label:"TDS",data:tdsArr,borderColor:"blue"}]},options:{responsive:true}});
})
.catch(()=>{})
.finally(()=>{historyInFlight=false;});
}

document.getElementById("historyDate").value=todayStr();
document.getElementById("historyDate").addEventListener("change",loadHistory);

syncModeFromServer().then(()=>{
loadOverview();
});

// Tổng quan: poll 200ms khi đang ở tab 0 (gần Serial); không chồng request; không cache.
setInterval(loadOverview,200);
setInterval(updateStatus,1000);
// Tab Biểu đồ + Dữ liệu: làm mới nhanh (500ms) để khớp với tổng quan / Serial sau khi bỏ gom 30s.
setInterval(function(){
if(currentTab===1||currentTab===2) loadHistory();
},500);

</script>

</body>
</html>