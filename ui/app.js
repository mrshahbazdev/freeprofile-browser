const qs = (s) => document.querySelector(s);
const qsa = (s) => document.querySelectorAll(s);

function send(cmd, data = {}) {
  return new Promise((resolve, reject) => {
    if (!window.cefQuery) {
      reject('cefQuery not available');
      return;
    }
    const payload = JSON.stringify({ cmd, data });
    window.cefQuery({
      request: payload,
      persistent: false,
      onSuccess: resolve,
      onFailure: (code, msg) => reject(`${code}: ${msg}`)
    });
  });
}

function initLogin() {
  const btn = qs('#loginBtn');
  if (!btn) return;
  btn.onclick = async () => {
    const username = qs('#username').value.trim();
    const password = qs('#password').value.trim();
    try {
      await send('login', { username, password });
      localStorage.setItem('fp_token', '1');
      location.href = 'dashboard.html';
    } catch (e) {
      qs('#error').textContent = e || 'Login failed';
    }
  };

  if (location.search.includes('auto=1')) {
    qs('#username').value = 'admin';
    qs('#password').value = 'admin';
    qs('#loginBtn').click();
  }
}

function val(id) {
  const el = qs(id);
  return el ? el.value.trim() : '';
}

function checked(id) {
  const el = qs(id);
  return el ? el.checked : false;
}

function setVal(id, value) {
  const el = qs(id);
  if (el) el.value = value;
}

function setChecked(id, value) {
  const el = qs(id);
  if (el) el.checked = value;
}

function escapeHtml(s) {
  const div = document.createElement('div');
  div.textContent = s;
  return div.innerHTML;
}

function osAvatarClass(os) {
  if (os === 'Windows') return 'win';
  if (os === 'macOS') return 'mac';
  if (os === 'Linux') return 'lin';
  if (os === 'Android') return 'and';
  if (os === 'iOS') return 'ios';
  return 'win';
}

function osInitial(os) {
  if (os === 'Windows') return 'W';
  if (os === 'macOS') return 'M';
  if (os === 'Linux') return 'L';
  if (os === 'Android') return 'A';
  if (os === 'iOS') return 'I';
  return 'P';
}

