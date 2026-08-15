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
          <div class="meta">${p.proxy || 'no proxy'} · ${p.userAgent ? 'custom UA' : 'default UA'} · ${escapeHtml(p.url || '')}</div>
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
    try {
      if (btn.dataset.action === 'delete') {
        await send('deleteProfile', { id });
        await refresh();
      } else if (btn.dataset.action === 'launch') {
        await send('launchProfile', { id });
      }
    } catch (err) {
      alert(err);
    }
  };

  form.onsubmit = async (e) => {
    e.preventDefault();
    const data = {
      name: qs('#pName').value.trim(),
      proxy: qs('#pProxy').value.trim(),
      userAgent: qs('#pUA').value.trim(),
      url: qs('#pUrl').value.trim() || 'https://www.google.com'
    };
    if (!data.name) return;
    try {
      await send('addProfile', data);
      form.reset();
      qs('#pUrl').value = 'https://www.google.com';
      await refresh();
    } catch (err) {
      alert(err);
    }
  };

  await refresh();
  setTimeout(() => send('repaint').catch(() => {}), 500);

  if (location.search.includes('auto=1')) {
    try {
      await send('addProfile', { name: 'Demo', proxy: '', userAgent: '', url: 'https://example.com' });
      await refresh();
      setTimeout(() => send('repaint').catch(() => {}), 500);
      const first = profileList.querySelector('[data-action="launch"]');
      if (first) first.click();
    } catch (err) {
      console.error(err);
    }
  }
}

function escapeHtml(s) {
  return (s || '').replace(/[&<>"']/g, (c) => ({ '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;', "'": '&#39;' }[c]));
}
