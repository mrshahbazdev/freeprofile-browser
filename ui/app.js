const qs = (s) => document.querySelector(s);

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
  qs('#loginBtn').onclick = async () => {
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

function profileSummary(p) {
  const parts = [
    p.proxy || 'no proxy',
    p.userAgent ? 'custom UA' : 'default UA',
    p.os,
    `${p.screenWidth}x${p.screenHeight}`,
    p.enableGeolocation ? `geo ${p.latitude},${p.longitude}` : 'no geo'
  ];
  return parts.join(' · ');
}

function escapeHtml(s) {
  const div = document.createElement('div');
  div.textContent = s;
  return div.innerHTML;
}

async function initDashboard() {
  const profileList = qs('#profileList');
  const form = qs('#profileForm');

  async function refresh() {
    const json = await send('getProfiles');
    const list = JSON.parse(json || '[]');
    profileList.innerHTML = '';
    list.forEach((p) => {
      const li = document.createElement('li');
      li.className = 'profile';
      li.innerHTML = `
        <div class="info">
          <div class="name">${escapeHtml(p.name)}</div>
          <div class="meta">${escapeHtml(profileSummary(p))}</div>
          <div class="meta">${escapeHtml(p.url || '')} · ${escapeHtml(p.timezone || '')} · ${p.disableWebrtc ? 'WebRTC off' : 'WebRTC on'}</div>
        </div>
        <div class="actions">
          <button class="small" data-id="${p.id}" data-action="launch">Launch</button>
          <button class="small danger" data-id="${p.id}" data-action="delete">Delete</button>
        </div>
      `;
      profileList.appendChild(li);
    });
  }

  profileList.onclick = async (e) => {
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

  form.onsubmit = async (e) => {
    e.preventDefault();
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
      webglRenderer: val('#pWebglRenderer')
    };
    await send('addProfile', data);
    form.reset();
    setVal('#pTimezone', 'America/New_York');
    setVal('#pLanguage', 'en-US');
    setVal('#pScreenWidth', '1920');
    setVal('#pScreenHeight', '1080');
    setVal('#pDeviceMemory', '8');
    setVal('#pLatitude', '40.7128');
    setVal('#pLongitude', '-74.0060');
    setVal('#pAccuracy', '10');
    setVal('#pUrl', 'https://www.google.com');
    setVal('#pWebglVendor', 'Google Inc. (NVIDIA)');
    setVal('#pWebglRenderer', 'ANGLE (NVIDIA, NVIDIA GeForce GTX 1660 Ti Direct3D11 vs_5_0 ps_5_0, D3D11)');
    setChecked('#pWebglNoise', true);
    setChecked('#pDisableWebrtc', true);
    setChecked('#pEnableGeolocation', true);
    setChecked('#pChromeSpoof', true);
    await refresh();
    setTimeout(() => send('repaint').catch(() => {}), 100);
  };

  await refresh();
  setTimeout(() => send('repaint').catch(() => {}), 500);

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
        webglRenderer: 'ANGLE (NVIDIA, NVIDIA GeForce GTX 1660 Ti Direct3D11 vs_5_0 ps_5_0, D3D11)'
      });
      await refresh();
      setTimeout(() => send('repaint').catch(() => {}), 500);
      const first = profileList.querySelector('[data-action="launch"]');
      if (first) first.click();
    } catch (err) {
      console.error(err);
    }
  }
}
