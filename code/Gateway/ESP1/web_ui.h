#pragma once

// The full web interface is embedded in flash as a single HTML document so the gateway has no external assets.
static const char WEB_UI_HTML[] PROGMEM = R"rawliteral(
<!doctype html>
<html lang="en" data-theme="dark">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>CapiBridge Control</title>
  <style>
    /* CSS variables and component styles for the embedded control panel. */
    :root{--bg-primary:#0a0e14;--bg-secondary:#111821;--bg-tertiary:#1a232f;--bg-card:#151d28;--bg-card-hover:#1a2636;--border-primary:#2a3744;--border-accent:#3d4f5f;--text-primary:#e6edf3;--text-secondary:#8b949e;--text-muted:#6e7681;--accent-primary:#f0883e;--accent-secondary:#db6d28;--accent-glow:rgba(240,136,62,0.16);--success:#3fb950;--success-glow:rgba(63,185,80,0.18);--warning:#d29922;--warning-glow:rgba(210,153,34,0.18);--danger:#f85149;--danger-glow:rgba(248,81,73,0.18);--shadow-md:0 4px 12px rgba(0,0,0,0.5);--ui-font-scale:1}
    [data-theme="light"]{--bg-primary:#f6f8fa;--bg-secondary:#ffffff;--bg-tertiary:#ebeef1;--bg-card:#ffffff;--bg-card-hover:#f3f6f9;--border-primary:#d0d7de;--border-accent:#a8b3bd;--text-primary:#1f2328;--text-secondary:#656d76;--text-muted:#8b949e;--accent-primary:#d35400;--accent-secondary:#b84700;--accent-glow:rgba(211,84,0,0.14);--success:#1a7f37;--success-glow:rgba(26,127,55,0.14);--warning:#9a6700;--warning-glow:rgba(154,103,0,0.14);--danger:#cf222e;--danger-glow:rgba(207,34,46,0.14);--shadow-md:0 4px 12px rgba(0,0,0,0.1)}
    *{margin:0;padding:0;box-sizing:border-box}
    body{font-family:-apple-system,BlinkMacSystemFont,"Segoe UI",sans-serif;background:var(--bg-primary);color:var(--text-primary);min-height:100vh;line-height:1.5;font-size:calc(15px * var(--ui-font-scale));transition:background .3s,color .3s}
    a{color:inherit}
    .app{display:flex;min-height:100vh;position:relative}
    .sidebar{width:240px;background:var(--bg-secondary);border-right:1px solid var(--border-primary);display:flex;flex-direction:column;position:fixed;height:100vh;transition:transform .3s;z-index:100}
    .sidebar-header{padding:16px;border-bottom:1px solid var(--border-primary)}
    .logo{display:flex;align-items:center;gap:10px}.logo-icon{width:36px;height:36px;background:linear-gradient(135deg,var(--accent-primary),var(--accent-secondary));border-radius:8px;display:flex;align-items:center;justify-content:center;color:#fff;font-size:calc(18px * var(--ui-font-scale));font-weight:700}.logo-text{display:flex;flex-direction:column}.logo-title{font-size:calc(15px * var(--ui-font-scale));font-weight:700}.logo-subtitle{font-size:calc(10px * var(--ui-font-scale));color:var(--text-secondary);text-transform:uppercase;letter-spacing:1px}
    .conn-status{display:flex;align-items:center;gap:8px;padding:8px 12px;margin:12px;background:var(--bg-tertiary);border-radius:6px;border:1px solid var(--border-primary)}
    .status-led{width:6px;height:6px;border-radius:50%;background:var(--success);animation:pulse 2s infinite}.status-led.warn{background:var(--warning)}.status-led.danger{background:var(--danger)}
    @keyframes pulse{0%,100%{opacity:1}50%{opacity:.5}}
    .status-text{font-size:calc(12px * var(--ui-font-scale));color:var(--text-secondary)}
    .nav-section{padding:4px 12px}.nav-label{font-size:calc(10px * var(--ui-font-scale));font-weight:600;color:var(--text-muted);text-transform:uppercase;letter-spacing:1px;padding:8px 6px 4px}
    .nav-item{display:flex;align-items:center;gap:8px;padding:8px 12px;border-radius:6px;color:var(--text-secondary);font-size:calc(13px * var(--ui-font-scale));font-weight:500;cursor:pointer;border:1px solid transparent;margin-bottom:2px;transition:all .2s;text-decoration:none}
    .nav-item:hover{background:var(--bg-tertiary);color:var(--text-primary)}.nav-item.active{background:var(--accent-glow);color:var(--accent-primary);border-color:var(--accent-primary)}
    .nav-item svg{width:16px;height:16px;flex-shrink:0;stroke:currentColor;fill:none;stroke-width:2}
    .sidebar-footer{margin-top:auto;padding:12px;border-top:1px solid var(--border-primary)}
    .theme-toggle{display:flex;align-items:center;justify-content:space-between;padding:8px 12px;background:var(--bg-tertiary);border-radius:6px;border:1px solid var(--border-primary)}
    .theme-label{font-size:calc(12px * var(--ui-font-scale));color:var(--text-secondary);display:flex;align-items:center;gap:6px}.theme-label svg{width:14px;height:14px}
    .toggle-sw{width:40px;height:22px;background:var(--bg-card);border-radius:11px;cursor:pointer;position:relative;border:2px solid var(--border-primary);transition:all .3s}.toggle-sw::after{content:'';position:absolute;width:14px;height:14px;background:var(--accent-primary);border-radius:50%;top:2px;left:2px;transition:transform .3s}
    [data-theme="dark"] .toggle-sw::after{transform:translateX(18px)}
    .mobile-menu{display:none;position:fixed;top:12px;left:12px;z-index:200;width:36px;height:36px;background:var(--bg-secondary);border:1px solid var(--border-primary);border-radius:8px;cursor:pointer;align-items:center;justify-content:center}.mobile-menu svg{width:20px;height:20px;stroke:currentColor;fill:none;stroke-width:2;color:var(--text-primary)}
    .sidebar-overlay{display:none;position:fixed;inset:0;background:rgba(0,0,0,.5);z-index:99}.sidebar-overlay.active{display:block}
    .main{flex:1;margin-left:240px;padding:20px;min-height:100vh}
    .page{display:none;animation:fadeIn .3s}.page.active{display:block}@keyframes fadeIn{from{opacity:0;transform:translateY(6px)}to{opacity:1;transform:translateY(0)}}
    .page-header{margin-bottom:20px}.page-title{font-size:calc(22px * var(--ui-font-scale));font-weight:700;margin-bottom:4px}.page-desc{color:var(--text-secondary);font-size:calc(14px * var(--ui-font-scale))}
    .card{background:var(--bg-card);border:1px solid var(--border-primary);border-radius:10px;padding:16px;margin-bottom:16px;transition:all .3s;box-shadow:var(--shadow-md)}.card:hover{border-color:var(--border-accent)}
    .card-header{display:flex;align-items:center;justify-content:space-between;gap:12px;margin-bottom:14px;padding-bottom:12px;border-bottom:1px solid var(--border-primary)}
    .card-title{font-size:calc(15px * var(--ui-font-scale));font-weight:600;display:flex;align-items:center;gap:6px}.card-title svg{width:16px;height:16px;stroke:currentColor;fill:none;stroke-width:2;color:var(--accent-primary)}
    .grid-2{display:grid;grid-template-columns:repeat(2,1fr);gap:16px}.status-grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(140px,1fr));gap:12px}
    .status-item{background:var(--bg-tertiary);border:1px solid var(--border-primary);border-radius:6px;padding:12px;display:flex;flex-direction:column;gap:4px}.status-label{font-size:calc(11px * var(--ui-font-scale));font-weight:700;color:var(--text-muted);text-transform:uppercase;letter-spacing:1px}.status-value{font-family:monospace;font-size:calc(14px * var(--ui-font-scale));font-weight:600;color:var(--text-primary)}.status-value.hl{color:var(--accent-primary)}
    .badge{display:inline-flex;align-items:center;gap:4px;padding:2px 8px;border-radius:12px;font-size:10px;font-weight:600}.badge.success{background:var(--success-glow);color:var(--success)}.badge.warning{background:var(--warning-glow);color:var(--warning)}.badge.danger{background:var(--danger-glow);color:var(--danger)}.badge::before{content:'';width:4px;height:4px;border-radius:50%;background:currentColor}
    .form-group{margin-bottom:14px}.form-label{display:block;font-size:calc(12px * var(--ui-font-scale));font-weight:700;color:var(--text-secondary);margin-bottom:4px}.form-input,.form-select,.form-textarea{width:100%;padding:8px 12px;background:var(--bg-tertiary);border:1px solid var(--border-primary);border-radius:6px;color:var(--text-primary);font-family:monospace;font-size:calc(13px * var(--ui-font-scale));transition:all .2s}.form-textarea{min-height:110px;resize:vertical}
    .form-input:focus,.form-select:focus,.form-textarea:focus{outline:none;border-color:var(--accent-primary)}.form-hint{font-size:10px;color:var(--text-muted);margin-top:3px}.form-hint.warning{color:var(--warning);display:flex;align-items:center;gap:4px;padding:8px;background:var(--warning-glow);border-radius:6px;margin-bottom:14px}.toggle-group{display:flex;align-items:center;justify-content:space-between;padding:12px;background:var(--bg-tertiary);border:1px solid var(--border-primary);border-radius:6px;margin-bottom:10px}.toggle-info{display:flex;flex-direction:column;gap:2px;padding-right:12px}.toggle-title{font-size:calc(13px * var(--ui-font-scale));font-weight:600;color:var(--text-primary)}.toggle-desc{font-size:calc(11px * var(--ui-font-scale));color:var(--text-muted)}.toggle-btn{width:44px;height:24px;background:var(--bg-primary);border-radius:12px;cursor:pointer;position:relative;border:2px solid var(--border-primary);transition:all .3s;flex-shrink:0}.toggle-btn::after{content:'';position:absolute;width:16px;height:16px;background:var(--text-muted);border-radius:50%;top:2px;left:2px;transition:all .3s}.toggle-btn.active{background:var(--accent-glow);border-color:var(--accent-primary)}.toggle-btn.active::after{background:var(--accent-primary);transform:translateX(20px)}
    .btn{display:inline-flex;align-items:center;justify-content:center;gap:6px;padding:8px 16px;border-radius:6px;font-size:calc(13px * var(--ui-font-scale));font-weight:600;cursor:pointer;transition:all .2s;border:none}.btn-primary{background:linear-gradient(135deg,var(--accent-primary),var(--accent-secondary));color:#fff}.btn-primary:hover{transform:translateY(-1px)}.btn-secondary{background:var(--bg-tertiary);color:var(--text-primary);border:1px solid var(--border-primary)}.btn-secondary:hover{background:var(--bg-card-hover)}.btn-danger{background:var(--danger);color:#fff}.btn-danger:hover{transform:translateY(-1px)}.btn-warning{background:var(--warning);color:#fff}.btn-group{display:flex;gap:8px;flex-wrap:wrap}
    .message{margin-top:12px;padding:10px 12px;border-radius:6px;font-size:11px;display:none}.message.success{background:var(--success-glow);color:var(--success)}.message.error{background:var(--danger-glow);color:var(--danger)}
    .traffic-toolbar{display:grid;grid-template-columns:repeat(auto-fit,minmax(180px,1fr));gap:10px;margin-bottom:14px}.traffic-list{display:grid;grid-template-columns:repeat(3,minmax(0,1fr));gap:14px;align-items:start}.traffic-card{padding:16px;border-radius:8px;background:var(--bg-tertiary);border:1px solid var(--border-primary);min-width:0}.traffic-head{display:flex;justify-content:space-between;gap:12px;flex-wrap:wrap;margin-bottom:10px}.traffic-badges{display:flex;gap:8px;flex-wrap:wrap;align-items:center}.traffic-meta{color:var(--text-muted);font-size:10px;font-family:monospace}.traffic-pre{margin:0;padding:14px;border-radius:8px;background:rgba(15,23,42,.22);overflow-x:auto;white-space:pre-wrap;word-break:break-word}.traffic-terminal{padding:12px 14px;border-radius:10px;border:1px solid var(--border-primary);background:#05080d;color:#8bffb0;font-family:monospace;font-size:calc(11px * var(--ui-font-scale));line-height:1.45;max-height:38vh;min-height:120px;overflow-y:auto;overflow-x:auto}.traffic-terminal-line{white-space:nowrap;margin-bottom:2px}.traffic-terminal-line:last-child{margin-bottom:0}.empty-state{padding:24px;border-radius:16px;border:1px dashed var(--border-primary);color:var(--text-muted);text-align:center}.node-grid{display:flex;flex-wrap:wrap;gap:12px;align-items:flex-start;justify-content:flex-start;align-content:flex-start;max-height:270px;overflow-y:auto;padding-right:4px}.node-card{background:var(--bg-tertiary);border:1px solid var(--border-primary);border-radius:12px;padding:14px;display:inline-grid;gap:10px;flex:0 0 auto;width:fit-content;min-width:0;max-width:100%}.node-card-header{display:flex;align-items:flex-start;justify-content:flex-start;gap:10px}.node-name{font-size:calc(14px * var(--ui-font-scale));font-weight:700;color:var(--text-primary);word-break:break-word}.node-meta{font-size:calc(11px * var(--ui-font-scale));color:var(--text-muted)}.node-badges{display:flex;flex-wrap:wrap;gap:6px}.node-card.status-valid{border-color:rgba(63,185,80,.34)}.node-card.status-wrong_key,.node-card.status-invalid_json,.node-card.status-missing_id{border-color:rgba(248,81,73,.24)}
    .badge.source-lora{background:var(--accent-glow);color:var(--accent-primary)}.badge.source-espnow{background:rgba(59,130,246,.14);color:#58a6ff}.badge.status-valid{background:var(--success-glow);color:var(--success)}.badge.status-invalid_json,.badge.status-wrong_key,.badge.status-missing_id{background:var(--danger-glow);color:var(--danger)}.badge.status-mqtt_offline{background:var(--warning-glow);color:var(--warning)}.badge.node-name{background:rgba(168,85,247,.14);color:#c084fc}.traffic-card.source-lora{box-shadow:inset 0 0 0 1px rgba(240,136,62,.08)}.traffic-card.source-espnow{box-shadow:inset 0 0 0 1px rgba(88,166,255,.08)}.traffic-card.status-valid{border-color:rgba(63,185,80,.35)}.traffic-card.status-invalid_json,.traffic-card.status-wrong_key,.traffic-card.status-missing_id{border-color:rgba(248,81,73,.35)}.traffic-card.status-mqtt_offline{border-color:rgba(210,153,34,.35)}
    .link-list{display:grid;gap:10px}.link-row{display:flex;justify-content:space-between;gap:16px;padding:12px 14px;border-radius:14px;background:var(--bg-tertiary);border:1px solid var(--border-primary)}.link-row span:first-child{font-size:calc(12px * var(--ui-font-scale));color:var(--text-muted)}.link-row a{color:var(--accent-primary);text-decoration:none;word-break:break-all}.link-row a:hover{text-decoration:underline}.info-feed-state{font-size:calc(12px * var(--ui-font-scale));color:var(--text-muted);margin-bottom:10px}.info-feed{display:grid;gap:10px;max-height:70vh;overflow:auto;padding-right:4px}.info-feed h1,.info-feed h2,.info-feed h3{font-size:calc(16px * var(--ui-font-scale));line-height:1.3}.info-feed p,.info-feed li{font-size:calc(13px * var(--ui-font-scale));color:var(--text-secondary)}.info-feed ul{padding-left:18px}.info-feed code{font-family:monospace;background:var(--bg-tertiary);border:1px solid var(--border-primary);border-radius:6px;padding:1px 5px;color:var(--text-primary)}.info-feed pre{margin:0;padding:12px 14px;background:var(--bg-tertiary);border:1px solid var(--border-primary);border-radius:10px;overflow:auto}.info-feed pre code{padding:0;border:none;background:transparent}.ui-config-grid{display:grid;grid-template-columns:repeat(2,minmax(0,1fr));gap:16px}.color-input{height:42px;padding:6px;cursor:pointer}.ui-config-note{margin-top:8px}
    @media(max-width:900px){.grid-2{grid-template-columns:1fr}.traffic-list{grid-template-columns:1fr}}
    @media(max-width:768px){.sidebar{transform:translateX(-100%)}.sidebar.open{transform:translateX(0)}.mobile-menu{display:flex}.main{margin-left:0;padding:60px 12px 16px}.page-title{font-size:18px}.status-grid{grid-template-columns:1fr}}
  </style>
</head>
<body>
  <div class="sidebar-overlay" onclick="toggleSidebar()"></div>
  <button class="mobile-menu" onclick="toggleSidebar()"><svg viewBox="0 0 24 24"><path d="M3 12h18M3 6h18M3 18h18"/></svg></button>
  <div class="app">
    <aside class="sidebar" id="sidebar">
      <div class="sidebar-header">
        <div class="logo">
          <div class="logo-icon">CB</div>
          <div class="logo-text"><span class="logo-title">CapiBridge</span><span class="logo-subtitle">by PricelessToolkit</span></div>
        </div>
      </div>
      <div class="conn-status"><div class="status-led" id="sidebarLed"></div><span class="status-text" id="sidebarStatus">Loading status...</span></div>
      <nav class="nav-section">
        <div class="nav-label">Main</div>
        <a class="nav-item active" data-page="status" href="#/status" onclick="navigateTo('status');return false;"><svg viewBox="0 0 24 24"><rect x="3" y="3" width="18" height="18" rx="2"/><path d="M3 9h18M9 21V9"/></svg>Status</a>
        <a class="nav-item" data-page="monitoring" href="#/monitoring" onclick="navigateTo('monitoring');return false;"><svg viewBox="0 0 24 24"><path d="M3 12h4l3 8 4-16 3 8h4"/></svg>Monitoring</a>
        <a class="nav-item" data-page="settings" href="#/settings" onclick="navigateTo('settings');return false;"><svg viewBox="0 0 24 24"><path d="M12 15.5A3.5 3.5 0 1 0 12 8.5a3.5 3.5 0 0 0 0 7Z"/><path d="M19.4 15a1.65 1.65 0 0 0 .33 1.82"/></svg>Settings</a>
        <a class="nav-item" data-page="info" href="#/info" onclick="navigateTo('info');return false;"><svg viewBox="0 0 24 24"><circle cx="12" cy="12" r="10"/><path d="M12 16v-4M12 8h.01"/></svg>Help</a>
      </nav>
      <div class="sidebar-footer"><div class="theme-toggle"><span class="theme-label"><svg viewBox="0 0 24 24" fill="currentColor"><path d="M12 22c5.523 0 10-4.477 10-10S17.523 2 12 2 2 6.477 2 12s4.477 10 10 10Zm0-1.5v-17a8.5 8.5 0 1 1 0 17Z"/></svg>Theme</span><div class="toggle-sw" onclick="toggleTheme()"></div></div></div>
    </aside>
    <main class="main">
      <!-- Status page: connection health, radio details, and maintenance actions. -->
      <section class="page active" id="page-status">
        <div class="page-header"><h1 class="page-title">System Status</h1><p class="page-desc">Overview of your CapiBridge gateway.</p></div>
        <div class="card"><div class="card-header"><h3 class="card-title"><svg viewBox="0 0 24 24"><path d="M22 11.08V12a10 10 0 1 1-5.93-9.14"/><path d="M22 4 12 14.01l-3-3"/></svg>Connection</h3><span class="badge success" id="connectionBadge">Checking</span></div><div class="status-grid" id="dashboardGrid"></div></div>
        <div class="grid-2"><div class="card"><div class="card-header"><h3 class="card-title"><svg viewBox="0 0 24 24"><path d="M4.9 19.1C1 15.2 1 8.8 4.9 4.9m2.9 11.3c-2.3-2.3-2.3-6.1 0-8.5"/><circle cx="12" cy="12" r="2"/><path d="M16.2 7.8c2.3 2.3 2.3 6.1 0 8.5m2.9-11.4C23 8.8 23 15.1 19.1 19"/></svg>LoRa Radio</h3></div><div class="status-grid" id="radioGrid"></div></div><div class="card"><div class="card-header"><h3 class="card-title"><svg viewBox="0 0 24 24"><path d="M3 3v18h18"/><path d="M7 16V9M12 16V5M17 16v-3"/></svg>Runtime</h3></div><div class="status-grid" id="runtimeGrid"></div></div></div>
      <div class="card"><div class="card-header"><h3 class="card-title"><svg viewBox="0 0 24 24"><path d="M4 12h16"/><path d="M7 8h10"/><path d="M9 16h6"/><circle cx="6" cy="12" r="1"/><circle cx="18" cy="12" r="1"/></svg>Last Seen Nodes</h3><span class="badge warning">Live cache</span></div><div class="node-grid" id="workingNodesGrid"></div></div><div class="card"><div class="card-header"><h3 class="card-title"><svg viewBox="0 0 24 24"><path d="M23 4v6h-6M1 20v-6h6"/><path d="M3.51 9a9 9 0 0 1 14.85-3.36L23 10M1 14l4.64 4.36A9 9 0 0 0 20.49 15"/></svg>Maintenance</h3></div><div class="btn-group"><button class="btn btn-warning" type="button" id="refreshAll">Refresh</button><button class="btn btn-secondary" type="button" id="rebootButton">Reboot Gateway</button><button class="btn btn-danger" type="button" id="defaultsButton">Restore Defaults</button></div><div class="message" id="actionsMessage"></div></div>
      </section>

      <!-- Monitoring page: prettified packet summaries plus the mirrored serial console. -->
      <section class="page" id="page-monitoring">
        <div class="page-header"><h1 class="page-title">Monitoring</h1><p class="page-desc">Live payload monitoring and mirrored serial output.</p></div>
        <div class="card"><div class="card-header"><h3 class="card-title"><svg viewBox="0 0 24 24"><path d="M3 12h4l3 8 4-16 3 8h4"/></svg>Prettified Payloads</h3><span class="badge warning">RAM buffer</span></div><div class="traffic-toolbar"><select class="form-select" id="trafficSourceFilter"><option value="all">All sources</option><option value="LoRa">LoRa</option><option value="ESP-NOW">ESP-NOW</option></select><select class="form-select" id="trafficStatusFilter"><option value="all">All statuses</option><option value="valid">Valid</option><option value="wrong_key">Wrong key</option><option value="missing_id">Missing ID</option><option value="invalid_json">Invalid JSON</option><option value="mqtt_offline">MQTT offline</option></select></div><div class="traffic-list" id="trafficList"></div></div><div class="card traffic-terminal-wrap"><div class="card-header"><h3 class="card-title"><svg viewBox="0 0 24 24"><path d="M8 9l3 3-3 3M13 15h3"/></svg>Raw View</h3><span class="badge warning">Live stream</span></div><div class="traffic-terminal" id="trafficTerminal"></div></div>
      </section>

      <!-- Settings page: runtime-editable WiFi, MQTT, gateway, encryption, radio, and UI preferences. -->
      <section class="page" id="page-settings">
        <div class="page-header"><h1 class="page-title">Settings</h1><p class="page-desc">Edit WiFi, MQTT, gateway, encryption, and LoRa settings.</p></div>
        <form id="settingsForm">
          <div class="grid-2">
            <div class="card"><div class="card-header"><h3 class="card-title"><svg viewBox="0 0 24 24"><path d="M5 12.55a11 11 0 0 1 14.08 0M1.42 9a16 16 0 0 1 21.16 0M8.53 16.11a6 6 0 0 1 6.95 0"/><circle cx="12" cy="20" r="1"/></svg>WiFi</h3></div><div class="grid-2"><div class="form-group"><label class="form-label" for="wifiSsid">SSID</label><input class="form-input" id="wifiSsid" maxlength="32"></div><div class="form-group"><label class="form-label" for="wifiPassword">Password</label><input class="form-input" id="wifiPassword" maxlength="64"></div></div></div>
            <div class="card"><div class="card-header"><h3 class="card-title"><svg viewBox="0 0 24 24"><path d="M12 2 2 7l10 5 10-5-10-5Z"/><path d="M2 17l10 5 10-5M2 12l10 5 10-5"/></svg>MQTT</h3></div><div class="grid-2"><div class="form-group"><label class="form-label" for="mqttServer">Server</label><input class="form-input" id="mqttServer" maxlength="64"></div><div class="form-group"><label class="form-label" for="mqttPort">Port</label><input class="form-input" id="mqttPort" type="number" min="1" max="65535"></div></div><div class="grid-2"><div class="form-group"><label class="form-label" for="mqttUsername">Username</label><input class="form-input" id="mqttUsername" maxlength="64"></div><div class="form-group"><label class="form-label" for="mqttPassword">Password</label><input class="form-input" id="mqttPassword" maxlength="64"></div></div></div>
          </div>
          <div class="grid-2">
            <div class="card"><div class="card-header"><h3 class="card-title"><svg viewBox="0 0 24 24"><path d="M12 15.5A3.5 3.5 0 1 0 12 8.5a3.5 3.5 0 0 0 0 7Z"/><path d="M19.4 15a1.65 1.65 0 0 0 .33 1.82"/></svg>Gateway</h3></div><div class="form-group"><label class="form-label" for="gatewayKey">Gateway Key</label><input class="form-input" id="gatewayKey" maxlength="16"></div><div class="toggle-group"><div class="toggle-info"><span class="toggle-title">Discovery Every Packet</span><span class="toggle-desc">When enabled, discovery is announced on every received packet.</span></div><div class="toggle-btn" id="discoveryEveryPacket" data-toggle="false" onclick="toggleSettingButton('discoveryEveryPacket')"></div></div><div class="toggle-group"><div class="toggle-info"><span class="toggle-title">ROW Debug</span><span class="toggle-desc">Enable extra serial debugging output for troubleshooting.</span></div><div class="toggle-btn" id="rowDebug" data-toggle="false" onclick="toggleSettingButton('rowDebug')"></div></div></div>
            <div class="card"><div class="card-header"><h3 class="card-title"><svg viewBox="0 0 24 24"><rect width="18" height="11" x="3" y="11" rx="2"/><path d="M7 11V7a5 5 0 0 1 10 0v4"/></svg>Encryption</h3></div><div class="toggle-group"><div class="toggle-info"><span class="toggle-title">LoRa Encryption</span><span class="toggle-desc">Enable encrypted LoRa payload handling.</span></div><div class="toggle-btn" id="loraEncryption" data-toggle="false" onclick="toggleSettingButton('loraEncryption')"></div></div><div class="toggle-group"><div class="toggle-info"><span class="toggle-title">ESP-NOW Encryption</span><span class="toggle-desc">Enable encrypted ESP-NOW payload handling.</span></div><div class="toggle-btn" id="espnowEncryption" data-toggle="false" onclick="toggleSettingButton('espnowEncryption')"></div></div><div class="form-group"><label class="form-label" for="encryptionKey">Encryption key bytes</label><textarea class="form-textarea" id="encryptionKey" placeholder='["0x4B","0xA3","0x3F","0x9C"]'></textarea><div class="form-hint">Use a JSON array with 2 to 16 values.</div></div></div>
          </div>
          <div class="grid-2"><div class="card"><div class="card-header"><h3 class="card-title"><svg viewBox="0 0 24 24"><path d="M4.9 19.1C1 15.2 1 8.8 4.9 4.9m2.9 11.3c-2.3-2.3-2.3-6.1 0-8.5"/><circle cx="12" cy="12" r="2"/><path d="M16.2 7.8c2.3 2.3 2.3 6.1 0 8.5m2.9-11.4C23 8.8 23 15.1 19.1 19"/></svg>LoRa</h3></div><div class="form-hint warning">Must match your sensors and region.</div><div class="grid-2"><div class="form-group"><label class="form-label">Module</label><input class="form-input" id="loraModuleDisplay" type="text" readonly></div><div class="form-group"><label class="form-label" for="band">Band</label><select class="form-select" id="band"><option value="433">433.0 MHz</option><option value="868">868.0 MHz</option><option value="915">915.0 MHz</option></select></div><div class="form-group"><label class="form-label" for="loraTxPower">TX Power</label><input class="form-input" id="loraTxPower" type="number"></div><div class="form-group"><label class="form-label" for="loraSignalBandwidth">Bandwidth</label><select class="form-select" id="loraSignalBandwidth"><option value="125">125.0 kHz</option><option value="250">250.0 kHz</option><option value="500">500.0 kHz</option></select></div><div class="form-group"><label class="form-label" for="loraSpreadingFactor">Spreading Factor</label><select class="form-select" id="loraSpreadingFactor"><option value="5">SF5</option><option value="6">SF6</option><option value="7">SF7</option><option value="8">SF8</option><option value="9">SF9</option><option value="10">SF10</option><option value="11">SF11</option><option value="12">SF12</option></select></div><div class="form-group"><label class="form-label" for="loraCodingRate">Coding Rate</label><select class="form-select" id="loraCodingRate"><option value="5">4/5</option><option value="6">4/6</option><option value="7">4/7</option><option value="8">4/8</option></select></div><div class="form-group"><label class="form-label" for="loraSyncWord">Sync Word</label><input class="form-input" id="loraSyncWord" type="text" placeholder="0x12"></div><div class="form-group"><label class="form-label" for="loraPreambleLength">Preamble Length</label><input class="form-input" id="loraPreambleLength" type="number"></div></div></div><div class="card"><div class="card-header"><h3 class="card-title"><svg viewBox="0 0 24 24"><path d="M4 6h16M4 12h16M4 18h8"/><circle cx="18" cy="18" r="2"/></svg>UI Config</h3><span class="badge warning">Browser only</span></div><div class="ui-config-grid"><div class="form-group"><label class="form-label" for="uiTextScale">Text Size</label><select class="form-select" id="uiTextScale"><option value="0.95">Compact</option><option value="1">Default</option><option value="1.08" selected>Large</option><option value="1.16">XL</option></select></div><div class="form-group"><label class="form-label" for="uiAccentColor">Accent Color</label><input class="form-input color-input" id="uiAccentColor" type="color" value="#f0883e"></div></div><div class="btn-group"><button class="btn btn-secondary" type="button" id="resetUiConfig">Reset UI Style</button></div><div class="form-hint ui-config-note">Applies only in this browser. Gateway settings stay unchanged.</div></div></div>
          <div class="btn-group"><button class="btn btn-primary" type="submit">Save & Reboot</button><button class="btn btn-secondary" type="button" id="reloadSettings">Reload</button></div><div class="message" id="settingsMessage"></div>
        </form>
      </section>

      <!-- Help page: project links plus markdown content loaded from GitHub. -->
      <section class="page" id="page-info">
        <div class="page-header"><h1 class="page-title">Help</h1><p class="page-desc">Project links, updates, and new features.</p></div>
        <div class="grid-2"><div class="card"><div class="card-header"><h3 class="card-title"><svg viewBox="0 0 24 24"><circle cx="12" cy="12" r="10"/><path d="M12 16v-4M12 8h.01"/></svg>Links</h3></div><div class="link-list"><div class="link-row"><span>GitHub</span><strong class="status-value"><a href="https://github.com/PricelessToolkit/CapiBridge" target="_blank" rel="noopener noreferrer">https://github.com/PricelessToolkit/CapiBridge</a></strong></div><div class="link-row"><span>YouTube</span><strong class="status-value"><a href="https://www.youtube.com/@PricelessToolkit" target="_blank" rel="noopener noreferrer">https://www.youtube.com/@PricelessToolkit</a></strong></div><div class="link-row"><span>Shop</span><strong class="status-value"><a href="https://www.pricelesstoolkit.com" target="_blank" rel="noopener noreferrer">https://www.pricelesstoolkit.com</a></strong></div></div></div><div class="card"><div class="card-header"><h3 class="card-title"><svg viewBox="0 0 24 24"><path d="M4 6h16M4 12h16M4 18h10"/></svg>Version changelog</h3><span class="badge warning">changelog.md</span></div><div class="info-feed-state" id="infoFeedState">Open this page to load updates.</div><div class="info-feed" id="infoFeedBody"></div></div></div><div class="card"><div class="card-header"><h3 class="card-title"><svg viewBox="0 0 24 24"><circle cx="12" cy="12" r="9"/><path d="M12 8h.01M11 12h1v4h1"/></svg>Info</h3><span class="badge warning">info.md</span></div><div class="info-feed-state" id="infoDocState">Open this page to load info.</div><div class="info-feed" id="infoDocBody"></div></div>
      </section>

      
    </main>
  </div>

  <script>
    // Frontend helpers: page navigation, live polling, markdown rendering, and settings form handling.
    const tabs=document.querySelectorAll('.nav-item');
    const pages=document.querySelectorAll('.page');
    const sidebar=document.getElementById('sidebar');
    const overlay=document.querySelector('.sidebar-overlay');
    const dashboardGrid=document.getElementById('dashboardGrid');
    const radioGrid=document.getElementById('radioGrid');
    const runtimeGrid=document.getElementById('runtimeGrid');
    const workingNodesGrid=document.getElementById('workingNodesGrid');
    const settingsForm=document.getElementById('settingsForm');
    const settingsMessage=document.getElementById('settingsMessage');
    const actionsMessage=document.getElementById('actionsMessage');
    const connectionBadge=document.getElementById('connectionBadge');
    const sidebarStatus=document.getElementById('sidebarStatus');
    const sidebarLed=document.getElementById('sidebarLed');
    const trafficList=document.getElementById('trafficList');
    const trafficTerminal=document.getElementById('trafficTerminal');
    const trafficSourceFilter=document.getElementById('trafficSourceFilter');
    const trafficStatusFilter=document.getElementById('trafficStatusFilter');
    const infoFeedState=document.getElementById('infoFeedState');
    const infoFeedBody=document.getElementById('infoFeedBody');
    const infoDocState=document.getElementById('infoDocState');
    const infoDocBody=document.getElementById('infoDocBody');
    const uiTextScale=document.getElementById('uiTextScale');
    const uiAccentColor=document.getElementById('uiAccentColor');
    const resetUiConfig=document.getElementById('resetUiConfig');
    const defaultAccentColor='#49A6FD';
    let settingsCache=null;
    let trafficCache=[];
    let serialCache=[];

    function showPage(page){pages.forEach(p=>p.classList.toggle('active',p.id===`page-${page}`));tabs.forEach(t=>t.classList.toggle('active',t.dataset.page===page));sidebar.classList.remove('open');overlay.classList.remove('active');if(page==='info'){loadInfoFeed();loadInfoDoc();}}
    function navigateTo(page){history.pushState(null,'',location.pathname+'#/'+page);showPage(page)}
    function loadPage(){const hash=location.hash.replace('#/','');showPage(hash||'status')}
    window.addEventListener('popstate',loadPage)
    function toggleSidebar(){sidebar.classList.toggle('open');overlay.classList.toggle('active')}
    function clampColor(value){return Math.max(0,Math.min(255,Math.round(value)))}
    function hexToRgb(hex){const clean=String(hex||'').replace('#','').trim();if(!/^[0-9a-fA-F]{6}$/.test(clean))return null;return {r:parseInt(clean.slice(0,2),16),g:parseInt(clean.slice(2,4),16),b:parseInt(clean.slice(4,6),16)}}
    function rgbToHex(r,g,b){return `#${[r,g,b].map(v=>clampColor(v).toString(16).padStart(2,'0')).join('')}`}
    function shiftColor(hex,delta){const rgb=hexToRgb(hex);if(!rgb)return defaultAccentColor;return rgbToHex(rgb.r+delta,rgb.g+delta,rgb.b+delta)}
    function applyUiScale(scale){const safe=Number(scale)||1;document.documentElement.style.setProperty('--ui-font-scale',String(safe));if(uiTextScale)uiTextScale.value=String(safe)}
    function applyAccentColor(color){const rgb=hexToRgb(color);const theme=document.documentElement.getAttribute('data-theme')==='dark'?'dark':'light';const base=rgb?rgbToHex(rgb.r,rgb.g,rgb.b):defaultAccentColor;const parsed=hexToRgb(base);document.documentElement.style.setProperty('--accent-primary',base);document.documentElement.style.setProperty('--accent-secondary',shiftColor(base,-28));document.documentElement.style.setProperty('--accent-glow',`rgba(${parsed.r},${parsed.g},${parsed.b},${theme==='dark'?0.16:0.14})`);if(uiAccentColor)uiAccentColor.value=base}
    function applyUiPreferences(){const savedScale=localStorage.getItem('capibridge-ui-font-scale')||'1.08';const savedAccent=localStorage.getItem('capibridge-ui-accent')||defaultAccentColor;applyUiScale(savedScale);applyAccentColor(savedAccent)}
    function toggleTheme(){const current=document.documentElement.getAttribute('data-theme')==='dark'?'light':'dark';document.documentElement.setAttribute('data-theme',current);localStorage.setItem('capibridge-theme',current);applyUiPreferences()}
    const savedTheme=localStorage.getItem('capibridge-theme');if(savedTheme)document.documentElement.setAttribute('data-theme',savedTheme)
    applyUiPreferences()

    function showMessage(target,text,isError){target.textContent=text;target.className='message '+(isError?'error':'success');target.style.display='block'}
    function clearMessage(target){target.style.display='none';target.textContent='';target.className='message'}
    function escapeHtml(value){return String(value||'').replaceAll('&','&amp;').replaceAll('<','&lt;').replaceAll('>','&gt;')}
    function normalizeStatusLabel(status){return String(status||'').replaceAll('_',' ')}
    function statusBadge(ok,on='Connected',off='Offline'){return `<span class="badge ${ok?'success':'danger'}">${ok?on:off}</span>`}
    function formatAge(seconds){const age=Math.max(0,Number(seconds)||0);if(age<5)return 'just now';if(age<60)return `${age}s ago`;if(age<3600)return `${Math.floor(age/60)}m ago`;if(age<86400)return `${Math.floor(age/3600)}h ago`;return `${Math.floor(age/86400)}d ago`}

    const infoFeedUrl='https://raw.githubusercontent.com/PricelessToolkit/CapiBridge/main/changelog.md';
    const infoDocUrl='https://raw.githubusercontent.com/PricelessToolkit/CapiBridge/main/info.md';
    let infoFeedLoaded=false;
    let infoDocLoaded=false;
    function renderInlineMarkdown(text){let html=escapeHtml(text);html=html.replace(/\[([^\]]+)\]\((https?:[^\s)]+)\)/g,'<a href="$2" target="_blank" rel="noopener noreferrer">$1</a>');html=html.replace(/`([^`]+)`/g,'<code>$1</code>');html=html.replace(/\*\*([^*]+)\*\*/g,'<strong>$1</strong>');return html}
    function renderInfoMarkdown(md){const lines=String(md||'').replace(/\r/g,'').split('\n');let html='';let inList=false;let inCode=false;for(const rawLine of lines){const line=rawLine.trimEnd();const trimmed=line.trim();if(trimmed.startsWith('```')){if(inList){html+='</ul>';inList=false}html+=inCode?'</code></pre>':'<pre><code>';inCode=!inCode;continue}if(inCode){html+=escapeHtml(line)+'\n';continue}if(!trimmed){if(inList){html+='</ul>';inList=false}continue}if(trimmed.startsWith('# ')){if(inList){html+='</ul>';inList=false}html+=`<h1>${renderInlineMarkdown(trimmed.slice(2))}</h1>`;continue}if(trimmed.startsWith('## ')){if(inList){html+='</ul>';inList=false}html+=`<h2>${renderInlineMarkdown(trimmed.slice(3))}</h2>`;continue}if(trimmed.startsWith('### ')){if(inList){html+='</ul>';inList=false}html+=`<h3>${renderInlineMarkdown(trimmed.slice(4))}</h3>`;continue}if(trimmed.startsWith('- ')||trimmed.startsWith('* ')){if(!inList){html+='<ul>';inList=true}html+=`<li>${renderInlineMarkdown(trimmed.slice(2))}</li>`;continue}if(inList){html+='</ul>';inList=false}html+=`<p>${renderInlineMarkdown(trimmed)}</p>`}if(inList)html+='</ul>';if(inCode)html+='</code></pre>';return html||'<p>No content found in changelog.md.</p>'}
    async function loadInfoFeed(){if(infoFeedLoaded)return;infoFeedState.textContent='Loading changelog.md...';infoFeedBody.innerHTML='';try{const response=await fetch(infoFeedUrl,{cache:'no-store'});if(!response.ok)throw new Error('fetch failed');const markdown=await response.text();infoFeedBody.innerHTML=renderInfoMarkdown(markdown);infoFeedState.textContent='Loaded from GitHub changelog.md';infoFeedLoaded=true;}catch(error){infoFeedState.textContent="Can't parse file. No internet or changelog.md unavailable.";infoFeedBody.innerHTML='';}}
    async function loadInfoDoc(){if(infoDocLoaded)return;infoDocState.textContent='Loading info.md...';infoDocBody.innerHTML='';try{const response=await fetch(infoDocUrl,{cache:'no-store'});if(!response.ok)throw new Error('fetch failed');const markdown=await response.text();infoDocBody.innerHTML=renderInfoMarkdown(markdown);infoDocState.textContent='Loaded from GitHub info.md';infoDocLoaded=true;}catch(error){infoDocState.textContent="Can't parse file. No internet or info.md unavailable.";infoDocBody.innerHTML='';}}

    function formatPrettyPacketJson(raw){try{return JSON.stringify(JSON.parse(raw),null,2)}catch(error){return raw||''}}

    function setToggleButton(id,value){const el=document.getElementById(id);if(!el)return;const active=!!value;el.dataset.toggle=String(active);el.classList.toggle('active',active)}
    function getToggleButtonValue(id){const el=document.getElementById(id);return !!(el&&el.dataset.toggle==='true')}
    function toggleSettingButton(id){setToggleButton(id,!getToggleButtonValue(id))}

    function renderStatus(status,settings){
      connectionBadge.className='badge '+(status.wifiConnected?'success':'warning');
      connectionBadge.textContent=status.wifiConnected?'Online':'Setup / Offline';
      sidebarStatus.textContent=status.apEnabled?`Setup AP: ${status.apSsid}`:(status.wifiConnected?'Connected':'Offline');
      sidebarLed.className='status-led '+(status.wifiConnected?'':(status.apEnabled?'warn':'danger'));
      dashboardGrid.innerHTML=[['WiFi',statusBadge(status.wifiConnected)],['MQTT',statusBadge(status.mqttConnected)],['IP Address',status.ipAddress||'Unavailable'],['Setup AP',status.apEnabled?status.apSsid:'Off'],['Firmware',status.firmwareVersion],['Config Source',status.settingsSource],['Gateway Key',settings.gatewayKey],['MQTT Broker',`${settings.mqttServer||'Not set'}:${settings.mqttPort}`]].map(([l,v])=>`<div class="status-item"><span class="status-label">${l}</span><span class="status-value ${l==='Gateway Key'?'hl':''}">${v}</span></div>`).join('');
      radioGrid.innerHTML=[['Module',settings.loraModuleLabel],['Band',`${settings.band} MHz`],['TX Power',`${settings.loraTxPower} dBm`],['Bandwidth',`${settings.loraSignalBandwidth} kHz`],['Spreading Factor',`SF${settings.loraSpreadingFactor}`],['Coding Rate',`4/${settings.loraCodingRate}`],['Sync Word',`0x${Number(settings.loraSyncWord).toString(16).toUpperCase().padStart(2,'0')}`],['Preamble',settings.loraPreambleLength]].map(([l,v])=>`<div class="status-item"><span class="status-label">${l}</span><span class="status-value">${v}</span></div>`).join('');
      runtimeGrid.innerHTML=[['WiFi RSSI',status.wifiRssi],['Free Heap',`${status.freeHeap} bytes`],['Uptime',status.uptime],['LoRa Enc',settings.loraEncryption?'Enabled':'Disabled'],['ESP-NOW Enc',settings.espnowEncryption?'Enabled':'Disabled'],['Discovery',settings.discoveryEveryPacket?'Every packet':'Boot only'],['ROW Debug',settings.rowDebug?'Enabled':'Disabled'],['AP IP',status.apEnabled?(status.apIp||'Active'):'Inactive']].map(([l,v])=>`<div class="status-item"><span class="status-label">${l}</span><span class="status-value">${v}</span></div>`).join('');
    }

    function renderWorkingNodes(items){if(!workingNodesGrid)return;workingNodesGrid.innerHTML=items.length?items.map(item=>{const sourceClass=item.source==='LoRa'?'source-lora':'source-espnow';const battery=item.battery!==undefined&&item.battery!==null&&item.battery!==''?`<span class="badge">${escapeHtml(item.battery)}%</span>`:'';const rssi=item.rssi!==undefined?`<span class="node-meta">RSSI ${item.rssi} dBm</span>`:'';return `<div class="node-card status-${item.status}"><div class="node-card-header"><div><div class="node-name">${escapeHtml(item.nodeId)}</div><div class="node-meta">Last seen ${formatAge(item.ageSeconds)}</div></div>${rssi}</div><div class="node-badges"><span class="badge ${sourceClass}">${item.source}</span><span class="badge status-${item.status}">${normalizeStatusLabel(item.status)}</span>${battery}</div></div>`}).join(''):'<div class="empty-state">No active nodes discovered yet.</div>'}

    function renderTraffic(items){
      const sourceFilter=trafficSourceFilter.value;const statusFilter=trafficStatusFilter.value;
      const filtered=items.filter(item=>(sourceFilter==='all'||item.source===sourceFilter)&&(statusFilter==='all'||item.status===statusFilter));
      if(!filtered.length){trafficList.innerHTML='<div class="empty-state">No packets match the current filters yet.</div>';}
      trafficList.innerHTML=filtered.length?filtered.slice(-3).map(item=>{const sourceClass=item.source==='LoRa'?'source-lora':'source-espnow';const rssi=item.rssi!==undefined?`<span class="traffic-meta">RSSI ${item.rssi} dBm</span>`:'';const node=item.nodeId?`<span class="badge node-name">${escapeHtml(item.nodeId)}</span>`:'';const prettyJson=formatPrettyPacketJson(item.raw||item.pretty||'');return `<div class="traffic-card ${sourceClass} status-${item.status}"><div class="traffic-head"><div class="traffic-badges"><span class="badge ${sourceClass}">${item.source}</span><span class="badge status-${item.status}">${normalizeStatusLabel(item.status)}</span>${node}</div><span class="traffic-meta">${item.time}</span></div><div class="traffic-badges">${rssi}</div><pre class="traffic-pre">${escapeHtml(prettyJson)}</pre></div>`}).join(''):'<div class="empty-state">No packets match the current filters yet.</div>';
      trafficTerminal.innerHTML=serialCache.length?serialCache.map(line=>`<div class="traffic-terminal-line">${escapeHtml(line)}</div>`).join(''):'Waiting for serial output...';
      trafficTerminal.scrollTop=trafficTerminal.scrollHeight;
    }

    function fillForm(settings){
      document.getElementById('wifiSsid').value=settings.wifiSsid||'';
      document.getElementById('wifiPassword').value=settings.wifiPassword||'';
      document.getElementById('mqttServer').value=settings.mqttServer||'';
      document.getElementById('mqttPort').value=settings.mqttPort||1883;
      document.getElementById('mqttUsername').value=settings.mqttUsername||'';
      document.getElementById('mqttPassword').value=settings.mqttPassword||'';
      document.getElementById('gatewayKey').value=settings.gatewayKey||'';
      setToggleButton('discoveryEveryPacket',!!settings.discoveryEveryPacket);
      setToggleButton('rowDebug',!!settings.rowDebug);
      setToggleButton('loraEncryption',!!settings.loraEncryption);
      setToggleButton('espnowEncryption',!!settings.espnowEncryption);
      document.getElementById('encryptionKey').value=JSON.stringify(settings.encryptionKey||[]);
      document.getElementById('loraModuleDisplay').value=settings.loraModuleLabel||'Default firmware module';
      document.getElementById('band').value=String(Number(settings.band));
      document.getElementById('loraTxPower').value=settings.loraTxPower ?? '';
      document.getElementById('loraSignalBandwidth').value=String(Number(settings.loraSignalBandwidth));
      document.getElementById('loraSpreadingFactor').value=String(settings.loraSpreadingFactor);
      document.getElementById('loraCodingRate').value=String(settings.loraCodingRate);
      document.getElementById('loraSyncWord').value=`0x${Number(settings.loraSyncWord).toString(16).toUpperCase().padStart(2,'0')}`;
      document.getElementById('loraPreambleLength').value=settings.loraPreambleLength ?? '';
    }

    async function fetchJson(path){const response=await fetch(path,{cache:'no-store'});if(!response.ok)throw new Error(`Request failed: ${response.status}`);return response.json()}
    async function loadTraffic(){const traffic=await fetchJson('/api/traffic');trafficCache=traffic.items||[];serialCache=traffic.serialLines||[];renderWorkingNodes(traffic.nodes||[]);renderTraffic(trafficCache)}
    async function loadAll(){clearMessage(settingsMessage);clearMessage(actionsMessage);const [settings,status]=await Promise.all([fetchJson('/api/settings'),fetchJson('/api/status')]);settingsCache=settings;renderStatus(status,settings);fillForm(settings);await loadTraffic()}

    trafficSourceFilter.addEventListener('change',()=>renderTraffic(trafficCache));
    trafficStatusFilter.addEventListener('change',()=>renderTraffic(trafficCache));
    if(uiTextScale)uiTextScale.addEventListener('change',event=>{localStorage.setItem('capibridge-ui-font-scale',event.target.value);applyUiScale(event.target.value)});
    if(uiAccentColor)uiAccentColor.addEventListener('input',event=>{localStorage.setItem('capibridge-ui-accent',event.target.value);applyAccentColor(event.target.value)});
    if(resetUiConfig)resetUiConfig.addEventListener('click',()=>{localStorage.removeItem('capibridge-ui-font-scale');localStorage.removeItem('capibridge-ui-accent');applyUiPreferences()});

    settingsForm.addEventListener('submit',async event=>{event.preventDefault();clearMessage(settingsMessage);let keyBytes;try{keyBytes=JSON.parse(document.getElementById('encryptionKey').value||'[]')}catch(error){showMessage(settingsMessage,'Encryption key must be valid JSON, for example ["0x4B","0xA3"].',true);return}
      const syncWordText=document.getElementById('loraSyncWord').value.trim();
      const syncWordValue=syncWordText.toLowerCase().startsWith('0x')?parseInt(syncWordText,16):parseInt(syncWordText,16);
      if(Number.isNaN(syncWordValue)){showMessage(settingsMessage,'Sync Word must be a valid hex value like 0x12.',true);return}
      const payload={wifiSsid:document.getElementById('wifiSsid').value.trim(),wifiPassword:document.getElementById('wifiPassword').value,mqttServer:document.getElementById('mqttServer').value.trim(),mqttPort:Number(document.getElementById('mqttPort').value),mqttUsername:document.getElementById('mqttUsername').value.trim(),mqttPassword:document.getElementById('mqttPassword').value,gatewayKey:document.getElementById('gatewayKey').value.trim(),discoveryEveryPacket:getToggleButtonValue('discoveryEveryPacket'),rowDebug:getToggleButtonValue('rowDebug'),loraEncryption:getToggleButtonValue('loraEncryption'),espnowEncryption:getToggleButtonValue('espnowEncryption'),encryptionKey:keyBytes,band:Number(document.getElementById('band').value),loraTxPower:Number(document.getElementById('loraTxPower').value),loraSignalBandwidth:Number(document.getElementById('loraSignalBandwidth').value),loraSpreadingFactor:Number(document.getElementById('loraSpreadingFactor').value),loraCodingRate:Number(document.getElementById('loraCodingRate').value),loraSyncWord:syncWordValue,loraPreambleLength:Number(document.getElementById('loraPreambleLength').value)};
      const response=await fetch('/api/settings',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(payload)});const result=await response.json();if(!response.ok||!result.ok){showMessage(settingsMessage,result.error||'Failed to save settings.',true);return}showMessage(settingsMessage,'Settings saved. The gateway will reboot to apply them.',false)});

    document.getElementById('reloadSettings').addEventListener('click',()=>loadAll().catch(error=>showMessage(settingsMessage,error.message,true)));
    document.getElementById('refreshAll').addEventListener('click',()=>loadAll().catch(error=>showMessage(actionsMessage,error.message,true)));
    document.getElementById('rebootButton').addEventListener('click',async()=>{clearMessage(actionsMessage);const response=await fetch('/api/reboot',{method:'POST'});const result=await response.json();if(!response.ok||!result.ok){showMessage(actionsMessage,result.error||'Failed to schedule reboot.',true);return}showMessage(actionsMessage,'Gateway reboot scheduled.',false)});
    document.getElementById('defaultsButton').addEventListener('click',async()=>{clearMessage(actionsMessage);const response=await fetch('/api/reset',{method:'POST'});const result=await response.json();if(!response.ok||!result.ok){showMessage(actionsMessage,result.error||'Failed to restore defaults.',true);return}showMessage(actionsMessage,'Defaults restored. The gateway will reboot now.',false)});

    loadPage();
    loadAll().catch(error=>showMessage(actionsMessage,error.message,true));
    setInterval(()=>{fetchJson('/api/status').then(status=>{if(settingsCache){renderStatus(status,settingsCache)}}).catch(()=>{});loadTraffic().catch(()=>{})},2000);
  </script>
</body>
</html>
)rawliteral";
