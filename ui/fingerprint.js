(function() {
  'use strict';
  window.__fp_injected = true;

  const cfg = {
    userAgent: "{{USER_AGENT}}",
    platform: "{{PLATFORM}}",
    language: "{{LANGUAGE}}",
    languages: ["{{LANGUAGE}}", "en"],
    timezone: "{{TIMEZONE}}",
    screenWidth: {{SCREEN_WIDTH}},
    screenHeight: {{SCREEN_HEIGHT}},
    colorDepth: 24,
    pixelDepth: 24,
    devicePixelRatio: 1,
    deviceMemory: {{DEVICE_MEMORY}},
    hardwareConcurrency: 8,
    maxTouchPoints: 0,
    canvasNoise: {{CANVAS_NOISE}},
    webglNoise: {{WEBGL_NOISE}},
    webglVendor: "{{WEBGL_VENDOR}}",
    webglRenderer: "{{WEBGL_RENDERER}}",
    disableWebrtc: {{DISABLE_WEBRTC}},
    enableGeolocation: {{ENABLE_GEOLOCATION}},
    chromeSpoof: {{CHROME_SPOOF}},
    latitude: {{LATITUDE}},
    longitude: {{LONGITUDE}},
    accuracy: {{ACCURACY}},
    seed: {{SEED}}
  };

  window.__fp_config = cfg;

  function define(obj, prop, getter) {
    try {
      Object.defineProperty(obj, prop, {
        get: getter,
        configurable: true,
        enumerable: true
      });
    } catch (e) {}
  }

  function defineConst(obj, prop, value) {
    try {
      Object.defineProperty(obj, prop, {
        get: () => value,
        configurable: true,
        enumerable: true
      });
    } catch (e) {}
  }

  // OS-specific font lists
  const fontsByOS = {
    Windows: [
      'Arial','Calibri','Cambria','Candara','Comic Sans MS','Consolas','Courier New',
      'Georgia','Impact','Lucida Console','Palatino Linotype','Segoe UI','Sitka Banner',
      'Tahoma','Times New Roman','Trebuchet MS','Verdana','Microsoft Sans Serif',
      'Microsoft YaHei','SimSun','Segoe UI Emoji','Segoe UI Symbol'
    ],
    macOS: [
      '.SF NS','.AppleSystemUIFont','Helvetica Neue','Helvetica','Arial','Arial Narrow',
      'Times','Times New Roman','Courier','Courier New','Georgia','Verdana','Tahoma',
      'Trebuchet MS','Impact','Comic Sans MS','Menlo','Monaco','PingFang SC',
      'STHeiti','Hiragino Kaku Gothic Pro','Apple Color Emoji','Apple Symbols'
    ],
    Linux: [
      'DejaVu Sans','DejaVu Serif','DejaVu Sans Mono','Liberation Sans','Liberation Serif',
      'Liberation Mono','Ubuntu','Ubuntu Mono','Noto Sans','Noto Serif','Noto Mono',
      'FreeSans','FreeSerif','FreeMono','Cantarell','Droid Sans','Droid Serif'
    ],
    Android: [
      'Roboto','Roboto Mono','Noto Sans','Noto Serif','Droid Sans','Droid Serif',
      'Droid Sans Mono','Arial','Times New Roman','Courier New','Georgia','Verdana'
    ],
    iOS: [
      '.SF UI Text','.SF UI Display','Helvetica Neue','Helvetica','Arial','Times New Roman',
      'Courier','Courier New','Georgia','Verdana','Menlo','Monaco','PingFang SC',
      'Hiragino Mincho ProN','Apple Color Emoji','Apple Symbols'
    ]
  };

  const osName = cfg.platform === 'MacIntel' ? 'macOS' :
                 cfg.platform === 'Linux x86_64' ? 'Linux' :
                 cfg.platform === 'Linux armv8l' ? 'Android' :
                 cfg.platform === 'iPhone' ? 'iOS' : 'Windows';
  const fonts = fontsByOS[osName] || fontsByOS.Windows;

  function randomInt(seed) {
    let h = 2166136261;
    h = Math.imul(h ^ seed, 16777619);
    h = Math.imul(h ^ (h >>> 16), 0x45d9f3b);
    h = Math.imul(h ^ (h >>> 13), 0xc2b2ae35);
    return (h ^ (h >>> 16)) >>> 0;
  }

  // Navigator basics
  define(navigator, 'userAgent', () => cfg.userAgent);
  define(navigator, 'platform', () => cfg.platform);
  define(navigator, 'language', () => cfg.language);
  define(navigator, 'languages', () => cfg.languages);
  defineConst(navigator, 'hardwareConcurrency', cfg.hardwareConcurrency);
  defineConst(navigator, 'maxTouchPoints', cfg.maxTouchPoints);
  define(navigator, 'webdriver', () => undefined);
  defineConst(navigator, 'vendor', 'Google Inc.');
  defineConst(navigator, 'vendorSub', '');
  defineConst(navigator, 'product', 'Gecko');
  defineConst(navigator, 'productSub', '20030107');
  defineConst(navigator, 'deviceMemory', cfg.deviceMemory);
  defineConst(navigator, 'pdfViewerEnabled', true);
  define(navigator, 'doNotTrack', () => null);

  // Screen
  define(window, 'devicePixelRatio', () => cfg.devicePixelRatio);
  define(window, 'outerWidth', () => cfg.screenWidth);
  define(window, 'outerHeight', () => cfg.screenHeight);
  define(window, 'innerWidth', () => cfg.screenWidth);
  define(window, 'innerHeight', () => cfg.screenHeight - 80);
  define(screen, 'width', () => cfg.screenWidth);
  define(screen, 'height', () => cfg.screenHeight);
  define(screen, 'availWidth', () => cfg.screenWidth);
  define(screen, 'availHeight', () => cfg.screenHeight - 40);
  define(screen, 'colorDepth', () => cfg.colorDepth);
  define(screen, 'pixelDepth', () => cfg.pixelDepth);
  define(screen, 'availLeft', () => 0);
  define(screen, 'availTop', () => 0);
  define(screen, 'orientation', () => ({ angle: 0, type: 'landscape-primary' }));

  // Plugins / mimeTypes
  function FakePlugins() {
    this.length = 3;
    this[0] = { name: 'Chrome PDF Plugin', filename: 'internal-pdf-viewer', description: 'Portable Document Format', version: undefined, itemType: 'type' };
    this[1] = { name: 'Widevine Content Decryption Module', filename: 'widevinecdmadapter.dll', description: 'Protected media decoder', version: undefined, itemType: 'type' };
    this[2] = { name: 'Native Client module', filename: 'internal-nacl-plugin', description: 'Native Client module', version: undefined, itemType: 'type' };
  }
  FakePlugins.prototype.item = function(index) { return this[index] || null; };
  FakePlugins.prototype.namedItem = function(name) {
    for (let i = 0; i < this.length; ++i) if (this[i].name === name) return this[i];
    return null;
  };
  FakePlugins.prototype.refresh = function() {};

  function FakeMimeTypes() {
    this.length = 2;
    this[0] = { type: 'application/pdf', suffixes: 'pdf', description: 'Portable Document Format', enabledPlugin: this[0] };
    this[1] = { type: 'application/x-google-chrome-pdf', suffixes: 'pdf', description: 'Portable Document Format', enabledPlugin: this[0] };
  }
  FakeMimeTypes.prototype.item = function(index) { return this[index] || null; };
  FakeMimeTypes.prototype.namedItem = function(name) {
    for (let i = 0; i < this.length; ++i) if (this[i].type === name) return this[i];
    return null;
  };

  define(navigator, 'plugins', () => new FakePlugins());
  define(navigator, 'mimeTypes', () => new FakeMimeTypes());

  // Fonts
  function FakeFontFace() {}
  FakeFontFace.prototype.load = function() { return Promise.resolve(this); };
  FakeFontFace.prototype.loaded = Promise.resolve();
  FakeFontFace.prototype.status = 'loaded';

  function FakeFontFaceSet(list) {
    const self = {};
    self.size = list.length;
    self.ready = Promise.resolve(self);
    self.status = 'loaded';
    self.check = function(family) {
      const f = (family || '').replace(/['"]/g, '').toLowerCase().split(',')[0].trim();
      return list.some(name => name.toLowerCase() === f || name.toLowerCase().indexOf(f) !== -1);
    };
    self.load = function(family) { return Promise.resolve([]); };
    self.has = function(family) { return self.check(family); };
    self.add = function() { return this; };
    self.delete = function() { return false; };
    self.clear = function() {};
    self.forEach = function(cb) { list.forEach(cb); };
    self.entries = function*() { for (let i=0;i<list.length;i++) yield [i,list[i]]; };
    self.keys = function*() { for (let i=0;i<list.length;i++) yield i; };
    self.values = function*() { for (let i=0;i<list.length;i++) yield list[i]; };
    self[Symbol.iterator] = self.entries;
    list.forEach((f, i) => { self[i] = new FakeFontFace(); });
    return self;
  }
  define(document, 'fonts', () => new FakeFontFaceSet(fonts));

  // Timezone
  const origResolvedOptions = Intl.DateTimeFormat.prototype.resolvedOptions;
  Intl.DateTimeFormat.prototype.resolvedOptions = function() {
    const o = origResolvedOptions.call(this);
    o.timeZone = cfg.timezone;
    return o;
  };

  // userAgentData (client hints)
  const uaFullVersion = (cfg.userAgent.match(/Chrome\/(\d+(\.\d+)*)/) || ['','134.0.0.0'])[1] || '134.0.0.0';
  const brands = [
    { brand: 'Chromium', version: uaFullVersion.split('.')[0] },
    { brand: 'Google Chrome', version: uaFullVersion.split('.')[0] },
    { brand: 'Not:A-Brand', version: '24' }
  ];
  const mobile = cfg.platform === 'Linux armv8l' || cfg.platform === 'iPhone';
  const uaData = {
    brands,
    mobile,
    platform: cfg.platform === 'iPhone' ? 'iPhone' : cfg.platform === 'Linux armv8l' ? 'Android' : cfg.platform === 'MacIntel' ? 'macOS' : cfg.platform === 'Linux x86_64' ? 'Linux' : 'Windows',
    getHighEntropyValues: function(hints) {
      const arch = cfg.platform === 'MacIntel' ? 'x86' : cfg.platform === 'Linux armv8l' || cfg.platform === 'iPhone' ? 'arm' : 'x86';
      const result = {
        architecture: arch,
        bitness: '64',
        model: '',
        platform: uaData.platform,
        platformVersion: cfg.platform === 'MacIntel' ? '13.5.0' : cfg.platform === 'Linux armv8l' ? '14.0' : cfg.platform === 'iPhone' ? '17.0' : cfg.platform === 'Linux x86_64' ? '6.5.0' : '10.0.22631',
        uaFullVersion,
        fullVersionList: brands
      };
      const out = {};
      if (Array.isArray(hints)) {
        hints.forEach(h => { if (result[h] !== undefined) out[h] = result[h]; });
      }
      return Promise.resolve(out);
    },
    toJSON: function() {
      return { brands, mobile, platform: uaData.platform };
    }
  };
  define(navigator, 'userAgentData', () => uaData);

  // Canvas noise
  if (cfg.canvasNoise) {
    const origGetImageData = CanvasRenderingContext2D.prototype.getImageData;
    function addNoise(data, width, height, seed) {
      for (let y = 0; y < height; ++y) {
        for (let x = 0; x < width; ++x) {
          const i = (y * width + x) * 4;
          data[i]   = Math.max(0, Math.min(255, data[i]   + ((seed + x * 374761393 + y * 668265263 + 1) % 5) - 2));
          data[i+1] = Math.max(0, Math.min(255, data[i+1] + ((seed + x * 374761393 + y * 668265263 + 2) % 5) - 2));
          data[i+2] = Math.max(0, Math.min(255, data[i+2] + ((seed + x * 374761393 + y * 668265263 + 3) % 5) - 2));
        }
      }
      return data;
    }
    CanvasRenderingContext2D.prototype.getImageData = function(sx, sy, sw, sh) {
      const img = origGetImageData.call(this, sx, sy, sw, sh);
      addNoise(img.data, img.width, img.height, cfg.seed);
      return img;
    };

    const origToDataURL = HTMLCanvasElement.prototype.toDataURL;
    const origToBlob = HTMLCanvasElement.prototype.toBlob;

    function cloneCanvasWithNoise(canvas, type, quality) {
      const w = canvas.width;
      const h = canvas.height;
      const tmp = document.createElement('canvas');
      tmp.width = w;
      tmp.height = h;
      const ctx = tmp.getContext('2d');
      if (!ctx) return null;
      ctx.drawImage(canvas, 0, 0);
      const img = ctx.getImageData(0, 0, w, h);
      addNoise(img.data, w, h, cfg.seed);
      ctx.putImageData(img, 0, 0);
      return tmp;
    }

    HTMLCanvasElement.prototype.toDataURL = function(type, quality) {
      const tmp = cloneCanvasWithNoise(this, type, quality);
      if (!tmp) return origToDataURL.call(this, type, quality);
      return origToDataURL.call(tmp, type, quality);
    };

    HTMLCanvasElement.prototype.toBlob = function(callback, type, quality) {
      const tmp = cloneCanvasWithNoise(this, type, quality);
      if (!tmp) return origToBlob.call(this, callback, type, quality);
      return origToBlob.call(tmp, callback, type, quality);
    };
  }

  // WebGL spoof
  if (cfg.webglNoise) {
    const GL_VENDOR = 0x1F00;
    const GL_RENDERER = 0x1F01;
    const GL_VERSION = 0x1F02;
    const GL_SHADING_LANGUAGE_VERSION = 0x8B8C;
    const UNMASKED_VENDOR = 0x9245;
    const UNMASKED_RENDERER = 0x9246;
    const origGetParameter = WebGLRenderingContext.prototype.getParameter;
    WebGLRenderingContext.prototype.getParameter = function(pname) {
      if (pname === UNMASKED_VENDOR || pname === GL_VENDOR) return cfg.webglVendor;
      if (pname === UNMASKED_RENDERER || pname === GL_RENDERER) return cfg.webglRenderer;
      if (pname === GL_VERSION) return 'WebGL 1.0 (OpenGL ES 2.0 Chromium)';
      if (pname === GL_SHADING_LANGUAGE_VERSION) return 'WebGL GLSL ES 1.0 (OpenGL ES GLSL ES 1.0 Chromium)';
      return origGetParameter.call(this, pname);
    };
  }

  // Geolocation spoof
  if (cfg.enableGeolocation) {
    const geo = {
      getCurrentPosition: function(success, error, options) {
        if (typeof success === 'function') {
          success({
            coords: {
              latitude: cfg.latitude,
              longitude: cfg.longitude,
              accuracy: cfg.accuracy,
              altitude: null,
              altitudeAccuracy: null,
              heading: null,
              speed: null
            },
            timestamp: Date.now()
          });
        }
      },
      watchPosition: function(success, error, options) {
        geo.getCurrentPosition(success, error, options);
        return 1;
      },
      clearWatch: function(id) {}
    };
    define(navigator, 'geolocation', () => geo);
  }

  // WebRTC disable
  if (cfg.disableWebrtc) {
    define(navigator, 'mediaDevices', () => undefined);
    navigator.getUserMedia = undefined;
    navigator.webkitGetUserMedia = undefined;
    navigator.mozGetUserMedia = undefined;
    window.RTCPeerConnection = undefined;
    window.RTCSessionDescription = undefined;
    window.RTCIceCandidate = undefined;
    window.RTCDataChannel = undefined;
    window.mozRTCPeerConnection = undefined;
    window.webkitRTCPeerConnection = undefined;
  }

  // Permissions API
  if (navigator.permissions && navigator.permissions.query) {
    const origQuery = navigator.permissions.query;
    navigator.permissions.query = function(param) {
      const name = (param && param.name) || '';
      if (cfg.disableWebrtc && (name === 'camera' || name === 'microphone')) {
        return Promise.resolve({ state: 'denied', onchange: null, addEventListener: ()=>{}, removeEventListener: ()=>{} });
      }
      return Promise.resolve({ state: name === 'notifications' ? 'default' : 'prompt', onchange: null, addEventListener: ()=>{}, removeEventListener: ()=>{} });
    };
  }

  // Connection
  const connection = {
    effectiveType: '4g',
    downlink: 10,
    downlinkMax: Infinity,
    rtt: 50,
    saveData: false,
    type: 'wifi',
    onchange: null,
    addEventListener: ()=>{},
    removeEventListener: ()=>{}
  };
  define(navigator, 'connection', () => connection);

  // Chrome object spoof
  if (cfg.chromeSpoof) {
    const chrome = {
      csi: function() { return { onloadT: Date.now(), pageT: 1, startE: Date.now() }; },
      loadTimes: function() {
        const now = Date.now() / 1000;
        return {
          commitLoadTime: now,
          connectionInfo: 'h2',
          finishDocumentLoadTime: now,
          finishLoadTime: now,
          firstPaintAfterLoadTime: 0,
          firstPaintTime: now,
          navigationType: 'Other',
          npnNegotiatedProtocol: 'h2',
          requestTime: now,
          startLoadTime: now,
          wasAlternateProtocolAvailable: false,
          wasFetchedViaSpdy: false,
          wasNpnNegotiated: true
        };
      },
      app: {
        isInstalled: false,
        getDetails: function() { return null; },
        getIsInstalled: function() { return false; },
        installState: function() { return { data: 'not_installed' }; },
        runningState: function() { return { data: 'cannot_run' }; },
        InstallState: { DISABLED: 'disabled', INSTALLED: 'installed', NOT_INSTALLED: 'not_installed' },
        RunningState: { CANNOT_RUN: 'cannot_run', READY_TO_RUN: 'ready_to_run', RUNNING: 'running' }
      },
      runtime: {
        OnInstalledReason: { BROWSER_UPDATE: 'browser_update', CHROME_UPDATE: 'chrome_update', INSTALL: 'install', SHARED_MODULE_UPDATE: 'shared_module_update', UPDATE: 'update' },
        OnRestartRequiredReason: { APP_UPDATE: 'app_update', OS_UPDATE: 'os_update', PERIODIC: 'periodic' },
        PlatformArch: { ARM: 'arm', ARM64: 'arm64', MIPS: 'mips', MIPS64: 'mips64', X32: 'x32', X64: 'x64', MIPSEL: 'mipsel', MIPSEL64: 'mipsel64' },
        PlatformNaclArch: { ARM: 'arm', MIPS: 'mips', MIPS64: 'mips64', MIPS32: 'mips32', MIPSEL: 'mipsel', MIPSEL64: 'mipsel64', X86_32: 'x86-32', X86_64: 'x86-64' },
        PlatformOs: { ANDROID: 'android', CROS: 'cros', LINUX: 'linux', MAC: 'mac', OPENBSD: 'openbsd', WINDOWS: 'windows' },
        RequestUpdateCheckStatus: { NO_UPDATE: 'no_update', THROTTLED: 'throttled', UPDATE_AVAILABLE: 'update_available' }
      }
    };
    if (typeof window.chrome === 'undefined') {
      try { window.chrome = chrome; } catch (e) {}
    } else if (window.chrome && typeof window.chrome === 'object') {
      for (const k in chrome) {
        if (!(k in window.chrome)) window.chrome[k] = chrome[k];
      }
    }
  }

  // Notification permission
  if (typeof Notification !== 'undefined') {
    try { Notification.permission = 'default'; } catch (e) {}
  }

  // Hide inconsistent hardware APIs
  const hideApis = ['bluetooth','usb','hid','serial','keyboard','presentation','wakeLock','windowControlsOverlay','xr','scheduling','contacts','managed'];
  hideApis.forEach(api => {
    if (navigator[api] !== undefined) {
      try { delete navigator[api]; } catch (e) {}
    }
  });

  // Memory
  if (typeof performance !== 'undefined') {
    define(performance, 'memory', () => ({
      usedJSHeapSize: 24000000,
      totalJSHeapSize: 32000000,
      jsHeapSizeLimit: cfg.deviceMemory * 268435456
    }));
  }
})();
