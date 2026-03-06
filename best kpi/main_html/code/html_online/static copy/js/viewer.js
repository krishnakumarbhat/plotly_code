// Log Viewer - Online Mode JavaScript

// Global state
let mappings = {};
let currentKey = null;
let currentImages = [];
let currentImageIndex = 0;
let remoteLogs = {};
const serveMode = true;

// Initialize on DOM load
document.addEventListener('DOMContentLoaded', async function() {
  await loadMappings();
  initializeUI();
  setupEventListeners();
});

// Load mappings from server
async function loadMappings() {
  try {
    const resp = await fetch('/api/mappings');
    const data = await resp.json();
    if (data.success) {
      mappings = data.mappings || {};
      populateLogDropdown();
    }
  } catch (e) {
    console.error('Failed to load mappings:', e);
  }
}

// Populate log dropdown
function populateLogDropdown() {
  const select = document.getElementById('logSelect');
  select.innerHTML = '';
  
  const keys = Object.keys(mappings).sort();
  if (keys.length === 0) {
    const opt = document.createElement('option');
    opt.value = '';
    opt.textContent = 'No data available';
    select.appendChild(opt);
    return;
  }
  
  keys.forEach(key => {
    const opt = document.createElement('option');
    opt.value = key;
    opt.textContent = mappings[key].html_folder || key;
    select.appendChild(opt);
  });
  
  // Select first and update
  if (keys.length > 0) {
    updateScenario(keys[0]);
  }
}

// Initialize UI elements
function initializeUI() {
  setupExpandButtons();
  setupImageModal();
}

// Setup event listeners
function setupEventListeners() {
  // Log selection
  document.getElementById('logSelect').addEventListener('change', function() {
    updateScenario(this.value);
  });
  
  // Sensor filter
  document.getElementById('sensorFilter').addEventListener('change', function() {
    const entry = mappings[currentKey];
    if (entry) {
      buildSensorGrid(entry, this.value);
    }
  });
  
  // Edit comment button
  document.getElementById('editCommentBtn').addEventListener('click', function() {
    const box = document.getElementById('commentBox');
    box.readOnly = false;
    box.focus();
  });
  
  // Save comment button
  document.getElementById('saveCommentBtn').addEventListener('click', saveComment);
  
  // Open HTML button
  document.getElementById('openHtmlBtn').addEventListener('click', openHtmlPreview);
  
  // Go Online button
  document.getElementById('goOnlineBtn').addEventListener('click', openClusterModal);
}

// Update scenario when log selected
async function updateScenario(key) {
  currentKey = key;
  const entry = mappings[key];
  
  if (!entry) {
    document.getElementById('logNameDisplay').textContent = '';
    document.getElementById('videoPlayer').src = '';
    document.getElementById('videoInfo').textContent = 'No video';
    document.getElementById('commentBox').value = '';
    document.getElementById('sensorGrid').innerHTML = '<div class="no-data">No data</div>';
    return;
  }
  
  // Update log name display
  document.getElementById('logNameDisplay').textContent = entry.html_folder || key;
  
  // Update video
  const video = document.getElementById('videoPlayer');
  video.src = entry.video;
  document.getElementById('videoInfo').textContent = entry.video_name;
  
  // Update sensor filter
  updateSensorFilter(entry);
  
  // Build sensor grid
  buildSensorGrid(entry, 'all');
  
  // Load existing comment
  await loadExistingComment(entry);
}

// Update sensor filter dropdown
function updateSensorFilter(entry) {
  const select = document.getElementById('sensorFilter');
  select.innerHTML = '<option value="all">All</option>';
  
  if (entry && entry.images) {
    Object.keys(entry.images).sort().forEach(sensor => {
      const opt = document.createElement('option');
      opt.value = sensor;
      opt.textContent = sensor.charAt(0).toUpperCase() + sensor.slice(1);
      select.appendChild(opt);
    });
  }
}

