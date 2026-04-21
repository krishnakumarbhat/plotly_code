/**
 * HPC Tools Platform - Main JavaScript
 */

// Global state
const HPCTools = {
    currentPath: '/net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/2-Sim/USER_DATA',
    selectedFile: null,
    fileBrowserModal: null
};

// Initialize on DOM ready
document.addEventListener('DOMContentLoaded', function() {
    initializeTooltips();
    initializeFileBrowser();
    initializeAutoRefresh();
});

/**
 * Initialize Bootstrap tooltips
 */
function initializeTooltips() {
    const tooltipTriggerList = document.querySelectorAll('[data-bs-toggle="tooltip"]');
    tooltipTriggerList.forEach(el => new bootstrap.Tooltip(el));
}

/**
 * Initialize file browser modal
 */
function initializeFileBrowser() {
    const modalEl = document.getElementById('fileBrowserModal');
    if (modalEl) {
        HPCTools.fileBrowserModal = new bootstrap.Modal(modalEl);
        
        const selectBtn = document.getElementById('selectFileBtn');
        if (selectBtn) {
            selectBtn.addEventListener('click', function() {
                if (HPCTools.selectedFile) {
                    document.getElementById('input_path').value = HPCTools.selectedFile;
                    HPCTools.fileBrowserModal.hide();
                }
            });
        }
    }
}

/**
 * Open file browser modal
 */
function openFileBrowser() {
    HPCTools.selectedFile = null;
    const selectBtn = document.getElementById('selectFileBtn');
    if (selectBtn) selectBtn.disabled = true;
    
    loadDirectory(HPCTools.currentPath);
    if (HPCTools.fileBrowserModal) {
        HPCTools.fileBrowserModal.show();
    }
}

/**
 * Load directory contents
 */
async function loadDirectory(path) {
    const content = document.getElementById('fileBrowserContent');
    if (!content) return;
    
    content.innerHTML = `
        <div class="text-center py-4">
            <div class="spinner-border text-primary" role="status">
                <span class="visually-hidden">Loading...</span>
            </div>
        </div>
    `;
    
    try {
        const response = await fetch(`/api/browse?path=${encodeURIComponent(path)}&extensions=h5,hdf5,mf4`);
        const data = await response.json();
        
        if (data.error) {
            content.innerHTML = `<div class="alert alert-danger">${data.error}</div>`;
            return;
        }
        
        HPCTools.currentPath = data.current_path;
        renderFileList(data, content);
    } catch (error) {
        console.error('Error loading directory:', error);
        content.innerHTML = '<div class="alert alert-danger">Error loading directory</div>';
    }
}

/**
 * Render file list in browser
 */
function renderFileList(data, container) {
    let html = '<div class="list-group">';
    
    // Parent directory link
    if (data.parent) {
        html += `
            <a href="#" class="list-group-item list-group-item-action" onclick="loadDirectory('${data.parent}'); return false;">
                <i class="bi bi-arrow-up me-2"></i>..
            </a>
        `;
    }
    
    // Directories
    data.directories.forEach(dir => {
        html += `
            <a href="#" class="list-group-item list-group-item-action" onclick="loadDirectory('${dir.path}'); return false;">
                <i class="bi bi-folder-fill text-warning me-2"></i>${dir.name}
            </a>
        `;
    });
    
    // Files
    data.files.forEach(file => {
        html += `
            <a href="#" class="list-group-item list-group-item-action file-item" 
               data-path="${file.path}" onclick="selectFile(this, '${file.path}'); return false;">
                <div class="d-flex justify-content-between align-items-center">
                    <span><i class="bi bi-file-earmark text-primary me-2"></i>${file.name}</span>
                    <small class="text-muted">${formatFileSize(file.size)}</small>
                </div>
            </a>
        `;
    });
    
    if (data.directories.length === 0 && data.files.length === 0) {
        html += '<div class="list-group-item text-muted text-center">No matching files found</div>';
    }
    
    html += '</div>';
    container.innerHTML = html;
}

/**
 * Select a file in the browser
 */
function selectFile(element, path) {
    // Remove previous selection
    document.querySelectorAll('.file-item').forEach(el => {
        el.classList.remove('active');
    });
    
    // Add selection to clicked item
    element.classList.add('active');
    HPCTools.selectedFile = path;
    
    // Enable select button
    const selectBtn = document.getElementById('selectFileBtn');
    if (selectBtn) selectBtn.disabled = false;
}

