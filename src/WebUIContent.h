#pragma once
#include <Arduino.h>

namespace WebUIContent {

static const char INDEX_HTML[] PROGMEM = R"rawliteral(<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>RaceChrono Monitor Config</title>
<style>
:root {
  --bg: #121214;
  --card: #1c1c22;
  --card-border: #2e2e38;
  --text: #f0f0f5;
  --text-dim: #9090a0;
  --accent: #00d2ff;
  --accent-green: #00e676;
  --accent-red: #ff3366;
  --input-bg: #282832;
}
* { box-sizing: border-box; margin: 0; padding: 0; font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif; }
body { background: var(--bg); color: var(--text); padding: 16px; padding-bottom: 90px; }
.container { max-width: 680px; margin: 0 auto; }
header { margin-bottom: 20px; text-align: center; }
h1 { font-size: 1.5rem; color: var(--accent); margin-bottom: 4px; display: flex; align-items: center; justify-content: center; gap: 8px; }
p.subtitle { font-size: 0.85rem; color: var(--text-dim); }
.card { background: var(--card); border: 1px solid var(--card-border); border-radius: 12px; padding: 16px; margin-bottom: 16px; }
.card-header { display: flex; justify-content: space-between; align-items: center; margin-bottom: 14px; }
.card-title { font-size: 1.1rem; font-weight: 600; }
.form-group { margin-bottom: 12px; }
.form-row { display: flex; gap: 10px; margin-bottom: 10px; }
.form-row .form-group { flex: 1; margin-bottom: 0; }
label { display: block; font-size: 0.75rem; font-weight: 600; text-transform: uppercase; color: var(--text-dim); margin-bottom: 4px; }
input[type="text"], input[type="number"], select, textarea {
  width: 100%; background: var(--input-bg); border: 1px solid var(--card-border); border-radius: 6px; color: var(--text); padding: 8px 10px; font-size: 0.9rem;
}
input[type="text"]:focus, input[type="number"]:focus, select:focus, textarea:focus {
  outline: none; border-color: var(--accent);
}
.checkbox-row { display: flex; align-items: center; gap: 10px; cursor: pointer; }
.checkbox-row input[type="checkbox"] { width: 18px; height: 18px; accent-color: var(--accent); }
.color-picker-group { display: flex; align-items: center; gap: 8px; }
.color-picker-group input[type="color"] {
  -webkit-appearance: none; border: none; width: 32px; height: 32px; border-radius: 6px; cursor: pointer; background: transparent;
}
.color-picker-group input[type="color"]::-webkit-color-swatch-wrapper { padding: 0; }
.color-picker-group input[type="color"]::-webkit-color-swatch { border: 1px solid var(--card-border); border-radius: 6px; }
.item-card { background: #23232c; border: 1px solid var(--card-border); border-radius: 8px; padding: 12px; margin-bottom: 10px; position: relative; }
.btn {
  display: inline-flex; align-items: center; justify-content: center; gap: 6px;
  background: var(--accent); color: #000; font-weight: 600; font-size: 0.85rem; border: none; border-radius: 6px; padding: 8px 14px; cursor: pointer;
}
.btn:hover { opacity: 0.9; }
.btn-sm { padding: 4px 8px; font-size: 0.75rem; }
.btn-danger { background: var(--accent-red); color: #fff; }
.btn-secondary { background: #32323e; color: var(--text); border: 1px solid var(--card-border); }
.action-bar {
  position: fixed; bottom: 0; left: 0; right: 0; background: rgba(28, 28, 34, 0.95); backdrop-filter: blur(8px);
  border-top: 1px solid var(--card-border); padding: 12px 16px; display: flex; justify-content: center; gap: 12px; z-index: 100;
}
.action-bar .btn { flex: 1; max-width: 260px; padding: 12px; font-size: 0.95rem; }
.raw-json-area { width: 100%; height: 160px; font-family: monospace; font-size: 0.8rem; line-height: 1.3; }
.badge { font-size: 0.7rem; padding: 2px 6px; border-radius: 4px; background: #32323e; color: var(--text-dim); font-weight: bold; }
</style>
</head>
<body>
<div class="container">
  <header>
    <h1>🏁 RaceChrono Monitor</h1>
    <p class="subtitle">Device & Screen Configuration</p>
  </header>

  <div class="card">
    <div class="card-header">
      <span class="card-title">General Settings</span>
    </div>
    <div class="form-group">
      <label class="checkbox-row">
        <input type="checkbox" id="isHud">
        <span>HUD Mirrored Mode</span>
      </label>
    </div>
    <div class="form-group" style="margin-top: 12px;">
      <label class="checkbox-row">
        <input type="checkbox" id="webuiEnabled">
        <span>Enable Wi-Fi Access Point (WebUI)</span>
      </label>
    </div>
    <div class="form-row" id="webuiFields" style="margin-top: 10px;">
      <div class="form-group">
        <label>AP SSID</label>
        <input type="text" id="webuiSsid" placeholder="RaceChrono-AP" maxlength="31">
      </div>
      <div class="form-group">
        <label>AP Password</label>
        <input type="text" id="webuiPass" placeholder="min 8 chars" maxlength="63">
      </div>
    </div>
  </div>

  <div class="card">
    <div class="card-header">
      <span class="card-title">Monitors (<span id="monCount">0</span>/8)</span>
      <button class="btn btn-sm btn-secondary" onclick="addMonitor()">+ Add Monitor</button>
    </div>
    <div id="monitorsContainer"></div>
  </div>

  <div class="card">
    <div class="card-header">
      <span class="card-title">Screens (<span id="scrCount">0</span>/8)</span>
      <button class="btn btn-sm btn-secondary" onclick="addScreen()">+ Add Screen</button>
    </div>
    <div id="screensContainer"></div>
  </div>

  <div class="card">
    <div class="card-header">
      <span class="card-title">Raw JSON Config</span>
      <button class="btn btn-sm btn-secondary" onclick="syncRawToForm()">Apply Raw JSON</button>
    </div>
    <textarea id="rawJson" class="raw-json-area" spellcheck="false"></textarea>
  </div>
</div>

<div class="action-bar">
  <button class="btn btn-secondary" onclick="rebootDevice()">🔄 Restart Device</button>
  <button class="btn" onclick="saveConfig()">💾 Save & Apply</button>
</div>

<script>
let config = { isHud: false, webui: { enabled: false, ssid: "", password: "" }, monitors: [], screens: [] };

function init() {
  fetch('/api/config')
    .then(r => r.json())
    .then(data => {
      config = data;
      if (!config.webui) config.webui = { enabled: false, ssid: "", password: "" };
      if (!config.monitors) config.monitors = [];
      if (!config.screens) config.screens = [];
      renderAll();
    })
    .catch(err => {
      console.warn("Could not load /api/config, using default template", err);
      renderAll();
    });
}

function renderAll() {
  document.getElementById('isHud').checked = !!config.isHud;
  document.getElementById('webuiEnabled').checked = !!config.webui.enabled;
  document.getElementById('webuiSsid').value = config.webui.ssid || '';
  document.getElementById('webuiPass').value = config.webui.password || '';

  renderMonitors();
  renderScreens();
  updateRawJson();
}

function renderMonitors() {
  const container = document.getElementById('monitorsContainer');
  container.innerHTML = '';
  document.getElementById('monCount').innerText = config.monitors.length;

  config.monitors.forEach((m, idx) => {
    const item = document.createElement('div');
    item.className = 'item-card';
    item.innerHTML = `
      <div style="display:flex; justify-content:space-between; align-items:center; margin-bottom:8px;">
        <span class="badge">#${idx + 1} ${m.id || 'monitor'}</span>
        <button class="btn btn-sm btn-danger" onclick="deleteMonitor(${idx})">Delete</button>
      </div>
      <div class="form-row">
        <div class="form-group">
          <label>ID</label>
          <input type="text" value="${m.id || ''}" onchange="updateMonProp(${idx}, 'id', this.value)" placeholder="lap_delta">
        </div>
        <div class="form-group">
          <label>Title</label>
          <input type="text" value="${m.title || ''}" onchange="updateMonProp(${idx}, 'title', this.value)" placeholder="TIME">
        </div>
      </div>
      <div class="form-group">
        <label>Formula</label>
        <input type="text" value="${m.formula || ''}" onchange="updateMonProp(${idx}, 'formula', this.value)">
      </div>
      <div class="form-row">
        <div class="form-group">
          <label>Multiplier</label>
          <input type="number" step="any" value="${m.multiplier !== undefined ? m.multiplier : 1.0}" onchange="updateMonProp(${idx}, 'multiplier', parseFloat(this.value))">
        </div>
        <div class="form-group">
          <label>Decimals</label>
          <input type="number" min="0" max="3" value="${m.decimals !== undefined ? m.decimals : 1}" onchange="updateMonProp(${idx}, 'decimals', parseInt(this.value))">
        </div>
        <div class="form-group">
          <label>Limit</label>
          <input type="number" step="any" value="${m.limit !== undefined ? m.limit : 1.0}" onchange="updateMonProp(${idx}, 'limit', parseFloat(this.value))">
        </div>
      </div>
    `;
    container.appendChild(item);
  });
}

function updateMonProp(idx, prop, val) {
  config.monitors[idx][prop] = val;
  updateRawJson();
  renderScreens(); // Re-render monitor dropdowns
}

function addMonitor() {
  if (config.monitors.length >= 8) return alert('Maximum 8 monitors supported');
  config.monitors.push({ id: `mon_${config.monitors.length + 1}`, title: "LABEL", formula: "channel(device(lap), delta_lap_time)*100.0", multiplier: 0.01, decimals: 2, limit: 0.5 });
  renderMonitors();
  renderScreens();
  updateRawJson();
}

function deleteMonitor(idx) {
  config.monitors.splice(idx, 1);
  renderMonitors();
  renderScreens();
  updateRawJson();
}

function renderScreens() {
  const container = document.getElementById('screensContainer');
  container.innerHTML = '';
  document.getElementById('scrCount').innerText = config.screens.length;

  config.screens.forEach((s, idx) => {
    const isDual = s.type === 'dual';
    const item = document.createElement('div');
    item.className = 'item-card';

    const monOptions = config.monitors.map(m => `<option value="${m.id}">${m.id} (${m.title})</option>`).join('');

    let contentHtml = '';
    if (!isDual) {
      contentHtml = `
        <div class="form-row">
          <div class="form-group">
            <label>Monitor</label>
            <select onchange="updateScrProp(${idx}, 'monitor', this.value)">
              ${config.monitors.map(m => `<option value="${m.id}" ${s.monitor === m.id ? 'selected' : ''}>${m.id} (${m.title})</option>`).join('')}
            </select>
          </div>
        </div>
        <div class="form-row">
          <div class="form-group">
            <label>Positive Color</label>
            <div class="color-picker-group">
              <input type="color" value="${s.positive_color || '#FF0000'}" onchange="updateScrProp(${idx}, 'positive_color', this.value); this.nextElementSibling.value=this.value;">
              <input type="text" value="${s.positive_color || '#FF0000'}" onchange="updateScrProp(${idx}, 'positive_color', this.value); this.previousElementSibling.value=this.value;">
            </div>
          </div>
          <div class="form-group">
            <label>Negative Color</label>
            <div class="color-picker-group">
              <input type="color" value="${s.negative_color || '#00FF00'}" onchange="updateScrProp(${idx}, 'negative_color', this.value); this.nextElementSibling.value=this.value;">
              <input type="text" value="${s.negative_color || '#00FF00'}" onchange="updateScrProp(${idx}, 'negative_color', this.value); this.previousElementSibling.value=this.value;">
            </div>
          </div>
        </div>
        <div class="form-row">
          <div class="form-group">
            <label>Title Color</label>
            <div class="color-picker-group">
              <input type="color" value="${s.title_color || '#0000FF'}" onchange="updateScrProp(${idx}, 'title_color', this.value); this.nextElementSibling.value=this.value;">
              <input type="text" value="${s.title_color || '#0000FF'}" onchange="updateScrProp(${idx}, 'title_color', this.value); this.previousElementSibling.value=this.value;">
            </div>
          </div>
          <div class="form-group">
            <label>Value Color</label>
            <div class="color-picker-group">
              <input type="color" value="${s.value_color || '#0000FF'}" onchange="updateScrProp(${idx}, 'value_color', this.value); this.nextElementSibling.value=this.value;">
              <input type="text" value="${s.value_color || '#0000FF'}" onchange="updateScrProp(${idx}, 'value_color', this.value); this.previousElementSibling.value=this.value;">
            </div>
          </div>
        </div>
      `;
    } else {
      const topObj = (typeof s.top === 'object') ? s.top : { monitor: s.top || '', positive_color: '#FF0000', negative_color: '#00FF00' };
      const btmObj = (typeof s.bottom === 'object') ? s.bottom : { monitor: s.bottom || '', positive_color: '#00FFFF', negative_color: '#FFA500' };

      contentHtml = `
        <div style="font-weight:600; font-size:0.8rem; color:var(--accent); margin-bottom:4px;">Top Slot:</div>
        <div class="form-row">
          <div class="form-group">
            <label>Top Monitor</label>
            <select onchange="updateDualSlot(${idx}, 'top', 'monitor', this.value)">
              ${config.monitors.map(m => `<option value="${m.id}" ${topObj.monitor === m.id ? 'selected' : ''}>${m.id} (${m.title})</option>`).join('')}
            </select>
          </div>
          <div class="form-group">
            <label>Top Positive</label>
            <div class="color-picker-group">
              <input type="color" value="${topObj.positive_color || '#FF0000'}" onchange="updateDualSlot(${idx}, 'top', 'positive_color', this.value); this.nextElementSibling.value=this.value;">
              <input type="text" value="${topObj.positive_color || '#FF0000'}" onchange="updateDualSlot(${idx}, 'top', 'positive_color', this.value); this.previousElementSibling.value=this.value;">
            </div>
          </div>
          <div class="form-group">
            <label>Top Negative</label>
            <div class="color-picker-group">
              <input type="color" value="${topObj.negative_color || '#00FF00'}" onchange="updateDualSlot(${idx}, 'top', 'negative_color', this.value); this.nextElementSibling.value=this.value;">
              <input type="text" value="${topObj.negative_color || '#00FF00'}" onchange="updateDualSlot(${idx}, 'top', 'negative_color', this.value); this.previousElementSibling.value=this.value;">
            </div>
          </div>
        </div>

        <div style="font-weight:600; font-size:0.8rem; color:var(--accent); margin-top:8px; margin-bottom:4px;">Bottom Slot:</div>
        <div class="form-row">
          <div class="form-group">
            <label>Bottom Monitor</label>
            <select onchange="updateDualSlot(${idx}, 'bottom', 'monitor', this.value)">
              ${config.monitors.map(m => `<option value="${m.id}" ${btmObj.monitor === m.id ? 'selected' : ''}>${m.id} (${m.title})</option>`).join('')}
            </select>
          </div>
          <div class="form-group">
            <label>Bottom Positive</label>
            <div class="color-picker-group">
              <input type="color" value="${btmObj.positive_color || '#00FFFF'}" onchange="updateDualSlot(${idx}, 'bottom', 'positive_color', this.value); this.nextElementSibling.value=this.value;">
              <input type="text" value="${btmObj.positive_color || '#00FFFF'}" onchange="updateDualSlot(${idx}, 'bottom', 'positive_color', this.value); this.previousElementSibling.value=this.value;">
            </div>
          </div>
          <div class="form-group">
            <label>Bottom Negative</label>
            <div class="color-picker-group">
              <input type="color" value="${btmObj.negative_color || '#FFA500'}" onchange="updateDualSlot(${idx}, 'bottom', 'negative_color', this.value); this.nextElementSibling.value=this.value;">
              <input type="text" value="${btmObj.negative_color || '#FFA500'}" onchange="updateDualSlot(${idx}, 'bottom', 'negative_color', this.value); this.previousElementSibling.value=this.value;">
            </div>
          </div>
        </div>
      `;
    }

    item.innerHTML = `
      <div style="display:flex; justify-content:space-between; align-items:center; margin-bottom:8px;">
        <span class="badge">#${idx + 1} ${s.type.toUpperCase()}</span>
        <div>
          <select style="width:auto; padding:4px 8px; font-size:0.75rem;" onchange="changeScreenType(${idx}, this.value)">
            <option value="single" ${!isDual ? 'selected' : ''}>Single</option>
            <option value="dual" ${isDual ? 'selected' : ''}>Dual</option>
          </select>
          <button class="btn btn-sm btn-danger" onclick="deleteScreen(${idx})">Delete</button>
        </div>
      </div>
      ${contentHtml}
    `;
    container.appendChild(item);
  });
}

function changeScreenType(idx, type) {
  if (type === 'dual') {
    const firstMon = config.monitors[0] ? config.monitors[0].id : '';
    const secondMon = config.monitors[1] ? config.monitors[1].id : firstMon;
    config.screens[idx] = {
      type: 'dual',
      top: { monitor: firstMon, positive_color: '#FF0000', negative_color: '#00FF00' },
      bottom: { monitor: secondMon, positive_color: '#00FFFF', negative_color: '#FFA500' }
    };
  } else {
    const firstMon = config.monitors[0] ? config.monitors[0].id : '';
    config.screens[idx] = {
      type: 'single',
      monitor: firstMon,
      positive_color: '#FF0000',
      negative_color: '#00FF00',
      title_color: '#0000FF',
      value_color: '#0000FF'
    };
  }
  renderScreens();
  updateRawJson();
}

function updateScrProp(idx, prop, val) {
  config.screens[idx][prop] = val;
  updateRawJson();
}

function updateDualSlot(idx, slot, prop, val) {
  if (typeof config.screens[idx][slot] !== 'object') {
    config.screens[idx][slot] = { monitor: config.screens[idx][slot] || '', positive_color: '#FF0000', negative_color: '#00FF00' };
  }
  config.screens[idx][slot][prop] = val;
  updateRawJson();
}

function addScreen() {
  if (config.screens.length >= 8) return alert('Maximum 8 screens supported');
  const firstMon = config.monitors[0] ? config.monitors[0].id : '';
  config.screens.push({ type: 'single', monitor: firstMon, positive_color: '#FF0000', negative_color: '#00FF00', title_color: '#0000FF', value_color: '#0000FF' });
  renderScreens();
  updateRawJson();
}

function deleteScreen(idx) {
  config.screens.splice(idx, 1);
  renderScreens();
  updateRawJson();
}

function updateRawJson() {
  config.isHud = document.getElementById('isHud').checked;
  config.webui.enabled = document.getElementById('webuiEnabled').checked;
  config.webui.ssid = document.getElementById('webuiSsid').value;
  config.webui.password = document.getElementById('webuiPass').value;
  document.getElementById('rawJson').value = JSON.stringify(config, null, 2);
}

function syncRawToForm() {
  try {
    const parsed = JSON.parse(document.getElementById('rawJson').value);
    config = parsed;
    if (!config.webui) config.webui = { enabled: false, ssid: "", password: "" };
    if (!config.monitors) config.monitors = [];
    if (!config.screens) config.screens = [];
    renderAll();
  } catch (e) {
    alert("Invalid JSON format: " + e.message);
  }
}

document.getElementById('isHud').addEventListener('change', updateRawJson);
document.getElementById('webuiEnabled').addEventListener('change', updateRawJson);
document.getElementById('webuiSsid').addEventListener('input', updateRawJson);
document.getElementById('webuiPass').addEventListener('input', updateRawJson);

function saveConfig() {
  updateRawJson();
  fetch('/api/config', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify(config)
  })
  .then(r => r.json())
  .then(res => {
    if (res.status === 'ok') {
      window.location.href = '/saved';
    } else {
      alert('Error saving configuration: ' + (res.message || 'Unknown error'));
    }
  })
  .catch(err => {
    alert('Failed to save configuration: ' + err.message);
  });
}

function rebootDevice() {
  if (!confirm("Are you sure you want to reboot the device?")) return;
  fetch('/api/restart', { method: 'POST' })
    .then(() => alert('Device is rebooting... Please wait a few seconds.'))
    .catch(() => alert('Reboot command sent.'));
}

window.onload = init;
</script>
</body>
</html>
)rawliteral";

static const char SAVED_HTML[] PROGMEM = R"rawliteral(<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Configuration Saved</title>
<style>
:root {
  --bg: #121214;
  --card: #1c1c22;
  --card-border: #2e2e38;
  --text: #f0f0f5;
  --text-dim: #9090a0;
  --accent: #00d2ff;
  --accent-green: #00e676;
  --accent-red: #ff3366;
}
* { box-sizing: border-box; margin: 0; padding: 0; font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif; }
body { background: var(--bg); color: var(--text); padding: 24px 16px; min-height: 100vh; display: flex; align-items: center; justify-content: center; }
.card { background: var(--card); border: 1px solid var(--card-border); border-radius: 16px; padding: 32px 24px; max-width: 440px; width: 100%; text-align: center; }
.icon { width: 64px; height: 64px; border-radius: 50%; background: rgba(0, 230, 118, 0.15); color: var(--accent-green); display: flex; align-items: center; justify-content: center; font-size: 32px; margin: 0 auto 16px; }
h1 { font-size: 1.4rem; margin-bottom: 8px; color: var(--text); }
p { font-size: 0.9rem; color: var(--text-dim); margin-bottom: 24px; line-height: 1.4; }
.btn {
  display: block; width: 100%; padding: 12px; font-weight: 600; font-size: 0.95rem; border: none; border-radius: 8px; cursor: pointer; text-decoration: none; margin-bottom: 10px;
}
.btn-primary { background: var(--accent); color: #000; }
.btn-danger { background: #282832; color: var(--accent-red); border: 1px solid var(--card-border); }
.btn:hover { opacity: 0.9; }
</style>
</head>
<body>
<div class="card">
  <div class="icon">✓</div>
  <h1>Configuration Saved</h1>
  <p>The new configuration has been saved and applied. The device has returned to normal monitoring mode.</p>
  <a href="/" class="btn btn-primary">📝 Back to Configuration</a>
  <button class="btn btn-danger" onclick="rebootDevice()">🔄 Restart Device</button>
</div>
<script>
function rebootDevice() {
  if (!confirm("Are you sure you want to reboot the device?")) return;
  fetch('/api/restart', { method: 'POST' })
    .then(() => alert('Device is rebooting... Please wait.'))
    .catch(() => alert('Reboot triggered.'));
}
</script>
</body>
</html>
)rawliteral";

} // namespace WebUIContent