// Build sensor grid
function buildSensorGrid(entry, filter) {
  const grid = document.getElementById('sensorGrid');
  grid.innerHTML = '';
  currentImages = [];
  
  if (!entry || !entry.images) {
    grid.innerHTML = '<div class="no-data">No images available</div>';
    return;
  }
  
  const images = entry.images;
  const sensors = filter === 'all' ? Object.keys(images) : [filter];
  
  sensors.forEach(sensor => {
    if (!images[sensor]) return;
    
    Object.entries(images[sensor]).forEach(([position, imgPath]) => {
      currentImages.push({
        path: imgPath,
        sensor: sensor,
        position: position
      });
      
      const card = document.createElement('div');
      card.className = 'sensor-card';
      card.onclick = () => openImageModal(currentImages.length - 1);
      
      const img = document.createElement('img');
      img.src = imgPath;
      img.alt = `${sensor} - ${position}`;
      img.loading = 'lazy';
      
      const label = document.createElement('div');
      label.className = 'sensor-card-label';
      label.textContent = `${sensor.toUpperCase()} - ${position}`;
      
      card.appendChild(img);
      card.appendChild(label);
      grid.appendChild(card);
    });
  });
  
  if (currentImages.length === 0) {
    grid.innerHTML = '<div class="no-data">No images for this filter</div>';
  } else {
    // Set data-count attribute for adaptive grid layout
    grid.setAttribute('data-count', currentImages.length.toString());
  }
}

// Load existing comment
async function loadExistingComment(entry) {
  const commentBox = document.getElementById('commentBox');
  const commentPath = document.getElementById('commentPath');
  
  if (!entry) {
    commentBox.value = '';
    commentPath.textContent = '';
    return;
  }
  
  commentPath.textContent = entry.comment_path || '';
  
  try {
    const resp = await fetch('/api/get-comment', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ path: entry.comment_path })
    });
    const data = await resp.json();
    commentBox.value = data.success ? (data.content || '') : (entry.comment_content || '');
  } catch (e) {
    commentBox.value = entry.comment_content || '';
  }
  
  commentBox.readOnly = true;
}

// Save comment
async function saveComment() {
  const entry = mappings[currentKey];
  if (!entry) return;
  
  const commentBox = document.getElementById('commentBox');
  const content = commentBox.value;
  
  try {
    const resp = await fetch('/api/save-comment', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({
        path: entry.comment_path,
        content: content
      })
    });
    const data = await resp.json();
    
    if (data.success) {
      commentBox.readOnly = true;
      showNotification('Comment saved!', 'success');
    } else {
      showNotification('Failed to save: ' + data.error, 'error');
    }
  } catch (e) {
    showNotification('Error saving comment', 'error');
  }
}

// Open HTML preview
function openHtmlPreview() {
  const entry = mappings[currentKey];
  if (!entry || !entry.html_files || entry.html_files.length === 0) {
    showNotification('No HTML files available', 'error');
    return;
  }
  
  // Open first HTML file in new tab
  const htmlFile = entry.html_files[0];
  window.open(htmlFile, '_blank');
}

// Setup expand buttons
function setupExpandButtons() {
  document.querySelectorAll('.expand-btn').forEach(btn => {
    btn.addEventListener('click', function(e) {
      e.stopPropagation();
      const box = this.closest('.box');
      box.classList.toggle('expanded');
      this.textContent = box.classList.contains('expanded') ? '✕' : '⛶';
    });
  });
}

// Image Modal functions
function setupImageModal() {
  const modal = document.getElementById('imageModal');
  
  modal.querySelector('.image-modal-close').addEventListener('click', closeImageModal);
  modal.querySelector('.image-modal-nav.prev').addEventListener('click', () => navigateModal(-1));
  modal.querySelector('.image-modal-nav.next').addEventListener('click', () => navigateModal(1));
  
  modal.addEventListener('click', function(e) {
    if (e.target === this) closeImageModal();
  });
  
  document.addEventListener('keydown', function(e) {
    if (!modal.classList.contains('active')) return;
    
    if (e.key === 'Escape') closeImageModal();
    else if (e.key === 'ArrowLeft') navigateModal(-1);
    else if (e.key === 'ArrowRight') navigateModal(1);
  });
}