/**
 * Format file size for display
 */
function formatFileSize(bytes) {
    if (bytes === 0) return '0 B';
    const k = 1024;
    const sizes = ['B', 'KB', 'MB', 'GB', 'TB'];
    const i = Math.floor(Math.log(bytes) / Math.log(k));
    return parseFloat((bytes / Math.pow(k, i)).toFixed(1)) + ' ' + sizes[i];
}

/**
 * Initialize auto-refresh for running jobs
 */
function initializeAutoRefresh() {
    // Refresh running job statuses every 30 seconds
    setInterval(() => {
        const runningBadges = document.querySelectorAll('.status-RUNNING, .status-QUEUED, .status-SUBMITTED');
        runningBadges.forEach(badge => {
            const row = badge.closest('tr');
            if (row && row.dataset.jobId) {
                refreshJobStatus(row.dataset.jobId);
            }
        });
    }, 30000);
}

/**
 * Refresh status of a specific job
 */
async function refreshJobStatus(jobId) {
    try {
        const response = await fetch(`/api/job/${jobId}/status`);
        const data = await response.json();
        
        // Update the status badge
        const row = document.querySelector(`tr[data-job-id="${jobId}"]`);
        if (row) {
            const badge = row.querySelector('.status-badge, .badge');
            if (badge) {
                badge.className = `badge status-${data.status}`;
                badge.textContent = data.status;
            }
        }
        
        return data;
    } catch (error) {
        console.error('Error refreshing job status:', error);
        return null;
    }
}

/**
 * Cancel a running job
 */
async function cancelJob(jobId) {
    if (!confirm('Are you sure you want to cancel this job?')) {
        return;
    }
    
    try {
        const response = await fetch(`/api/job/${jobId}/cancel`, { 
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            }
        });
        
        const data = await response.json();
        
        if (data.success) {
            await refreshJobStatus(jobId);
            showNotification('Job cancelled successfully', 'success');
        } else {
            showNotification('Failed to cancel job: ' + data.error, 'error');
        }
    } catch (error) {
        console.error('Error cancelling job:', error);
        showNotification('Error cancelling job', 'error');
    }
}

/**
 * Show a notification toast
 */
function showNotification(message, type = 'info') {
    // Create toast container if it doesn't exist
    let container = document.getElementById('toastContainer');
    if (!container) {
        container = document.createElement('div');
        container.id = 'toastContainer';
        container.className = 'toast-container position-fixed bottom-0 end-0 p-3';
        container.style.zIndex = '1100';
        document.body.appendChild(container);
    }
    
    const toastId = 'toast-' + Date.now();
    const bgClass = type === 'error' ? 'bg-danger' : 
                    type === 'success' ? 'bg-success' : 
                    type === 'warning' ? 'bg-warning' : 'bg-info';
    
    const toastHtml = `
        <div id="${toastId}" class="toast align-items-center text-white ${bgClass}" role="alert">
            <div class="d-flex">
                <div class="toast-body">${message}</div>
                <button type="button" class="btn-close btn-close-white me-2 m-auto" data-bs-dismiss="toast"></button>
            </div>
        </div>
    `;
    
    container.insertAdjacentHTML('beforeend', toastHtml);
    
    const toastEl = document.getElementById(toastId);
    const toast = new bootstrap.Toast(toastEl, { autohide: true, delay: 5000 });
    toast.show();
    
    toastEl.addEventListener('hidden.bs.toast', () => toastEl.remove());
}

/**
 * Copy text to clipboard
 */
function copyToClipboard(text) {
    navigator.clipboard.writeText(text).then(() => {
        showNotification('Copied to clipboard', 'success');
    }).catch(err => {
        console.error('Failed to copy:', err);
        showNotification('Failed to copy to clipboard', 'error');
    });
}

/**
 * Format date for display
 */
function formatDate(dateString) {
    const date = new Date(dateString);
    return date.toLocaleDateString() + ' ' + date.toLocaleTimeString();
}

/**
 * Debounce function for search inputs
 */
function debounce(func, wait) {
    let timeout;
    return function executedFunction(...args) {
        const later = () => {
            clearTimeout(timeout);
            func(...args);
        };
        clearTimeout(timeout);
        timeout = setTimeout(later, wait);
    };
}
