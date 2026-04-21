"""
Utility functions for HPC Flask Application
Includes Slurm job submission, file browser, and helper functions
"""
import os
import subprocess
import uuid
import re
from datetime import datetime
from pathlib import Path
from typing import Optional, Dict, List, Tuple
import logging

logger = logging.getLogger(__name__)


class SlurmManager:
    """Manager class for Slurm job submission and monitoring"""
    
    def __init__(self, partition: str = 'compute', account: str = 'default'):
        self.partition = partition
        self.account = account
    
    def submit_job(self, 
                   script_path: str, 
                   args: List[str] = None,
                   job_name: str = None,
                   time_limit: str = '01:00:00',
                   memory: str = '8G',
                   cpus: int = 4,
                   gpu: bool = False,
                   output_dir: str = '/scratch/logs') -> Tuple[bool, str, str]:
        """
        Submit a job to Slurm using sbatch
        
        Args:
            script_path: Path to the shell script to execute
            args: Additional arguments for the script
            job_name: Name for the Slurm job
            time_limit: Time limit in HH:MM:SS format
            memory: Memory allocation (e.g., '8G')
            cpus: Number of CPUs
            gpu: Whether to request GPU resources
            output_dir: Directory for output logs
        
        Returns:
            Tuple of (success, job_id, error_message)
        """
        if job_name is None:
            job_name = f"hpc_tool_{uuid.uuid4().hex[:8]}"
        
        # Ensure output directory exists
        os.makedirs(output_dir, exist_ok=True)
        
        # Build sbatch command
        cmd = [
            'sbatch',
            '--parsable',
            f'--partition={self.partition}',
            f'--account={self.account}',
            f'--job-name={job_name}',
            f'--time={time_limit}',
            f'--mem={memory}',
            f'--cpus-per-task={cpus}',
            f'--output={output_dir}/slurm_{job_name}_%j.out',
            f'--error={output_dir}/slurm_{job_name}_%j.err'
        ]
        
        if gpu:
            cmd.append('--gres=gpu:1')
        
        cmd.append(script_path)
        
        if args:
            cmd.extend(args)
        
        try:
            result = subprocess.run(
                cmd,
                capture_output=True,
                text=True,
                timeout=30
            )
            
            if result.returncode == 0:
                job_id = result.stdout.strip()
                logger.info(f"Submitted Slurm job: {job_id}")
                return True, job_id, None
            else:
                error_msg = result.stderr.strip()
                logger.error(f"Slurm submission failed: {error_msg}")
                return False, None, error_msg
                
        except subprocess.TimeoutExpired:
            return False, None, "Slurm submission timed out"
        except Exception as e:
            return False, None, str(e)
    
    def check_job_status(self, job_id: str) -> Dict[str, str]:
        """
        Check the status of a Slurm job
        
        Args:
            job_id: The Slurm job ID
        
        Returns:
            Dictionary with job status information
        """
        try:
            # First try squeue for running/pending jobs
            result = subprocess.run(
                ['squeue', '-j', job_id, '-h', '-o', '%T|%M|%N|%r'],
                capture_output=True,
                text=True,
                timeout=10
            )
            
            if result.returncode == 0 and result.stdout.strip():
                parts = result.stdout.strip().split('|')
                return {
                    'status': parts[0] if len(parts) > 0 else 'UNKNOWN',
                    'time': parts[1] if len(parts) > 1 else '',
                    'node': parts[2] if len(parts) > 2 else '',
                    'reason': parts[3] if len(parts) > 3 else ''
                }
            
            # Job not in queue, check sacct for completed jobs
            result = subprocess.run(
                ['sacct', '-j', job_id, '-n', '-o', 'State,ExitCode,Elapsed', '--parsable2'],
                capture_output=True,
                text=True,
                timeout=10
            )
            
            if result.returncode == 0 and result.stdout.strip():
                lines = result.stdout.strip().split('\n')
                if lines:
                    parts = lines[0].split('|')
                    return {
                        'status': parts[0] if len(parts) > 0 else 'COMPLETED',
                        'exit_code': parts[1] if len(parts) > 1 else '',
                        'elapsed': parts[2] if len(parts) > 2 else ''
                    }
            
            return {'status': 'UNKNOWN'}
            
        except Exception as e:
            logger.error(f"Error checking job status: {e}")
            return {'status': 'ERROR', 'error': str(e)}
    
    def cancel_job(self, job_id: str) -> Tuple[bool, str]:
        """Cancel a Slurm job"""
        try:
            result = subprocess.run(
                ['scancel', job_id],
                capture_output=True,
                text=True,
                timeout=10
            )
            
            if result.returncode == 0:
                return True, f"Job {job_id} cancelled"
            else:
                return False, result.stderr.strip()
                
        except Exception as e:
            return False, str(e)