function openImageModal(index) {
  currentImageIndex = index;
  const modal = document.getElementById('imageModal');
  const img = modal.querySelector('img');
  const info = modal.querySelector('.image-modal-info');
  
  const imageData = currentImages[index];
  img.src = imageData.path;
  info.textContent = `${imageData.sensor.toUpperCase()} - ${imageData.position} (${index + 1}/${currentImages.length})`;
  
  modal.classList.add('active');
}

function closeImageModal() {
  document.getElementById('imageModal').classList.remove('active');
}

function navigateModal(direction) {
  currentImageIndex = (currentImageIndex + direction + currentImages.length) % currentImages.length;
  const modal = document.getElementById('imageModal');
  const img = modal.querySelector('img');
  const info = modal.querySelector('.image-modal-info');
  
  const imageData = currentImages[currentImageIndex];
  img.src = imageData.path;
  info.textContent = `${imageData.sensor.toUpperCase()} - ${imageData.position} (${currentImageIndex + 1}/${currentImages.length})`;
}

// ============================================
// Cluster Connection Functions
// ============================================

function openClusterModal() {
  document.getElementById('clusterModal').classList.add('active');
  loadServers();
  checkClusterStatus();
}

function closeClusterModal() {
  document.getElementById('clusterModal').classList.remove('active');
}

async function loadServers() {
  try {
    const resp = await fetch('/api/cluster/servers');
    const data = await resp.json();
    const select = document.getElementById('serverSelect');
    select.innerHTML = '';
    
    (data.servers || []).forEach(server => {
      const opt = document.createElement('option');
      opt.value = server;
      opt.textContent = server;
      select.appendChild(opt);
    });
  } catch (e) {
    console.error('Failed to load servers:', e);
  }
}

async function checkClusterStatus() {
  try {
    const resp = await fetch('/api/cluster/status');
    const data = await resp.json();
    updateClusterUI(data.connected, data.server);
  } catch (e) {
    console.error('Failed to check cluster status:', e);
  }
}

function updateClusterUI(connected, serverName) {
  const statusDot = document.getElementById('statusDot');
  const statusText = document.getElementById('statusText');
  const loginPanel = document.getElementById('loginPanel');
  const pathPanel = document.getElementById('pathPanel');
  
  statusDot.className = 'status-dot ' + (connected ? 'connected' : 'disconnected');
  statusText.textContent = connected ? `Connected to ${serverName}` : 'Disconnected';
  
  // Show appropriate panel
  if (connected) {
    loginPanel.classList.remove('active');
    pathPanel.classList.add('active');
  } else {
    loginPanel.classList.add('active');
    pathPanel.classList.remove('active');
  }
}

// Toggle password visibility
function togglePassword() {
  const input = document.getElementById('passwordInput');
  const btn = document.querySelector('.toggle-password');
  
  if (input.type === 'password') {
    input.type = 'text';
    btn.textContent = '🙈';
  } else {
    input.type = 'password';
    btn.textContent = '👁';
  }
}

async function clusterConnect() {
  const server = document.getElementById('serverSelect').value;
  const username = document.getElementById('usernameInput').value;
  const password = document.getElementById('passwordInput').value;
  
  if (!server || !username || !password) {
    showClusterMessage('Please fill all fields', 'error');
    return;
  }
  
  showLoading('Connecting to ' + server + '...');
  
  try {
    const resp = await fetch('/api/cluster/connect', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ server, username, password })
    });
    const data = await resp.json();
    
    hideLoading();
    
    if (data.success) {
      showClusterMessage('Connected successfully!', 'success');
      updateClusterUI(true, server);
    } else {
      showClusterMessage(data.error || 'Connection failed', 'error');
    }
  } catch (e) {
    hideLoading();
    showClusterMessage('Connection error: ' + e.message, 'error');
  }
}

async function clusterDisconnect() {
  try {
    await fetch('/api/cluster/disconnect', { method: 'POST' });
    updateClusterUI(false, null);
    showClusterMessage('Disconnected', 'info');
    
    // Clear remote logs
    document.getElementById('remoteLogsSection').style.display = 'none';
    document.getElementById('remoteLogsList').innerHTML = '';
    remoteLogs = {};
  } catch (e) {
    console.error('Disconnect error:', e);
  }
}

