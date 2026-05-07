#ifndef WEB_PAGE_H
#define WEB_PAGE_H

#include <Arduino.h>

const char WEB_PAGE_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1,user-scalable=no,maximum-scale=1">
<title>Comet Drone</title>
<style>
*{margin:0;padding:0;box-sizing:border-box;-webkit-user-select:none;user-select:none;touch-action:none}
@import url('https://fonts.googleapis.com/css2?family=Orbitron:wght@400;700;900&family=Inter:wght@300;400;600&display=swap');
:root{
--bg:#0a0a12;--panel:rgba(20,20,40,0.85);--border:rgba(80,120,255,0.15);
--accent:#4e8cff;--accent2:#00e4a0;--danger:#ff3b5c;--warn:#ffaa00;
--text:#e0e6f0;--dim:#6b7a99;--glow:0 0 20px rgba(78,140,255,0.3);
}
html,body{height:100%;overflow:hidden;background:var(--bg);color:var(--text);font-family:'Inter',sans-serif}
body{display:flex;flex-direction:column}

/* ── Header ── */
.hdr{display:flex;align-items:center;justify-content:space-between;padding:8px 16px;
background:var(--panel);border-bottom:1px solid var(--border);backdrop-filter:blur(12px);z-index:10}
.hdr .logo{font-family:'Orbitron',monospace;font-weight:900;font-size:14px;
background:linear-gradient(135deg,var(--accent),var(--accent2));-webkit-background-clip:text;
-webkit-text-fill-color:transparent;letter-spacing:2px}
.hdr .status{display:flex;align-items:center;gap:12px;font-size:11px}
.conn-dot{width:8px;height:8px;border-radius:50%;background:#555;transition:.3s}
.conn-dot.on{background:var(--accent2);box-shadow:0 0 8px var(--accent2)}

/* ── Telemetry Bar ── */
.telem{display:grid;grid-template-columns:repeat(4,1fr);gap:6px;padding:8px 12px;
background:var(--panel);border-bottom:1px solid var(--border)}
.telem .tbox{text-align:center;padding:4px}
.tbox label{font-size:9px;color:var(--dim);text-transform:uppercase;letter-spacing:1px;display:block}
.tbox .val{font-family:'Orbitron',monospace;font-size:16px;font-weight:700;color:var(--accent)}
.tbox .val.warn{color:var(--warn)}
.tbox .val.crit{color:var(--danger)}

/* ── Motor indicators ── */
.motors-bar{display:flex;justify-content:center;gap:10px;padding:4px 12px;
background:var(--panel);border-bottom:1px solid var(--border)}
.motor-ind{display:flex;align-items:center;gap:4px;font-size:9px;color:var(--dim)}
.motor-ind .bar{width:40px;height:5px;background:rgba(255,255,255,0.05);border-radius:3px;overflow:hidden}
.motor-ind .bar-fill{height:100%;background:var(--accent);border-radius:3px;transition:width 0.1s;width:0%}

/* ── Main Joystick Area ── */
.joy-area{flex:1;display:flex;justify-content:space-around;align-items:center;padding:12px;gap:12px;
position:relative;overflow:hidden}
.joy-area::before{content:'';position:absolute;inset:0;
background:radial-gradient(ellipse at 30% 50%,rgba(78,140,255,0.03) 0%,transparent 70%),
radial-gradient(ellipse at 70% 50%,rgba(0,228,160,0.03) 0%,transparent 70%);pointer-events:none}

.joy-container{position:relative;display:flex;flex-direction:column;align-items:center;gap:6px}
.joy-label{font-size:10px;color:var(--dim);text-transform:uppercase;letter-spacing:2px;font-family:'Orbitron',monospace}
.joy-zone{width:160px;height:160px;border-radius:50%;background:rgba(255,255,255,0.02);
border:2px solid var(--border);position:relative;touch-action:none}
.joy-zone .ring{position:absolute;inset:15%;border-radius:50%;border:1px solid rgba(78,140,255,0.1)}
.joy-knob{width:50px;height:50px;border-radius:50%;position:absolute;
top:50%;left:50%;transform:translate(-50%,-50%);cursor:grab;
background:radial-gradient(circle at 35% 35%,rgba(78,140,255,0.6),rgba(78,140,255,0.15));
border:2px solid rgba(78,140,255,0.5);box-shadow:var(--glow);transition:box-shadow 0.2s}
.joy-knob.active{box-shadow:0 0 30px rgba(78,140,255,0.6);border-color:var(--accent)}

/* ── Control Buttons ── */
.controls{display:flex;justify-content:center;align-items:center;gap:12px;padding:10px 16px;
background:var(--panel);border-top:1px solid var(--border)}
.btn{padding:10px 20px;border:none;border-radius:8px;font-family:'Orbitron',monospace;
font-size:11px;font-weight:700;cursor:pointer;letter-spacing:1px;transition:all 0.2s;text-transform:uppercase}
.btn-arm{background:rgba(0,228,160,0.1);color:var(--accent2);border:1.5px solid rgba(0,228,160,0.3)}
.btn-arm:active{background:rgba(0,228,160,0.3)}
.btn-arm.armed{background:rgba(255,59,92,0.15);color:var(--danger);border-color:rgba(255,59,92,0.4);
animation:pulse-danger 1.5s infinite}
.btn-stop{background:rgba(255,59,92,0.15);color:var(--danger);border:1.5px solid rgba(255,59,92,0.3);
min-width:50px}
.btn-stop:active{background:rgba(255,59,92,0.4)}
.btn-mode{background:rgba(78,140,255,0.1);color:var(--accent);border:1.5px solid rgba(78,140,255,0.2)}
.btn-mode:active{background:rgba(78,140,255,0.3)}

@keyframes pulse-danger{0%,100%{box-shadow:0 0 5px rgba(255,59,92,0.2)}50%{box-shadow:0 0 20px rgba(255,59,92,0.4)}}

/* ── Landscape tweaks ── */
@media(min-width:600px){
.joy-zone{width:200px;height:200px}
.joy-knob{width:60px;height:60px}
.telem{grid-template-columns:repeat(6,1fr)}
}
</style>
</head>
<body>

<!-- Header -->
<div class="hdr">
  <div class="logo">COMET DRONE</div>
  <div class="status">
    <span id="armBadge" style="font-size:10px;font-weight:600;color:var(--accent2)">DISARMED</span>
    <div class="conn-dot" id="connDot"></div>
  </div>
</div>

<!-- Telemetry -->
<div class="telem">
  <div class="tbox"><label>Roll</label><div class="val" id="tRoll">0.0&deg;</div></div>
  <div class="tbox"><label>Pitch</label><div class="val" id="tPitch">0.0&deg;</div></div>
  <div class="tbox"><label>Yaw</label><div class="val" id="tYaw">0.0&deg;</div></div>
  <div class="tbox"><label>Battery</label><div class="val" id="tBatt">--%</div></div>
</div>

<!-- Motor bars -->
<div class="motors-bar">
  <div class="motor-ind">FL<div class="bar"><div class="bar-fill" id="mFL"></div></div></div>
  <div class="motor-ind">FR<div class="bar"><div class="bar-fill" id="mFR"></div></div></div>
  <div class="motor-ind">RL<div class="bar"><div class="bar-fill" id="mRL"></div></div></div>
  <div class="motor-ind">RR<div class="bar"><div class="bar-fill" id="mRR"></div></div></div>
</div>

<!-- Joysticks -->
<div class="joy-area">
  <div class="joy-container">
    <div class="joy-zone" id="joyL">
      <div class="ring"></div>
      <div class="joy-knob" id="knobL"></div>
    </div>
    <div class="joy-label">Throttle / Yaw</div>
  </div>
  <div class="joy-container">
    <div class="joy-zone" id="joyR">
      <div class="ring"></div>
      <div class="joy-knob" id="knobR"></div>
    </div>
    <div class="joy-label">Roll / Pitch</div>
  </div>
</div>

<!-- Controls -->
<div class="controls">
  <button class="btn btn-mode" id="btnMode" onclick="toggleMode()">ANGLE</button>
  <button class="btn btn-arm" id="btnArm" onclick="toggleArm()">ARM</button>
  <button class="btn btn-stop" id="btnStop" onclick="emergencyStop()">STOP</button>
</div>

<script>
// ── State ──
let armed=false, mode=0, connected=false;
let throttle=0, roll=0, pitch=0, yaw=0;
let ws=null, sendTimer=null;

// ── WebSocket ──
function connect(){
  const host=window.location.hostname||'192.168.4.1';
  ws=new WebSocket('ws://'+host+':81');
  ws.onopen=()=>{connected=true;document.getElementById('connDot').classList.add('on')};
  ws.onclose=()=>{connected=false;document.getElementById('connDot').classList.remove('on');setTimeout(connect,2000)};
  ws.onerror=()=>{ws.close()};
  ws.onmessage=(e)=>{
    try{
      const d=JSON.parse(e.data);
      document.getElementById('tRoll').innerHTML=d.r.toFixed(1)+'&deg;';
      document.getElementById('tPitch').innerHTML=d.p.toFixed(1)+'&deg;';
      document.getElementById('tYaw').innerHTML=d.y.toFixed(1)+'&deg;';
      const bp=d.bp;
      const bEl=document.getElementById('tBatt');
      bEl.textContent=bp+'%';
      bEl.className='val'+(bp<15?' crit':bp<30?' warn':'');
      // Motor bars (values 0-255 → 0-100%)
      document.getElementById('mFR').style.width=(d.m1/255*100)+'%';
      document.getElementById('mRL').style.width=(d.m2/255*100)+'%';
      document.getElementById('mFL').style.width=(d.m3/255*100)+'%';
      document.getElementById('mRR').style.width=(d.m4/255*100)+'%';
    }catch(ex){}
  };
}

function sendCmd(){
  if(ws&&ws.readyState===1){
    ws.send('T:'+throttle.toFixed(3)+',R:'+roll.toFixed(3)+',P:'+pitch.toFixed(3)+',Y:'+yaw.toFixed(3)+',A:'+(armed?1:0)+',M:'+mode);
  }
}

// ── Buttons ──
function toggleArm(){
  // Safety: only arm if throttle is zero
  if(!armed&&throttle>0.05){alert('Lower throttle to arm!');return}
  armed=!armed;
  const el=document.getElementById('btnArm');
  const badge=document.getElementById('armBadge');
  if(armed){el.textContent='DISARM';el.classList.add('armed');badge.textContent='ARMED';badge.style.color='var(--danger)'}
  else{el.textContent='ARM';el.classList.remove('armed');badge.textContent='DISARMED';badge.style.color='var(--accent2)';throttle=0;updateKnob('L',0,0)}
  sendCmd();
}
function emergencyStop(){armed=false;throttle=0;roll=0;pitch=0;yaw=0;
  document.getElementById('btnArm').textContent='ARM';
  document.getElementById('btnArm').classList.remove('armed');
  document.getElementById('armBadge').textContent='DISARMED';
  document.getElementById('armBadge').style.color='var(--accent2)';
  updateKnob('L',0,0);updateKnob('R',0,0);sendCmd()}
function toggleMode(){mode=mode?0:1;document.getElementById('btnMode').textContent=mode?'RATE':'ANGLE';sendCmd()}

// ── Joystick Logic ──
function setupJoystick(id,knobId,onMove,onRelease){
  const zone=document.getElementById(id), knob=document.getElementById(knobId);
  let active=false, tid=null, rect;

  function start(ex,ey){
    rect=zone.getBoundingClientRect();active=true;knob.classList.add('active');
    move(ex,ey);
  }
  function move(ex,ey){
    if(!active)return;
    const cx=rect.left+rect.width/2, cy=rect.top+rect.height/2;
    let dx=(ex-cx)/(rect.width/2), dy=(ey-cy)/(rect.height/2);
    const dist=Math.sqrt(dx*dx+dy*dy);
    if(dist>1){dx/=dist;dy/=dist}
    knob.style.left=(50+dx*40)+'%';
    knob.style.top=(50+dy*40)+'%';
    onMove(dx,dy);
  }
  function end(){
    if(!active)return;active=false;knob.classList.remove('active');
    if(onRelease)onRelease();
  }

  zone.addEventListener('touchstart',(e)=>{e.preventDefault();const t=e.touches[0];start(t.clientX,t.clientY)},{passive:false});
  zone.addEventListener('touchmove',(e)=>{e.preventDefault();const t=e.touches[0];move(t.clientX,t.clientY)},{passive:false});
  zone.addEventListener('touchend',(e)=>{e.preventDefault();end()},{passive:false});
  zone.addEventListener('mousedown',(e)=>{start(e.clientX,e.clientY)});
  window.addEventListener('mousemove',(e)=>{move(e.clientX,e.clientY)});
  window.addEventListener('mouseup',()=>{end()});
}

function updateKnob(side,dx,dy){
  const knob=document.getElementById('knob'+side);
  knob.style.left=(50+dx*40)+'%';
  knob.style.top=(50+dy*40)+'%';
}

// Left stick: X=Yaw, Y=Throttle (inverted, does NOT auto-center Y)
let leftY=0;
setupJoystick('joyL','knobL',
  (dx,dy)=>{yaw=dx; throttle=Math.max(0,Math.min(1,(-dy+1)/2)); leftY=dy},
  ()=>{yaw=0; updateKnob('L',0,leftY)} // Only center X (yaw), keep Y (throttle)
);

// Right stick: X=Roll, Y=Pitch (auto-centers both)
setupJoystick('joyR','knobR',
  (dx,dy)=>{roll=dx; pitch=-dy},
  ()=>{roll=0;pitch=0;updateKnob('R',0,0)}
);

// ── Init ──
connect();
sendTimer=setInterval(sendCmd,50); // 20Hz command rate
</script>
</body>
</html>
)rawliteral";

#endif
