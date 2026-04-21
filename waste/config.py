"""
Configuration settings for the HPC Flask Application
"""
import os
from datetime import timedelta


def _default_database_uri() -> str:
    """Return a sensible default DB URL.

    - Prefer DATABASE_URL if provided.
    - On Windows, default to a local SQLite file so the app can run without
      requiring PostgreSQL to be installed/running.
    - On non-Windows, default to the project's PostgreSQL settings.
    """
    env_url = os.environ.get('DATABASE_URL')
    if env_url:
        return env_url

    if os.name == 'nt':
        base_dir = os.path.abspath(os.path.dirname(__file__))
        db_path = os.path.join(base_dir, 'hpc_tools_dev.db')
        # SQLAlchemy expects 3 slashes for absolute paths on Windows.
        return f"sqlite:///{db_path}"

    return 'postgresql://hpc_user:hpc_password@localhost:5432/hpc_tools_db'

class Config:
    """Base configuration"""
    SECRET_KEY = os.environ.get('SECRET_KEY') or 'your-secret-key-change-in-production'
    
    # Database Configuration
    SQLALCHEMY_DATABASE_URI = _default_database_uri()
    SQLALCHEMY_TRACK_MODIFICATIONS = False
    
    # Session Configuration
    PERMANENT_SESSION_LIFETIME = timedelta(hours=8)
    
    # HPC/Slurm Configuration
    SLURM_PARTITION = os.environ.get('SLURM_PARTITION') or 'compute'
    SLURM_ACCOUNT = os.environ.get('SLURM_ACCOUNT') or 'default'
    
    # File System Paths
    SCRATCH_DIR = os.environ.get('SCRATCH_DIR') or '/scratch/uploads'
    DATA_BASE_PATH = os.environ.get('DATA_BASE_PATH') or '/net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/2-Sim/USER_DATA'
    UPLOAD_FOLDER = os.environ.get('UPLOAD_FOLDER') or '/scratch/uploads'
    ALLOWED_EXTENSIONS = {'h5', 'hdf5', 'mf4', 'csv', 'json', 'xml'}
    MAX_CONTENT_LENGTH = 500 * 1024 * 1024  # 500MB max upload
    
    # Singularity Configuration
    SINGULARITY_IMAGE_PATH = os.environ.get('SINGULARITY_IMAGE_PATH') or '/apps/singularity/images'
    
    # LLM Configuration
    LLM_SERVICE_URL = os.environ.get('LLM_SERVICE_URL') or 'http://10.214.45.45:8000/generate'
    LLM_MODEL_NAME = os.environ.get('LLM_MODEL_NAME') or 'qwen'
    
    # ChromaDB Configuration
    CHROMADB_PATH = os.environ.get('CHROMADB_PATH') or './chromadb_data'
    
    # Tool Configurations
    TOOLS = {
        'dc_html': {
            'name': 'DC HTML Report',
            'description': 'Generate HTML reports from data collection',
            'singularity_image': 'dc_html.sif',
            'script': 'run_dc_html.sh'
        },
        'interactive_plot': {
            'name': 'Interactive Plot',
            'description': 'Create interactive visualizations',
            'singularity_image': 'interactive_plot.sif',
            'script': 'run_interactive_plot.sh'
        },
        'kpi': {
            'name': 'KPI Analysis',
            'description': 'Key Performance Indicator analysis',
            'singularity_image': 'kpi.sif',
            'script': 'run_kpi.sh'
        },
        'hyperlink_tool': {
            'name': 'Hyperlink Tool',
            'description': 'Generate hyperlinks for reports',
            'singularity_image': 'hyperlink.sif',
            'script': 'run_hyperlink.sh'
        }
    }


class DevelopmentConfig(Config):
    """Development configuration"""
    DEBUG = True
    SQLALCHEMY_ECHO = True


class ProductionConfig(Config):
    """Production configuration"""
    DEBUG = False
    SQLALCHEMY_ECHO = False


class TestingConfig(Config):
    """Testing configuration"""
    TESTING = True
    SQLALCHEMY_DATABASE_URI = 'sqlite:///:memory:'


config = {
    'development': DevelopmentConfig,
    'production': ProductionConfig,
    'testing': TestingConfig,
    'default': DevelopmentConfig
}
