"""
Gunicorn configuration for HPC Tools Platform

This config is optimized for:
- Video streaming (large file transfers)
- Multiple concurrent users
- Long-running SSH connections
"""
import os
import multiprocessing

# Server socket
bind = f"{os.environ.get('HOST', '0.0.0.0')}:{os.environ.get('PORT', '5001')}"
backlog = 2048

# Worker processes
workers = int(os.environ.get('WORKERS', min(multiprocessing.cpu_count() * 2 + 1, 8)))

# Use gthread worker class for better handling of:
# - Video streaming (long-lived connections)
# - WebSocket-like behavior
# - SSH session management
worker_class = 'gthread'
threads = 4

# Timeouts - increased for video streaming
# 5 minutes should handle most video seek/stream operations
timeout = int(os.environ.get('TIMEOUT', 300))
graceful_timeout = 30
keepalive = 5

# Request handling
max_requests = 1000  # Recycle workers periodically to prevent memory leaks
max_requests_jitter = 100

# Logging
accesslog = '-'
errorlog = '-'
loglevel = os.environ.get('LOG_LEVEL', 'info')

# Security
limit_request_line = 8190
limit_request_fields = 100

# Server mechanics
daemon = False
pidfile = None
tmp_upload_dir = None

# Handle SIGTERM gracefully
def on_exit(server):
    """Clean up resources on server exit."""
    pass

# Worker lifecycle hooks
def worker_exit(server, worker):
    """Called when a worker exits - cleanup any lingering resources."""
    pass

def pre_fork(server, worker):
    """Called just before a worker is forked."""
    pass

def post_fork(server, worker):
    """Called just after a worker has been forked."""
    pass
