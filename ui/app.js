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
    ? `<span class="badge os">geo</span>`
    : '';
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
        <span class="badge os">${escapeHtml(p.os || 'Windows')}</span>
        ${proxyBadge}
        <span class="badge dim">${escapeHtml(screen)}</span>
        ${webrtcBadge}
        ${geoBadge}
      </div>
      <div class="profile-meta">
        <div>${escapeHtml(p.timezone || '')} &middot; ${escapeHtml(p.language || 'en-US')}</div>
        <div>${p.proxy ? escapeHtml(p.proxy) : 'No proxy'} &middot; ${p.enableGeolocation ? `geo ${p.latitude}, ${p.longitude}` : 'no geo'}</div>
      </div>
      <div class="profile-actions">
        <button class="small" data-action="launch" data-id="${p.id}">Launch</button>
        <button class="small danger" data-action="delete" data-id="${p.id}">Delete</button>
      </div>
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
    setChecked('#pWebglNoise', true);
    setChecked('#pDisableWebrtc', true);
    setChecked('#pEnableGeolocation', true);
    setChecked('#pChromeSpoof', true);
    setChecked('#pCanvasNoise', false);
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
      webglRenderer: val('#pWebglRenderer')
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
  closeBtn.onclick = closeModal;
  cancelBtn.onclick = closeModal;
  modal.onclick = (e) => { if (e.target === modal) closeModal(); };

  searchInput.oninput = render;

  tabs.forEach(tab => {
    tab.onclick = () => {
      tabs.forEach(t => t.classList.remove('active'));
      tab.classList.add('active');
      const target = tab.dataset.tab;
      qsa('.tab-content').forEach(c => {
        c.classList.toggle('active', c.dataset.tab === target);
      });
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
        webglRenderer: 'ANGLE (NVIDIA, NVIDIA GeForce GTX 1660 Ti Direct3D11 vs_5_0 ps_5_0, D3D11)'
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
      if (location.search.includes('tab=fingerprint')) {
        const fpTab = qs('.tab[data-tab="fingerprint"]');
        if (fpTab) fpTab.click();
      }
      if (location.search.includes('tab=network')) {
        const netTab = qs('.tab[data-tab="network"]');
        if (netTab) netTab.click();
      }
      setTimeout(() => send('repaint').catch(() => {}), 300);
    }, 600);
  }
}
