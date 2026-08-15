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
    hardwareConcurrency: 8,
    maxTouchPoints: 0,
    canvasNoise: {{CANVAS_NOISE}},
    webglNoise: {{WEBGL_NOISE}},
    webglVendor: "{{WEBGL_VENDOR}}",
    webglRenderer: "{{WEBGL_RENDERER}}",
    disableWebrtc: {{DISABLE_WEBRTC}},
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

  // Navigator basics
  define(navigator, 'userAgent', () => cfg.userAgent);
  define(navigator, 'platform', () => cfg.platform);
  define(navigator, 'language', () => cfg.language);
  define(navigator, 'languages', () => cfg.languages);
  define(navigator, 'hardwareConcurrency', () => cfg.hardwareConcurrency);
  define(navigator, 'maxTouchPoints', () => cfg.maxTouchPoints);
  define(navigator, 'webdriver', () => undefined);

  // Screen
  define(window, 'devicePixelRatio', () => cfg.devicePixelRatio);
  define(screen, 'width', () => cfg.screenWidth);
  define(screen, 'height', () => cfg.screenHeight);
  define(screen, 'availWidth', () => cfg.screenWidth);
  define(screen, 'availHeight', () => cfg.screenHeight - 40);
  define(screen, 'colorDepth', () => cfg.colorDepth);
  define(screen, 'pixelDepth', () => cfg.pixelDepth);

  // Plugins / mimeTypes
  function FakePlugins() {
    this.length = 3;
    this[0] = { name: 'Chrome PDF Plugin', filename: 'internal-pdf-viewer', description: 'Portable Document Format' };
    this[1] = { name: 'Widevine Content Decryption Module', filename: 'widevinecdmadapter.dll', description: 'Protected media decoder' };
    this[2] = { name: 'Native Client module', filename: 'internal-nacl-plugin', description: 'Native Client module' };
  }
  FakePlugins.prototype.item = function(index) { return this[index] || null; };
  FakePlugins.prototype.namedItem = function(name) {
    for (let i = 0; i < this.length; ++i) if (this[i].name === name) return this[i];
    return null;
  };
  FakePlugins.prototype.refresh = function() {};

  function FakeMimeTypes() {
    this.length = 2;
    this[0] = { type: 'application/pdf', suffixes: 'pdf', description: 'Portable Document Format', enabledPlugin: { name: 'Chrome PDF Plugin' } };
    this[1] = { type: 'application/x-google-chrome-pdf', suffixes: 'pdf', description: 'Portable Document Format', enabledPlugin: { name: 'Chrome PDF Plugin' } };
  }
  FakeMimeTypes.prototype.item = function(index) { return this[index] || null; };
  FakeMimeTypes.prototype.namedItem = function(name) {
    for (let i = 0; i < this.length; ++i) if (this[i].type === name) return this[i];
    return null;
  };

  const fakePlugins = new FakePlugins();
  const fakeMimeTypes = new FakeMimeTypes();
  define(navigator, 'plugins', () => fakePlugins);
  define(navigator, 'mimeTypes', () => fakeMimeTypes);

  // Timezone
  const origResolvedOptions = Intl.DateTimeFormat.prototype.resolvedOptions;
  Intl.DateTimeFormat.prototype.resolvedOptions = function() {
    const o = origResolvedOptions.call(this);
    o.timeZone = cfg.timezone;
    return o;
  };

  // Canvas noise
  if (cfg.canvasNoise) {
    const origGetImageData = CanvasRenderingContext2D.prototype.getImageData;
    function addNoise(data, width, height) {
      for (let y = 0; y < height; ++y) {
        for (let x = 0; x < width; ++x) {
          const i = (y * width + x) * 4;
          data[i]   = Math.max(0, Math.min(255, data[i]   + ((cfg.seed + x * 374761393 + y * 668265263 + 1) % 5) - 2));
          data[i+1] = Math.max(0, Math.min(255, data[i+1] + ((cfg.seed + x * 374761393 + y * 668265263 + 2) % 5) - 2));
          data[i+2] = Math.max(0, Math.min(255, data[i+2] + ((cfg.seed + x * 374761393 + y * 668265263 + 3) % 5) - 2));
        }
      }
      return data;
    }
    CanvasRenderingContext2D.prototype.getImageData = function(sx, sy, sw, sh) {
      const img = origGetImageData.call(this, sx, sy, sw, sh);
      addNoise(img.data, img.width, img.height);
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
      if (!ctx) return origToDataURL.call(canvas, type, quality);
      ctx.drawImage(canvas, 0, 0);
      const img = ctx.getImageData(0, 0, w, h);
      addNoise(img.data, w, h);
      ctx.putImageData(img, 0, 0);
      return tmp;
    }

    HTMLCanvasElement.prototype.toDataURL = function(type, quality) {
      const tmp = cloneCanvasWithNoise(this, type, quality);
      return origToDataURL.call(tmp, type, quality);
    };

    HTMLCanvasElement.prototype.toBlob = function(callback, type, quality) {
      const tmp = cloneCanvasWithNoise(this, type, quality);
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

  // WebRTC disable
  if (cfg.disableWebrtc) {
    define(navigator, 'mediaDevices', () => undefined);
    if (navigator.mediaDevices) {
      try {
        navigator.mediaDevices.getUserMedia = undefined;
        navigator.mediaDevices.enumerateDevices = undefined;
        navigator.mediaDevices.getDisplayMedia = undefined;
      } catch (e) {}
    }
    window.RTCPeerConnection = undefined;
    window.RTCSessionDescription = undefined;
    window.RTCIceCandidate = undefined;
    window.RTCDataChannel = undefined;
  }

  // chrome object presence
  if (typeof window.chrome === 'undefined') {
    try { window.chrome = { runtime: {} }; } catch (e) {}
  }
})();
