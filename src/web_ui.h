// Single page web interface served from flash. Self-contained (no CDN) so it also works on the
// setup access point, where there is no internet access.
#pragma once

#include <Arduino.h>

static const char WEB_UI_HTML[] PROGMEM = R"rawliteral(<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Micronova Controller</title>
<style>
:root{--bg:#f4f5f7;--card:#fff;--fg:#1d2330;--muted:#6b7280;--line:#e2e5ea;--accent:#e8590c;--accent-fg:#fff;--ok:#2f9e44;--warn:#e67700;--err:#c92a2a;--input:#fff}
@media (prefers-color-scheme:dark){:root{--bg:#14171c;--card:#1d2128;--fg:#e6e8eb;--muted:#9aa1ab;--line:#2d333c;--accent:#ff7a2e;--accent-fg:#14171c;--ok:#51cf66;--warn:#fcc419;--err:#ff6b6b;--input:#14171c}}
*{box-sizing:border-box}
body{margin:0;font:15px/1.45 system-ui,-apple-system,Segoe UI,Roboto,sans-serif;background:var(--bg);color:var(--fg)}
header{background:var(--card);border-bottom:1px solid var(--line);padding:12px 16px;display:flex;align-items:center;gap:12px;flex-wrap:wrap}
header h1{font-size:17px;margin:0;flex:1;white-space:nowrap}
header h1 span{color:var(--accent)}
nav{display:flex;gap:4px;overflow-x:auto;padding:0 16px;background:var(--card);border-bottom:1px solid var(--line)}
nav button{background:none;border:0;border-bottom:2px solid transparent;color:var(--muted);padding:10px 12px;font:inherit;cursor:pointer;white-space:nowrap}
nav button.on{color:var(--fg);border-color:var(--accent);font-weight:600}
main{max-width:980px;margin:0 auto;padding:16px}
section{display:none}section.on{display:block}
.card{background:var(--card);border:1px solid var(--line);border-radius:10px;padding:16px;margin-bottom:16px}
.card h2{font-size:15px;margin:0 0 12px}
.grid{display:grid;grid-template-columns:repeat(auto-fill,minmax(160px,1fr));gap:12px}
.metric{background:var(--card);border:1px solid var(--line);border-radius:10px;padding:12px}
.metric .l{color:var(--muted);font-size:13px}
.metric .v{font-size:24px;font-weight:600;margin-top:2px;font-variant-numeric:tabular-nums}
.metric .v small{font-size:14px;color:var(--muted);font-weight:400}
.metric.stale .v{color:var(--muted)}
.pills{display:flex;gap:8px;flex-wrap:wrap;font-size:13px}
.pill{border:1px solid var(--line);border-radius:999px;padding:2px 10px;color:var(--muted)}
.pill b{font-weight:600}
.dot{display:inline-block;width:8px;height:8px;border-radius:50%;margin-right:5px;background:var(--muted)}
.dot.ok{background:var(--ok)}.dot.warn{background:var(--warn)}.dot.err{background:var(--err)}
.state{display:flex;align-items:center;gap:16px;flex-wrap:wrap}
.state .big{font-size:26px;font-weight:700;flex:1;min-width:180px}
.row{display:flex;gap:8px;flex-wrap:wrap;align-items:end}
.row>label{display:flex;flex-direction:column;gap:4px;font-size:13px;color:var(--muted)}
input,select{font:inherit;color:var(--fg);background:var(--input);border:1px solid var(--line);border-radius:7px;padding:7px 9px;min-width:0}
input[type=checkbox]{width:18px;height:18px;accent-color:var(--accent)}
.chk{flex-direction:row!important;align-items:center;color:var(--fg)!important;font-size:15px!important;padding-bottom:8px}
button.b{font:inherit;border:1px solid var(--line);background:var(--card);color:var(--fg);border-radius:7px;padding:7px 14px;cursor:pointer}
button.b:hover{border-color:var(--accent)}
button.p{background:var(--accent);border-color:var(--accent);color:var(--accent-fg);font-weight:600}
button.d{color:var(--err)}
button:disabled{opacity:.5;cursor:default}
.muted{color:var(--muted);font-size:13px}
.msg{margin-top:10px;font-size:14px;min-height:1em}
.msg.err{color:var(--err)}.msg.ok{color:var(--ok)}
.banner{background:var(--accent);color:var(--accent-fg);border-radius:10px;padding:12px 16px;margin-bottom:16px;display:none}
.tw{overflow-x:auto}
table{border-collapse:collapse;width:100%;font-size:14px;font-variant-numeric:tabular-nums}
th,td{text-align:left;padding:6px 8px;border-bottom:1px solid var(--line);white-space:nowrap}
th{color:var(--muted);font-weight:500;font-size:13px}
td input,td select{width:84px;padding:4px 6px}
td select{width:100px}
tr.ch td{background:color-mix(in srgb,var(--accent) 14%,transparent)}
.bar{height:6px;background:var(--line);border-radius:3px;overflow:hidden;margin:10px 0}
.bar div{height:100%;background:var(--accent);width:0}
pre{background:var(--bg);border:1px solid var(--line);border-radius:7px;padding:10px;max-height:220px;overflow:auto;font-size:12px;margin:10px 0 0;white-space:pre-wrap}
.nets{display:flex;flex-direction:column;gap:4px;margin:8px 0;max-height:200px;overflow:auto}
.nets button{text-align:left;display:flex;justify-content:space-between}
.two{display:grid;grid-template-columns:repeat(auto-fit,minmax(300px,1fr));gap:16px}
.two .card{margin:0}
</style>
</head>
<body>
<header><h1><span>&#128293;</span> Micronova Controller</h1><div class="pills" id="pills"></div></header>
<nav id="nav">
<button data-t="dash" class="on">Dashboard</button><button data-t="tests">Tests</button><button data-t="addr">Addresses</button><button data-t="settings">Settings</button>
</nav>
<main>
<div class="banner" id="banner"></div>

<section id="dash" class="on">
<div class="card">
<div class="state"><div><div class="muted">Stove state</div><div class="big" id="stateName">&ndash;</div></div>
<div class="row"><button class="b p" onclick="cmd('ON')">Turn on</button><button class="b" onclick="cmd('OFF')">Turn off</button><button class="b d" onclick="forceOff()">Force off</button></div></div>
<div class="row" style="margin-top:14px">
<label>Temperature setpoint (&deg;C)<input id="tset" type="number" step="0.5" min="5" max="40" style="width:110px"></label><button class="b" onclick="setTemp()">Set</button>
<label style="margin-left:12px">Power<select id="pset"><option>1</option><option>2</option><option>3</option><option>4</option><option>5</option></select></label><button class="b" onclick="setPower()">Set</button>
<span style="flex:1"></span><button class="b" onclick="pollNow(this)">Refresh now</button>
</div>
<div class="msg" id="dashMsg"></div>
</div>
<div class="grid" id="metrics"></div>
<p class="muted" id="pollInfo"></p>
</section>

<section id="tests">
<div class="two">
<div class="card"><h2>Read every metric</h2><p class="muted">Reads all configured addresses once and shows the raw reply.</p>
<button class="b p" onclick="readAll(this)">Run</button><div class="tw"><table id="raTable"></table></div></div>
<div class="card"><h2>On / off cycle</h2><p class="muted">Sends ON, waits, then sends OFF. The stove will really start its ignition sequence.</p>
<div class="row"><label>Seconds before OFF<input id="ooSec" type="number" value="2" min="1" max="600" style="width:100px"></label><button class="b p" onclick="onOff()">Start</button></div></div>
</div>
<div class="card" style="margin-top:16px"><h2>Address scan</h2>
<p class="muted">Reads every address of the range (about 0.2 s each). Scan once, change something on the stove (temperature, power, state), then scan again: changed values are highlighted, which helps finding the right address.</p>
<div class="row">
<label>Memory<select id="scMem"><option value="0">RAM</option><option value="1">EEPROM</option></select></label>
<label>From<input id="scStart" value="0x00" style="width:80px"></label><label>To<input id="scEnd" value="0xFF" style="width:80px"></label>
<button class="b p" id="scBtn" onclick="scan()">Start scan</button><button class="b" onclick="stopTest()">Stop</button>
<label class="chk"><input type="checkbox" id="scChanged" onchange="renderScan()"> Only changed</label>
</div>
<div class="bar"><div id="scBar"></div></div><div class="muted" id="scInfo"></div>
<div class="tw" style="max-height:420px;overflow:auto"><table id="scTable"></table></div>
</div>
<div class="two">
<div class="card"><h2>Read one address</h2>
<div class="row"><label>Memory<select id="rdMem"><option value="0">RAM</option><option value="1">EEPROM</option></select></label><label>Address<input id="rdAddr" value="0x21" style="width:80px"></label><button class="b p" onclick="readOne()">Read</button></div>
<div class="msg" id="rdMsg"></div></div>
<div class="card"><h2>Write one address</h2><p class="muted">Raw write, use with care: a wrong EEPROM address can change the stove configuration.</p>
<div class="row"><label>Memory<select id="wrMem"><option value="0">RAM</option><option value="1">EEPROM</option></select></label><label>Address<input id="wrAddr" style="width:80px"></label><label>Value<input id="wrVal" style="width:80px"></label><button class="b d" onclick="writeOne()">Write</button></div>
<div class="msg" id="wrMsg"></div></div>
</div>
<div class="card" style="margin-top:16px"><h2>Test log</h2><pre id="log">No test run yet.</pre></div>
</section>

<section id="addr">
<div class="card"><h2>Metric addresses</h2>
<p class="muted">If a value is wrong or missing on your stove, change its address here (use the scan in Tests to find it). <b>Test</b> reads the address without saving. Value = raw / divisor. Flame power is mapped from 0&ndash;16 to 10&ndash;100&nbsp;%.</p>
<div class="tw"><table><thead><tr><th>Metric</th><th>Current</th><th>Address</th><th>Memory</th><th>Divisor</th><th>On</th><th></th></tr></thead><tbody id="addrBody"></tbody></table></div>
<div class="row" style="margin-top:12px"><button class="b" onclick="resetAll()">Restore all defaults</button></div>
<div class="msg" id="addrMsg"></div></div>
</section>

<section id="settings">
<div class="two">
<div class="card"><h2>WiFi</h2>
<div class="row"><button class="b" onclick="wifiScan(this)">Scan networks</button></div>
<div class="nets" id="nets"></div>
<div class="row"><label style="flex:1">Network (SSID)<input id="wSsid" maxlength="32"></label></div>
<div class="row" style="margin-top:8px"><label style="flex:1">Password<input id="wPass" type="password" maxlength="64" placeholder=""></label></div>
<div class="row" style="margin-top:12px"><button class="b p" onclick="saveWifi()">Save and connect</button></div>
<div class="msg" id="wMsg"></div></div>
<div class="card"><h2>MQTT</h2>
<div class="row"><label style="flex:1">Server (empty = disabled)<input id="mSrv" maxlength="64"></label><label>Port<input id="mPort" type="number" style="width:90px"></label></div>
<div class="row" style="margin-top:8px"><label style="flex:1">User<input id="mUser" maxlength="32"></label><label style="flex:1">Password<input id="mPass" type="password" maxlength="64"></label></div>
<div class="row" style="margin-top:8px"><label style="flex:1">Topic prefix<input id="mTopic" maxlength="32"></label><label class="chk"><input type="checkbox" id="mClear"> Clear password</label></div>
<div class="row" style="margin-top:12px"><button class="b p" onclick="saveMqtt()">Save</button></div>
<div class="msg" id="mMsg"></div></div>
<div class="card"><h2>General</h2>
<div class="row"><label style="flex:1">Hostname<input id="gHost" maxlength="32"></label><label>Poll every (s)<input id="gPoll" type="number" min="5" max="3600" style="width:90px"></label></div>
<div class="row" style="margin-top:8px"><label class="chk"><input type="checkbox" id="gHydro"> Hydro stove (water temperature and pressure)</label></div>
<div class="row" style="margin-top:8px"><label style="flex:1">Web / OTA user<input id="gUser" maxlength="32"></label><label style="flex:1">New password<input id="gPass" type="password" maxlength="64"></label></div>
<div class="row" style="margin-top:8px"><label class="chk"><input type="checkbox" id="gNoAuth"> No password (anyone on the network can control the stove)</label></div>
<div class="row" style="margin-top:12px"><button class="b p" onclick="saveGeneral()">Save</button></div>
<div class="msg" id="gMsg"></div></div>
<div class="card"><h2>System</h2><p class="muted" id="sysInfo"></p>
<div class="row"><button class="b" onclick="sys('reboot','Restart the controller?')">Restart</button><button class="b d" onclick="sys('wifi/forget','Erase the WiFi credentials? The controller will open its setup access point.')">Forget WiFi</button><button class="b d" onclick="sys('factory-reset','Restore every setting (WiFi, MQTT, addresses) to factory defaults?')">Factory reset</button></div>
<div class="msg" id="sysMsg"></div></div>
</div>
</section>
</main>
<script>
const $=id=>document.getElementById(id);
let S=null,prevScan=null,lastScan=null,testTimer=null,settingsLoaded=false;
const esc=s=>String(s).replace(/[&<>"']/g,c=>({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'}[c]));
const hex=n=>'0x'+(n<16?'0':'')+n.toString(16).toUpperCase();
function msg(id,t,ok){const e=$(id);e.textContent=t||'';e.className='msg '+(ok===false?'err':ok?'ok':'')}
async function api(path,data,method){
  const o={method:method||(data?'POST':'GET')};
  if(data){o.body=new URLSearchParams(data)}else if(o.method==='POST'){o.body=''}
  const r=await fetch('/api/'+path,o);let j={};try{j=await r.json()}catch(e){}
  if(!r.ok)throw new Error(j.message||('HTTP '+r.status));return j}
async function run(id,fn){try{const j=await fn();if(j&&j.message)msg(id,j.message,j.ok!==false);return j}catch(e){msg(id,e.message,false)}}

document.querySelectorAll('#nav button').forEach(b=>b.onclick=()=>tab(b.dataset.t));
function tab(t){document.querySelectorAll('#nav button').forEach(b=>b.classList.toggle('on',b.dataset.t===t));
  document.querySelectorAll('section').forEach(s=>s.classList.toggle('on',s.id===t));
  if(t==='settings')loadSettings();if(t==='tests')testStatus();if(t==='addr')renderAddr(true)}

async function status(){
  try{S=await api('status')}catch(e){$('pills').innerHTML='<span class="pill"><span class="dot err"></span>Offline</span>';return}
  const n=S.net,m=S.mqtt;
  let p='<span class="pill"><span class="dot '+(n.mode==='connected'?'ok':n.mode==='setup'?'warn':'err')+'"></span>WiFi <b>'+(n.mode==='connected'?esc(n.ssid)+' '+n.rssi+' dBm':n.mode)+'</b></span>';
  p+='<span class="pill"><span class="dot '+(!m.enabled?'':m.connected?'ok':'err')+'"></span>MQTT <b>'+(!m.enabled?'off':m.connected?'ok':'error '+m.state)+'</b></span>';
  if(n.ap)p+='<span class="pill"><span class="dot warn"></span>AP <b>'+esc(n.apSsid)+'</b></span>';
  $('pills').innerHTML=p;
  const b=$('banner');
  if(n.mode==='setup'){b.style.display='block';b.innerHTML='<b>Setup mode.</b> Choose your WiFi network below; the controller will restart and join it.'}
  else if(n.ap){b.style.display='block';b.innerHTML='Could not connect to <b>'+esc(n.ssid)+'</b>. The setup access point is open while it keeps retrying; check the WiFi settings.'}
  else b.style.display='none';
  $('stateName').textContent=S.stove.state<0?'Unknown (no reply yet)':S.stove.stateName;
  let g='';
  S.metrics.forEach(x=>{if(!x.active||x.key==='state')return;
    g+='<div class="metric'+(x.valid?'':' stale')+'"><div class="l">'+esc(x.label)+'</div><div class="v">'+(x.valid?esc(x.value)+' <small>'+esc(x.unit)+'</small>':'&ndash;')+'</div></div>'});
  $('metrics').innerHTML=g;
  const ts=S.metrics.find(x=>x.key==='tempset'),ps=S.metrics.find(x=>x.key==='stovepower');
  if(ts&&ts.valid&&document.activeElement!==$('tset'))$('tset').value=parseFloat(ts.value);
  if(ps&&ps.valid&&document.activeElement!==$('pset'))$('pset').value=ps.value;
  $('pollInfo').textContent=(S.stove.lastPoll<0?'Not polled yet':'Last read '+S.stove.lastPoll+' s ago')+' · '+S.stove.readOk+' good replies, '+S.stove.readErrors+' errors'+(S.testBusy?' · test running, polling paused':'');
  $('sysInfo').textContent='Firmware '+S.firmware+' · up '+Math.floor(S.uptime/3600)+'h'+Math.floor(S.uptime%3600/60)+'m · free heap '+S.heap+' B'+(n.ip?' · IP '+n.ip:'')+' · http://'+esc(n.hostname)+'.local/';
  if(!settingsLoaded&&n.mode==='setup'){settingsLoaded=true;tab('settings')}
  if($('addr').classList.contains('on'))renderAddr(false)}

async function cmd(c){msg('dashMsg','Sending...');await run('dashMsg',()=>api('command',{cmd:c}));setTimeout(status,2500)}
function forceOff(){if(confirm('Force off writes 0 to the state register, bypassing the normal shutdown. Continue?'))cmd('force off')}
function setTemp(){const v=parseFloat($('tset').value);if(isNaN(v))return msg('dashMsg','Enter a temperature',false);cmd('T'+v)}
function setPower(){cmd('P'+$('pset').value)}
async function pollNow(b){b.disabled=true;msg('dashMsg','Reading the stove...');try{await api('poll',null,'POST');msg('dashMsg','');await status()}catch(e){msg('dashMsg',e.message,false)}b.disabled=false}

/* Tests */
async function readAll(b){b.disabled=true;$('raTable').innerHTML='<tr><td>Reading...</td></tr>';
  try{const j=await api('test/readall',null,'POST');let h='<thead><tr><th>Metric</th><th>Address</th><th>Result</th><th>Raw</th><th>Value</th><th>Reply</th></tr></thead>';
  j.results.forEach(r=>{const d=r.read;h+='<tr><td>'+esc(r.label)+'</td><td>'+(d.memory?'EEPROM ':'RAM ')+d.address+'</td><td><span class="dot '+(d.ok?'ok':'err')+'"></span>'+d.result+'</td><td>'+(d.ok?d.value:'')+'</td><td>'+esc(r.text)+'</td><td>'+d.reply+' ('+d.bytes+')</td></tr>'});
  $('raTable').innerHTML=h}catch(e){$('raTable').innerHTML='<tr><td class="msg err">'+esc(e.message)+'</td></tr>'}b.disabled=false;status()}
async function readOne(){await run('rdMsg',async()=>{const d=await api('test/read',{address:$('rdAddr').value,memory:$('rdMem').value});
  msg('rdMsg',d.ok?(d.memory?'EEPROM ':'RAM ')+d.address+' = '+d.value+' ('+hex(d.value)+', /2 = '+d.value/2+', /10 = '+d.value/10+')':'Failed: '+d.result+' (reply '+d.reply+', '+d.bytes+' bytes)',d.ok)})}
async function writeOne(){const a=$('wrAddr').value,v=$('wrVal').value,m=$('wrMem').value;if(!a||!v)return msg('wrMsg','Address and value required',false);
  if(!confirm('Write '+v+' to '+(m==1?'EEPROM':'RAM')+' '+a+'?'))return;await run('wrMsg',()=>api('test/write',{address:a,value:v,memory:m}));testStatus()}
async function onOff(){const s=$('ooSec').value;if(!confirm('The stove will be turned ON, then OFF after '+s+' s. Continue?'))return;
  try{await api('test/onoff',{seconds:s});watchTest()}catch(e){alert(e.message)}}
async function scan(){if(lastScan)prevScan=lastScan;try{await api('test/scan',{memory:$('scMem').value,start:$('scStart').value,end:$('scEnd').value});watchTest()}catch(e){$('scInfo').textContent=e.message}}
async function stopTest(){try{await api('test/stop',null,'POST')}catch(e){}testStatus()}
function watchTest(){clearTimeout(testTimer);testStatus()}
async function testStatus(){let t;try{t=await api('test/status')}catch(e){return}
  $('log').textContent=t.log.length?t.log.join('\n'):'No test run yet.';$('log').scrollTop=1e9;
  $('scBtn').disabled=t.running!=='idle';
  if(t.last==='scan'){const total=t.end-t.start+1,done=t.running==='scan'?t.current-t.start:total;
    $('scBar').style.width=(100*done/total)+'%';
    $('scInfo').textContent=(t.running==='scan'?'Scanning '+hex(t.current)+'... ':'Done. ')+t.ok+' replies, '+t.errors+' errors, '+t.elapsed+' s'+(prevScan?' · compared with the previous scan':'');
    lastScan={mem:t.memory,v:new Map(t.scan)};renderScan()}
  if(t.running!=='idle')testTimer=setTimeout(testStatus,1000);else if(S&&S.testBusy)status()}
function renderScan(){if(!lastScan)return;const only=$('scChanged').checked,p=prevScan&&prevScan.mem===lastScan.mem?prevScan.v:null;
  let h='<thead><tr><th>Address</th><th>Dec</th><th>Hex</th><th>/2</th><th>/10</th><th>Previous</th><th></th></tr></thead><tbody>';
  lastScan.v.forEach((v,a)=>{const pv=p?p.get(a):undefined,ch=p&&pv!==v;if(only&&!ch)return;
    h+='<tr'+(ch?' class="ch"':'')+'><td>'+hex(a)+'</td><td>'+v+'</td><td>'+hex(v)+'</td><td>'+v/2+'</td><td>'+v/10+'</td><td>'+(pv===undefined?'':pv)+'</td><td><button class="b" onclick="useAddr('+a+','+lastScan.mem+')">Use for&hellip;</button></td></tr>'});
  $('scTable').innerHTML=h+'</tbody>'}
function useAddr(a,m){const k=prompt('Assign '+hex(a)+' to which metric? ('+S.metrics.map(x=>x.key).join(', ')+')');if(!k)return;
  const x=S.metrics.find(y=>y.key===k.trim());if(!x)return alert('Unknown metric');
  api('metric',{key:x.key,address:a,memory:m,divisor:x.divisor}).then(j=>{alert(j.message);status()}).catch(e=>alert(e.message))}

/* Addresses */
function renderAddr(force){if(!S)return;const body=$('addrBody');
  if(force||!body.children.length){let h='';S.metrics.forEach(x=>{const fixedDiv=x.key==='state'||x.key==='flamepower';
    h+='<tr data-k="'+x.key+'"><td>'+esc(x.label)+(x.hydro?' <span class="muted">(hydro)</span>':'')+'<div class="muted" id="t_'+x.key+'"></div></td><td id="c_'+x.key+'"></td>'+
    '<td><input id="a_'+x.key+'" value="'+x.address+'"></td><td><select id="m_'+x.key+'"><option value="0">RAM</option><option value="1">EEPROM</option></select></td>'+
    '<td><input id="d_'+x.key+'" type="number" min="1" max="100" value="'+x.divisor+'"'+(fixedDiv?' disabled':'')+'></td><td><input type="checkbox" id="e_'+x.key+'"'+(x.enabled?' checked':'')+'></td>'+
    '<td><button class="b" onclick="testAddr(\''+x.key+'\')">Test</button> <button class="b p" onclick="saveAddr(\''+x.key+'\')">Save</button> <button class="b" title="Default: '+(x.default.memory?'EEPROM ':'RAM ')+x.default.address+' /'+x.default.divisor+'" onclick="resetAddr(\''+x.key+'\')">Default</button></td></tr>'});
    body.innerHTML=h;S.metrics.forEach(x=>$('m_'+x.key).value=x.memory)}
  S.metrics.forEach(x=>{$('c_'+x.key).innerHTML=x.valid?esc(x.value)+' '+esc(x.unit)+' <span class="muted">(raw '+x.raw+')</span>':'<span class="muted">'+(x.active?'no reply':x.enabled?'hydro mode off':'disabled')+'</span>'})}
async function testAddr(k){const t=$('t_'+k);t.textContent='Reading...';
  try{const d=await api('test/read',{address:$('a_'+k).value,memory:$('m_'+k).value}),div=parseInt($('d_'+k).value)||1;
  t.textContent=d.ok?'Read '+d.value+' → '+(d.value/div)+(k==='state'?' ('+d.value+')':''):'Failed: '+d.result}catch(e){t.textContent=e.message}}
async function saveAddr(k){await run('addrMsg',()=>api('metric',{key:k,address:$('a_'+k).value,memory:$('m_'+k).value,divisor:$('d_'+k).value,enabled:$('e_'+k).checked?1:0}));setTimeout(async()=>{await status();renderAddr(true)},1500)}
async function resetAddr(k){await run('addrMsg',()=>api('metric/reset',{key:k}));setTimeout(async()=>{await status();renderAddr(true)},1500)}
async function resetAll(){if(!confirm('Restore every address to its default?'))return;resetAddr('all')}

/* Settings */
async function loadSettings(){try{const s=await api('settings');
  $('wSsid').value=s.wifi.ssid;$('wPass').value='';$('wPass').placeholder=s.wifi.hasPass?'(unchanged)':'';
  $('mSrv').value=s.mqtt.server;$('mPort').value=s.mqtt.port;$('mUser').value=s.mqtt.user;$('mPass').value='';$('mPass').placeholder=s.mqtt.hasPass?'(unchanged)':'';$('mTopic').value=s.mqtt.topic;$('mClear').checked=false;
  $('gHost').value=s.general.hostname;$('gPoll').value=s.general.poll;$('gHydro').checked=s.general.hydro;$('gUser').value=s.general.webuser;$('gPass').value='';$('gPass').placeholder=s.general.auth?'(unchanged)':'(none)';$('gNoAuth').checked=!s.general.auth}catch(e){msg('wMsg',e.message,false)}}
async function wifiScan(b){b.disabled=true;$('nets').innerHTML='<span class="muted">Scanning...</span>';
  try{let j;for(let i=0;i<20;i++){j=await api('wifi/scan');if(!j.scanning)break;await new Promise(r=>setTimeout(r,1000))}
  const seen=new Set();$('nets').innerHTML=(j.networks||[]).sort((a,b)=>b.rssi-a.rssi).filter(n=>!seen.has(n.ssid)&&seen.add(n.ssid)).map(n=>'<button class="b" data-s="'+esc(n.ssid)+'"><span>'+(n.open?'':'&#128274; ')+esc(n.ssid)+'</span><span class="muted">'+n.rssi+' dBm</span></button>').join('')||'<span class="muted">No network found</span>';
  $('nets').querySelectorAll('button').forEach(x=>x.onclick=()=>{$('wSsid').value=x.dataset.s;$('wPass').value='';$('wPass').placeholder='';$('wPass').focus()})}catch(e){$('nets').innerHTML='<span class="msg err">'+esc(e.message)+'</span>'}b.disabled=false}
function saveWifi(){run('wMsg',()=>api('settings/wifi',{ssid:$('wSsid').value,pass:$('wPass').value}))}
function saveMqtt(){run('mMsg',()=>api('settings/mqtt',{server:$('mSrv').value,port:$('mPort').value,user:$('mUser').value,pass:$('mPass').value,topic:$('mTopic').value,clearpass:$('mClear').checked?1:0}))}
function saveGeneral(){if($('gNoAuth').checked&&!confirm('Disable the password? Anyone on your network will be able to control the stove.'))return;
  run('gMsg',()=>api('settings/general',{hostname:$('gHost').value,poll:$('gPoll').value,hydro:$('gHydro').checked?1:0,webuser:$('gUser').value,webpass:$('gPass').value,noauth:$('gNoAuth').checked?1:0}))}
function sys(p,q){if(confirm(q))run('sysMsg',()=>api(p,null,'POST'))}

status();setInterval(()=>{if(!document.hidden)status()},5000);
</script>
</body>
</html>
)rawliteral";