function formatUrl(url) {
  if (!url) return 'No start URL';
  return url.replace(/^https?:\/\//, '').replace(/\/$/, '');
}

function profileCard(p) {
  const proxyBadge = p.proxy
    ? `<span class="badge proxy">proxy</span>`
    : `<span class="badge dim">direct</span>`;
  const webrtcBadge = p.disableWebrtc
    ? `<span class="badge">WebRTC off</span>`
    : `<span class="badge warn">WebRTC on</span>`;
  const geoBadge = p.enableGeolocation
    ? `<span class="badge accent">geo</span>`
    : '';
  const autoBadge = p.automationPort > 0
    ? `<span class="badge accent">port ${p.automationPort}</span>`
    : '';
  const macroBadge = p.automationScript
    ? `<span class="badge">macro</span>`
    : '';
  const audioBadge = p.audioNoise ? `<span class="badge">audio</span>` : '';
  const rectBadge = p.clientRectNoise ? `<span class="badge">rects</span>` : '';
  const screen = `${p.screenWidth || 1920} × ${p.screenHeight || 1080}`;
  return `
    <div class="profile-card" data-id="${p.id}">
      <div class="profile-head">
        <div class="profile-avatar ${osAvatarClass(p.os)}">${osInitial(p.os)}</div>
        <div class="profile-title">
          <div class="name">${escapeHtml(p.name || 'Unnamed')}</div>
          <div class="url">${escapeHtml(formatUrl(p.url))}</div>
        </div>
      </div>
      <div class="profile-badges">
        <span class="badge accent">${escapeHtml(p.os || 'Windows')}</span>
        ${proxyBadge}
        <span class="badge dim">${escapeHtml(screen)}</span>
        ${webrtcBadge}
        ${geoBadge}
        ${autoBadge}
        ${macroBadge}
        ${audioBadge}
        ${rectBadge}
      </div>
      <div class="profile-meta">
        <div>${escapeHtml(p.timezone || '')} · ${escapeHtml(p.language || 'en-US')}</div>
        <div>${p.proxy ? escapeHtml(p.proxy) : 'No proxy'} · ${p.enableGeolocation ? `geo ${p.latitude}, ${p.longitude}` : 'no geo'}</div>
      </div>
      <div class="profile-actions">
        <button class="small" data-action="launch" data-id="${p.id}">Launch</button>
        <button class="small danger" data-action="delete" data-id="${p.id}">Delete</button>
      </div>
    </div>
  `;
}

function automationExample(tool, port) {
  if (!tool || tool === 'none' || !port || port <= 0) {
    return 'Select an automation tool and port to generate a connection snippet.';
  }
  if (tool === 'puppeteer' || tool === 'playwright') {
    return `const browser = await ${tool}.launch({\n  wsEndpoint: 'ws://127.0.0.1:${port}/devtools/browser/<id>'\n});`;
  }
  if (tool === 'selenium') {
    return `# First launch the profile with remote debugging port ${port}
# Then attach with Selenium:
from selenium import webdriver
options = webdriver.ChromeOptions()
options.debugger_address = '127.0.0.1:${port}'
driver = webdriver.Chrome(options=options)
# driver.get('https://example.com')
# element = driver.find_element('css selector', 'input')
# element.send_keys('hello')`;
  }
  return 'Select an automation tool and port to generate a connection snippet.';
}

function updateAutomationExample() {
  const tool = val('#pAutomationTool');
  const port = parseInt(val('#pAutomationPort')) || 0;
  const el = qs('#automationExample');
  if (el) el.textContent = automationExample(tool, port);
}

function generateAutomationMacro(actions) {
  const helpers = [
    "function __fp_click(sel){var el=document.querySelector(sel);if(el){el.scrollIntoView({behavior:'smooth',block:'center'});el.click();}}",
    "function __fp_type(sel,text){var el=document.querySelector(sel);if(!el)return;el.focus();el.value=text;el.dispatchEvent(new Event('input',{bubbles:true}));el.dispatchEvent(new Event('change',{bubbles:true}));}",
    "function __fp_scroll(px){window.scrollBy({top:parseInt(px)||0,behavior:'smooth'});}",
    "function __fp_key(key){document.dispatchEvent(new KeyboardEvent('keydown',{key:key,bubbles:true}));document.dispatchEvent(new KeyboardEvent('keyup',{key:key,bubbles:true}));}",
    "async function __fp_wait(ms){await new Promise(r=>setTimeout(r,parseInt(ms)||0));}"
  ].join('\n');
  const lines = [helpers];
  for (const a of actions) {
    const delay = parseInt(a.delay) || 0;
    if (delay > 0) lines.push(`await __fp_wait(${delay});`);
    const t = JSON.stringify(a.target || '');
    const v = JSON.stringify(a.value || '');
    switch (a.type) {
      case 'click': lines.push(`__fp_click(${t});`); break;
      case 'type': lines.push(`__fp_type(${t},${v});`); break;
      case 'scroll': lines.push(`__fp_scroll(${t});`); break;
      case 'keypress': lines.push(`__fp_key(${t});`); break;
      case 'wait': lines.push(`await __fp_wait(${t});`); break;
    }
  }
  return lines.join('\n');
}

function actionRowHTML(action, index) {
  return `
    <div class="action-row" data-index="${index}">
      <select class="action-type">
        <option value="click" ${action.type === 'click' ? 'selected' : ''}>Click</option>
        <option value="type" ${action.type === 'type' ? 'selected' : ''}>Type</option>
        <option value="scroll" ${action.type === 'scroll' ? 'selected' : ''}>Scroll</option>
        <option value="keypress" ${action.type === 'keypress' ? 'selected' : ''}>Key press</option>
        <option value="wait" ${action.type === 'wait' ? 'selected' : ''}>Wait</option>
      </select>
      <input class="action-target" placeholder="${action.type === 'scroll' || action.type === 'wait' ? 'ms/pixels' : (action.type === 'keypress' ? 'key' : 'selector')}" value="${escapeHtml(action.target || '')}">
      <input class="action-value" placeholder="text" value="${escapeHtml(action.value || '')}" ${action.type === 'type' ? '' : 'disabled'}>
      <input class="action-delay" type="number" placeholder="delay ms" value="${action.delay || 0}" min="0">
      <button type="button" class="removeAction" title="Remove">×</button>
    </div>
  `;
}

async function initDashboard() {
  const grid = qs('#profilesGrid');
  const empty = qs('#emptyState');
  const countLabel = qs('#profileCount');
  const modal = qs('#profileModal');
  const newBtn = qs('#newProfileBtn');
  const closeBtn = qs('#closeModal');
  const cancelBtn = qs('#cancelModal');
  const saveBtn = qs('#saveProfile');
  const searchInput = qs('#searchInput');
  const tabs = qsa('.tab');

  let profiles = [];
  let automationActions = [];
  const actionBuilder = qs('#actionBuilder');
  const addActionBtn = qs('#addActionBtn');

  function updateMacroFromActions() {
    if (automationActions.length === 0) {
      setVal('#pAutomationScript', '');
      return;
    }
    setVal('#pAutomationScript', generateAutomationMacro(automationActions));
  }

  function attachActionListeners() {
    qsa('#actionBuilder .action-row').forEach((row, i) => {
      const typeSel = row.querySelector('.action-type');
      const targetInput = row.querySelector('.action-target');
      const valueInput = row.querySelector('.action-value');
      const delayInput = row.querySelector('.action-delay');
      const removeBtn = row.querySelector('.removeAction');
      if (typeSel) typeSel.onchange = (e) => {
        automationActions[i].type = e.target.value;
        renderActionBuilder();
        updateMacroFromActions();
      };
      if (targetInput) targetInput.oninput = (e) => {
        automationActions[i].target = e.target.value;
        updateMacroFromActions();
      };
      if (valueInput) valueInput.oninput = (e) => {
        automationActions[i].value = e.target.value;
        updateMacroFromActions();
      };
      if (delayInput) delayInput.oninput = (e) => {
        automationActions[i].delay = parseInt(e.target.value) || 0;
        updateMacroFromActions();
      };
      if (removeBtn) removeBtn.onclick = () => {
        automationActions.splice(i, 1);
        renderActionBuilder();
        updateMacroFromActions();
      };
    });
  }

  function renderActionBuilder() {
    if (!actionBuilder) return;
    actionBuilder.innerHTML = automationActions.map(actionRowHTML).join('');
    attachActionListeners();
  }

  function resetActionBuilder() {
    automationActions = [];
    renderActionBuilder();
    updateMacroFromActions();
  }

  function openModal() {
    modal.classList.add('open');
    resetForm();
  }

  function closeModal() {
    modal.classList.remove('open');
  }

  function resetForm() {
    qs('#profileForm').reset();
    setVal('#pTimezone', 'America/New_York');
    setVal('#pLanguage', 'en-US');
    setVal('#pDeviceMemory', '8');
    setVal('#pScreenWidth', '1920');
    setVal('#pScreenHeight', '1080');
    setVal('#pUrl', 'https://www.google.com');
    setVal('#pLatitude', '40.7128');
    setVal('#pLongitude', '-74.0060');
    setVal('#pAccuracy', '10');
    setVal('#pWebglVendor', 'Google Inc. (NVIDIA)');
    setVal('#pWebglRenderer', 'ANGLE (NVIDIA, NVIDIA GeForce GTX 1660 Ti Direct3D11 vs_5_0 ps_5_0, D3D11)');
    setVal('#pAutomationPort', '0');
    setVal('#pAutomationTool', 'none');
    setVal('#pAutomationScript', '');
    setVal('#pHardwareConcurrency', '8');
    setVal('#pMaxTouchPoints', '0');
    setVal('#pBatteryLevel', '0.85');
    setVal('#pDevicePixelRatio', '1.0');
    setChecked('#pWebglNoise', true);
    setChecked('#pDisableWebrtc', true);
    setChecked('#pEnableGeolocation', true);
    setChecked('#pChromeSpoof', true);
    setChecked('#pCanvasNoise', false);
    setChecked('#pAudioNoise', false);
    setChecked('#pClientRectNoise', false);
    setChecked('#pPluginsSpoof', true);
    resetActionBuilder();
    updateAutomationExample();
  }

  function render() {
    const term = searchInput ? searchInput.value.toLowerCase() : '';
    const filtered = profiles.filter(p =>
      (p.name || '').toLowerCase().includes(term) ||
      (p.os || '').toLowerCase().includes(term) ||
      (p.url || '').toLowerCase().includes(term)
    );

    if (filtered.length === 0) {
      grid.innerHTML = '';
      grid.appendChild(empty);
      empty.style.display = 'block';
    } else {
      empty.style.display = 'none';
      grid.innerHTML = filtered.map(profileCard).join('');
    }
    if (countLabel) countLabel.textContent = `${profiles.length} profile${profiles.length === 1 ? '' : 's'}`;
  }

  async function refresh() {
    try {
      const json = await send('getProfiles');
      profiles = JSON.parse(json || '[]');
      render();
    } catch (e) {
      console.error(e);
    }
  }

  grid.onclick = async (e) => {
    const btn = e.target.closest('button');
    if (!btn) return;
    const id = btn.dataset.id;
    const action = btn.dataset.action;
    if (action === 'delete') {
      await send('deleteProfile', { id });
      await refresh();
      setTimeout(() => send('repaint').catch(() => {}), 100);
    } else if (action === 'launch') {
      await send('launchProfile', { id });
    }
  };

  saveBtn.onclick = async () => {
    const data = {
      name: val('#pName'),
      proxy: val('#pProxy'),
      userAgent: val('#pUA'),
      url: val('#pUrl') || 'https://www.google.com',
      os: val('#pOS'),
      timezone: val('#pTimezone'),
      language: val('#pLanguage'),
      screenWidth: parseInt(val('#pScreenWidth')) || 1920,
      screenHeight: parseInt(val('#pScreenHeight')) || 1080,
      deviceMemoryGb: parseInt(val('#pDeviceMemory')) || 8,
      latitude: parseFloat(val('#pLatitude')) || 0,
      longitude: parseFloat(val('#pLongitude')) || 0,
      accuracy: parseFloat(val('#pAccuracy')) || 10,
      canvasNoise: checked('#pCanvasNoise'),
      webglNoise: checked('#pWebglNoise'),
      disableWebrtc: checked('#pDisableWebrtc'),
      enableGeolocation: checked('#pEnableGeolocation'),
      chromeSpoof: checked('#pChromeSpoof'),
      webglVendor: val('#pWebglVendor'),
      webglRenderer: val('#pWebglRenderer'),
      automationTool: val('#pAutomationTool'),
      automationPort: parseInt(val('#pAutomationPort')) || 0,
      automationScript: val('#pAutomationScript'),
      hardwareConcurrency: parseInt(val('#pHardwareConcurrency')) || 8,
      maxTouchPoints: parseInt(val('#pMaxTouchPoints')) || 0,
      batteryLevel: parseFloat(val('#pBatteryLevel')) || 0.85,
      devicePixelRatio: parseFloat(val('#pDevicePixelRatio')) || 1.0,
      audioNoise: checked('#pAudioNoise'),
      clientRectNoise: checked('#pClientRectNoise'),
      pluginsSpoof: checked('#pPluginsSpoof')
    };
    try {
      await send('addProfile', data);
      closeModal();
      await refresh();
      setTimeout(() => send('repaint').catch(() => {}), 100);
    } catch (e) {
      console.error(e);
    }
  };

  newBtn.onclick = openModal;
  if (addActionBtn) {
    addActionBtn.onclick = () => {
      automationActions.push({ type: 'click', target: '', value: '', delay: 0 });
      renderActionBuilder();
      updateMacroFromActions();
    };
  }
  closeBtn.onclick = closeModal;
  cancelBtn.onclick = closeModal;
  modal.onclick = (e) => { if (e.target === modal) closeModal(); };

  searchInput.oninput = render;

  qs('#pAutomationTool').onchange = updateAutomationExample;
  qs('#pAutomationPort').oninput = updateAutomationExample;

  tabs.forEach(tab => {
    tab.onclick = () => {
      tabs.forEach(t => t.classList.remove('active'));
      tab.classList.add('active');
      const target = tab.dataset.tab;
      qsa('.tab-content').forEach(c => {
        c.classList.toggle('active', c.dataset.tab === target);
      });
      if (target === 'automation') updateAutomationExample();
    };
  });

  await refresh();
  setTimeout(() => send('repaint').catch(() => {}), 400);

  if (location.search.includes('auto=1')) {
    try {
      await send('addProfile', {
        name: 'Demo',
        proxy: '',
        userAgent: '',
        url: 'https://example.com',
        os: 'Windows',
        timezone: 'America/New_York',
        language: 'en-US',
        screenWidth: 1920,
        screenHeight: 1080,
        deviceMemoryGb: 8,
        latitude: 40.7128,
        longitude: -74.0060,
        accuracy: 10,
        canvasNoise: false,
        webglNoise: true,
        disableWebrtc: true,
        enableGeolocation: true,
        chromeSpoof: true,
        webglVendor: 'Google Inc. (NVIDIA)',
        webglRenderer: 'ANGLE (NVIDIA, NVIDIA GeForce GTX 1660 Ti Direct3D11 vs_5_0 ps_5_0, D3D11)',
        automationTool: 'none',
        automationPort: 0,
        automationScript: '',
        hardwareConcurrency: 8,
        maxTouchPoints: 0,
        batteryLevel: 0.85,
        devicePixelRatio: 1.0,
        audioNoise: false,
        clientRectNoise: false,
        pluginsSpoof: true
      });
      await refresh();
      setTimeout(() => send('repaint').catch(() => {}), 500);
    } catch (err) {
      console.error(err);
    }
  }

  if (location.search.includes('modal=1')) {
    setTimeout(() => {
      openModal();
      if (location.search.includes('tab=automation')) {
        const tab = qs('.tab[data-tab="automation"]');
        if (tab) tab.click();
      }
      if (location.search.includes('tab=fingerprint')) {
        const fpTab = qs('.tab[data-tab="fingerprint"]');
        if (fpTab) fpTab.click();
      }
      if (location.search.includes('tab=network')) {
        const netTab = qs('.tab[data-tab="network"]');
        if (netTab) netTab.click();
      }
      const params = new URLSearchParams(location.search);
      const autoTool = params.get('automationTool');
      const autoPort = params.get('automationPort');
      if (autoTool) {
        setVal('#pAutomationTool', autoTool);
      }
      if (autoPort) {
        setVal('#pAutomationPort', autoPort);
      }
      updateAutomationExample();
      setTimeout(() => send('repaint').catch(() => {}), 300);
    }, 600);
  }
}