// Scan remote logs from the given path
async function scanRemoteLogs() {
  const remotePath = document.getElementById('remotePathInput').value.trim();
  
  if (!remotePath) {
    showClusterMessage('Please enter a remote path', 'error');
    return;
  }
  
  showLoading('Scanning remote logs...');
  
  try {
    const resp = await fetch('/api/cluster/scan-logs', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ path: remotePath })
    });
    const data = await resp.json();
    
    hideLoading();
    
    if (data.success) {
      remoteLogs = data.logs || {};
      displayRemoteLogs();
      showClusterMessage(`Found ${Object.keys(remoteLogs).length} logs`, 'success');
    } else {
      showClusterMessage(data.error || 'Scan failed', 'error');
    }
  } catch (e) {
    hideLoading();
    showClusterMessage('Scan error: ' + e.message, 'error');
  }
}

function displayRemoteLogs() {
  const section = document.getElementById('remoteLogsSection');
  const list = document.getElementById('remoteLogsList');
  
  list.innerHTML = '';
  
  const logKeys = Object.keys(remoteLogs).sort();
  
  if (logKeys.length === 0) {
    section.style.display = 'none';
    return;
  }
  
  section.style.display = 'block';
  
  logKeys.forEach(key => {
    const log = remoteLogs[key];
    const isCached = log.cached === true;
    const item = document.createElement('div');
    item.className = 'remote-log-item' + (isCached ? ' cached' : '');
    item.id = 'log-' + key;
    item.onclick = () => downloadLog(key);
    
    const icon = isCached ? '✅' : '📦';
    const status = isCached ? 'Cached - Click to view' : 'Click to download';
    
    item.innerHTML = `
      <span class="remote-log-icon">${icon}</span>
      <span class="remote-log-name">${log.name || key}</span>
      <span class="remote-log-status">${status}</span>
    `;
    
    list.appendChild(item);
  });
}

async function downloadLog(key) {
  const log = remoteLogs[key];
  if (!log) return;
  
  const item = document.getElementById('log-' + key);
  
  // If already cached or downloaded, just load it
  if (item.classList.contains('cached') || item.classList.contains('downloaded')) {
    closeClusterModal();
    await loadMappings();
    updateScenario(key);
    return;
  }
  
  if (item.classList.contains('downloading')) {
    return;
  }
  
  item.classList.add('downloading');
  item.querySelector('.remote-log-status').textContent = 'Downloading...';
  
  showLoading('Downloading ' + (log.name || key) + '...');
  
  try {
    const resp = await fetch('/api/cluster/download-log', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({
        key: key,
        html_path: log.html_path,
        video_path: log.video_path
      })
    });
    const data = await resp.json();
    
    hideLoading();
    
    if (data.success) {
      item.classList.remove('downloading');
      item.classList.add('downloaded');
      item.querySelector('.remote-log-status').textContent = '✓ Downloaded';
      item.querySelector('.remote-log-icon').textContent = '✅';
      
      showClusterMessage('Downloaded successfully! Click again to view.', 'success');
      
      // Reload mappings
      await loadMappings();
    } else {
      item.classList.remove('downloading');
      item.querySelector('.remote-log-status').textContent = 'Failed';
      showClusterMessage(data.error || 'Download failed', 'error');
    }
  } catch (e) {
    hideLoading();
    item.classList.remove('downloading');
    item.querySelector('.remote-log-status').textContent = 'Error';
    showClusterMessage('Download error: ' + e.message, 'error');
  }
}

// Loading helpers
function showLoading(text) {
  const overlay = document.getElementById('loadingOverlay');
  document.getElementById('loadingText').textContent = text || 'Loading...';
  overlay.style.display = 'flex';
}

function hideLoading() {
  document.getElementById('loadingOverlay').style.display = 'none';
}

function showClusterMessage(msg, type) {
  const el = document.getElementById('clusterMessage');
  el.textContent = msg;
  el.className = 'cluster-message ' + type;
  el.style.display = 'block';
  
  setTimeout(() => {
    el.style.display = 'none';
  }, 5000);
}

// Notification helper
function showNotification(message, type) {
  if (type === 'error') {
    alert('Error: ' + message);
  } else {
    alert(message);
  }
}