class FileBrowser:
    """Secure file browser for HPC file system"""
    
    def __init__(self, base_paths: List[str]):
        """
        Initialize file browser with allowed base paths
        
        Args:
            base_paths: List of allowed root directories
        """
        self.base_paths = [Path(p).resolve() for p in base_paths]
    
    def is_path_allowed(self, path: str) -> bool:
        """Check if a path is within allowed directories"""
        try:
            resolved = Path(path).resolve()
            return any(
                resolved == base or base in resolved.parents
                for base in self.base_paths
            )
        except Exception:
            return False
    
    def list_directory(self, path: str, 
                       extensions: List[str] = None) -> Dict[str, List[Dict]]:
        """
        List contents of a directory
        
        Args:
            path: Directory path to list
            extensions: Optional list of file extensions to filter
        
        Returns:
            Dictionary with 'directories' and 'files' lists
        """
        if not self.is_path_allowed(path):
            raise PermissionError(f"Access denied to path: {path}")
        
        target = Path(path)
        if not target.exists():
            raise FileNotFoundError(f"Path does not exist: {path}")
        
        if not target.is_dir():
            raise NotADirectoryError(f"Not a directory: {path}")
        
        result = {'directories': [], 'files': [], 'current_path': str(target)}
        
        try:
            for item in sorted(target.iterdir()):
                if item.name.startswith('.'):
                    continue  # Skip hidden files
                
                item_info = {
                    'name': item.name,
                    'path': str(item),
                    'modified': datetime.fromtimestamp(item.stat().st_mtime).isoformat()
                }
                
                if item.is_dir():
                    result['directories'].append(item_info)
                elif item.is_file():
                    item_info['size'] = item.stat().st_size
                    item_info['extension'] = item.suffix.lower()
                    
                    if extensions:
                        if item.suffix.lower().lstrip('.') in extensions:
                            result['files'].append(item_info)
                    else:
                        result['files'].append(item_info)
        
        except PermissionError:
            raise PermissionError(f"Permission denied reading: {path}")
        
        return result
    
    def get_parent_path(self, path: str) -> Optional[str]:
        """Get parent directory if allowed"""
        parent = Path(path).parent
        if self.is_path_allowed(str(parent)):
            return str(parent)
        return None


def extract_hdf_filename(path: str) -> str:
    """Extract HDF filename from path for display"""
    if not path:
        return ''
    
    # Get basename
    basename = os.path.basename(path)
    
    # Match common HDF patterns
    hdf_pattern = re.compile(r'.*\.(h5|hdf5|hdf)$', re.IGNORECASE)
    if hdf_pattern.match(basename):
        return basename
    
    return basename


def generate_slurm_script(tool_name: str,
                          input_path: str,
                          output_path: str,
                          singularity_image: str,
                          script_content: str = None,
                          job_name: str = None) -> str:
    """
    Generate a Slurm batch script for tool execution
    
    Args:
        tool_name: Name of the tool
        input_path: Input file path
        output_path: Output directory path
        singularity_image: Path to Singularity image
        script_content: Custom script content
        job_name: Custom job name
    
    Returns:
        Path to generated script file
    """
    if job_name is None:
        job_name = f"{tool_name}_{uuid.uuid4().hex[:8]}"
    
    script = f"""#!/bin/bash
#SBATCH --job-name={job_name}
#SBATCH --output=/scratch/logs/{job_name}_%j.out
#SBATCH --error=/scratch/logs/{job_name}_%j.err
#SBATCH --time=02:00:00
#SBATCH --mem=16G
#SBATCH --cpus-per-task=4

# Load Singularity module if needed
module load singularity 2>/dev/null || true

# Set up environment
export INPUT_PATH="{input_path}"
export OUTPUT_PATH="{output_path}"

# Create output directory
mkdir -p "$OUTPUT_PATH"

# Run tool in Singularity container
singularity exec \\
    --bind /scratch:/scratch \\
    --bind /net:/net \\
    {singularity_image} \\
    python /app/{tool_name}/main.py \\
    --input "$INPUT_PATH" \\
    --output "$OUTPUT_PATH"

# Capture exit code
EXIT_CODE=$?

echo "Job completed with exit code: $EXIT_CODE"
exit $EXIT_CODE
"""
    
    if script_content:
        script = script_content
    
    # Write script to file
    script_path = f"/scratch/scripts/{job_name}.sh"
    os.makedirs(os.path.dirname(script_path), exist_ok=True)
    
    with open(script_path, 'w') as f:
        f.write(script)
    
    os.chmod(script_path, 0o755)
    
    return script_path


def format_file_size(size_bytes: int) -> str:
    """Format file size in human-readable format"""
    for unit in ['B', 'KB', 'MB', 'GB', 'TB']:
        if size_bytes < 1024.0:
            return f"{size_bytes:.1f} {unit}"
        size_bytes /= 1024.0
    return f"{size_bytes:.1f} PB"


def sanitize_path(path: str) -> str:
    """Sanitize file path to prevent path traversal attacks"""
    # Remove null bytes
    path = path.replace('\x00', '')
    
    # Normalize path
    path = os.path.normpath(path)
    
    # Remove any parent directory references that escape
    parts = path.split(os.sep)
    safe_parts = []
    for part in parts:
        if part == '..':
            if safe_parts and safe_parts[-1] != '':
                safe_parts.pop()
        elif part != '.':
            safe_parts.append(part)
    
    return os.sep.join(safe_parts)
