"""
KPI Process Manager
Handles separate process execution for KPI processing
"""

import multiprocessing as mp
import subprocess
import sys
import os
import logging
import time
import signal
from typing import Dict, Any, Optional, List, Tuple
from pathlib import Path
import json
import tempfile

# Add parent directory to path for imports
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from a_persistence_layer.kpi_zmq_communication import KPIProcessManager, KPIZMQClient, MessageType, KPIMessage
from a_persistence_layer.kpi_hdf_parser import KPIHDFParser
from c_business_layer.kpi_factory import KpiDataModel
from d_presentation_layer.kpi_html_gen import KPIHTMLGenerator

logger = logging.getLogger(__name__)

class KPIProcessExecutor:
    """Executes KPI processing in a separate process"""
    
    def __init__(self, server_port: int = 5555):
        self.server_port = server_port
        self.process_manager = KPIProcessManager(server_port)

        self.active_processes = {}
        
    def start_kpi_server(self):
        """Start the KPI server to receive messages from InteractivePlot"""
        self.process_manager.start_server()
        
        # Register handlers
        self.process_manager.register_sensor_data_handler(self._handle_sensor_data_ready)
        self.process_manager.register_kpi_complete_handler(self._handle_kpi_complete)
        
        logger.info("KPI Process Manager started and ready to receive messages")
        
    def _handle_sensor_data_ready(self, message_data: Dict[str, Any]) -> Dict[str, Any]:
        """Handle sensor data ready message from InteractivePlot"""
        try:
            sensor_id = message_data.get("sensor_id")
            data_path = message_data.get("data_path")
            metadata = message_data.get("metadata", {})
            
            logger.info(f"Received sensor data ready for sensor {sensor_id}")
            
            # Start KPI processing in separate process
            process = self._start_kpi_processing_process(sensor_id, data_path, metadata)
            self.active_processes[sensor_id] = process
            
            return {
                "status": "processing_started",
                "sensor_id": sensor_id,
                "process_id": process.pid
            }
            
        except Exception as e:
            logger.error(f"Error handling sensor data ready: {e}")
            return {
                "status": "error",
                "message": str(e)
            }
            
    def _handle_kpi_complete(self, message_data: Dict[str, Any]) -> Dict[str, Any]:
        """Handle KPI completion message"""
        try:
            sensor_id = message_data.get("sensor_id")
            html_report_path = message_data.get("html_report_path")
            
            logger.info(f"KPI processing completed for sensor {sensor_id}")
            
            # Clean up process
            if sensor_id in self.active_processes:
                del self.active_processes[sensor_id]
                
            return {
                "status": "completed",
                "sensor_id": sensor_id,
                "html_report_path": html_report_path
            }
            
        except Exception as e:
            logger.error(f"Error handling KPI complete: {e}")
            return {
                "status": "error",
                "message": str(e)
            }
            
    def _start_kpi_processing_process(self, sensor_id: str, data_path: str, metadata: Dict[str, Any]) -> mp.Process:
        """Start KPI processing in a separate process"""
        
        def kpi_processing_worker(sensor_id: str, data_path: str, server_port: int, metadata: Dict[str, Any]):
            """Worker function for KPI processing"""
            try:
                logger.info(f"Starting KPI processing for sensor {sensor_id}")
                
                # Initialize components
                hdf_parser = KPIHDFParser()
                kpi_factory = KpiDataModel()
                html_generator = KPIHTMLGenerator()
                client = KPIZMQClient(port=server_port)
                
                # Parse HDF data for KPI
                kpi_data = hdf_parser.parse_sensor_data_for_kpi(data_path, sensor_id)
                
                # Validate data completeness
                is_complete, missing_fields = hdf_parser.validate_kpi_data_completeness(kpi_data)
                if not is_complete:
                    logger.warning(f"Missing KPI data fields: {missing_fields}")
                    
                # Preprocess data
                processed_data = hdf_parser.preprocess_kpi_data(kpi_data)
                
                # Generate KPI metrics
                kpi_metrics = {}
                
                # Alignment KPI
                if "alignment_stream" in processed_data:
                    alignment_kpi = kpi_factory.create_kpi("alignment")
                    alignment_metrics = alignment_kpi.calculate_kpi(processed_data["alignment_stream"])
                    kpi_metrics.update(alignment_metrics)
                    
                # Detection KPI
                if "detection_stream" in processed_data:
                    detection_kpi = kpi_factory.create_kpi("detection")
                    detection_metrics = detection_kpi.calculate_kpi(processed_data["detection_stream"])
                    kpi_metrics.update(detection_metrics)
                    
                # Tracker KPI
                if "tracker_stream" in processed_data:
                    tracker_kpi = kpi_factory.create_kpi("tracker")
                    tracker_metrics = tracker_kpi.calculate_kpi(processed_data["tracker_stream"])
                    kpi_metrics.update(tracker_metrics)
                    
                # Generate HTML report
                report_data = {
                    "sensor_id": sensor_id,
                    "kpi_metrics": kpi_metrics,
                    "data_summary": hdf_parser.get_kpi_data_summary(processed_data),
                    "processing_timestamp": time.time()
                }
                
                # Determine desired output location based on metadata
                # Expecting metadata keys: output_dir, base_name (input base), kpi_subdir (optional), kpi_filename (optional)
                output_dir = metadata.get("output_dir")
                base_name = metadata.get("base_name")
                kpi_subdir = metadata.get("kpi_subdir", "kpi")
                kpi_filename = metadata.get("kpi_filename", f"{sensor_id}_kpi.html")

                html_report_path = None
                if output_dir and base_name:
                    base_folder = Path(output_dir) / base_name / kpi_subdir
                    base_folder.mkdir(parents=True, exist_ok=True)
                    html_path = base_folder / kpi_filename
                    html_content = html_generator.generate_kpi_report(report_data)
                    with open(html_path, 'w', encoding='utf-8') as f:
                        f.write(html_content)
                    html_report_path = str(html_path)
                else:
                    # Fallback temporary file if metadata not provided
                    with tempfile.NamedTemporaryFile(mode='w', suffix='.html', delete=False) as f:
                        html_content = html_generator.generate_kpi_report(report_data)
                        f.write(html_content)
                        html_report_path = f.name
                    
                # Notify completion
                client.notify_kpi_complete(sensor_id, html_report_path)
                
                logger.info(f"KPI processing completed for sensor {sensor_id}")
                
            except Exception as e:
                logger.error(f"Error in KPI processing worker: {e}")
                # Send error notification
                try:
                    client.send_message(KPIMessage(
                        message_type=MessageType.ERROR,
                        sensor_id=sensor_id,
                        error_message=str(e)
                    ))
                except:
                    pass
                    
        # Start process
        process = mp.Process(
            target=kpi_processing_worker,
            args=(sensor_id, data_path, self.server_port, metadata),
            daemon=True
        )
        process.start()
        
        logger.info(f"Started KPI processing process (PID: {process.pid}) for sensor {sensor_id}")
        return process
        
    def stop_all_processes(self):
        """Stop all active KPI processes"""
        for sensor_id, process in self.active_processes.items():
            try:
                if process.is_alive():
                    process.terminate()
                    process.join(timeout=5)
                    if process.is_alive():
                        process.kill()
                logger.info(f"Stopped KPI process for sensor {sensor_id}")
            except Exception as e:
                logger.error(f"Error stopping process for sensor {sensor_id}: {e}")
                
        self.active_processes.clear()
        
    def get_process_status(self) -> Dict[str, Any]:
        """Get status of all active processes"""
        status = {}
        for sensor_id, process in self.active_processes.items():
            status[sensor_id] = {
                "pid": process.pid,
                "alive": process.is_alive(),
                "exitcode": process.exitcode
            }
        return status
        
    def cleanup(self):
        """Cleanup resources"""
        self.stop_all_processes()
        self.process_manager.close()

class KPIProcessLauncher:
    """Launches KPI processing as a separate executable process"""
    
    def __init__(self, kpi_script_path: Optional[str] = None):
        self.kpi_script_path = kpi_script_path or self._get_kpi_script_path()
        
    def _get_kpi_script_path(self) -> str:
        """Get the path to the KPI processing script"""
        current_dir = Path(__file__).parent
        script_path = current_dir / "kpi_standalone_processor.py"
        return str(script_path)
        
    def launch_kpi_process(self, sensor_id: str, data_path: str, 
                          server_port: int = 5555) -> subprocess.Popen:
        """Launch KPI processing as a separate process"""
        
        cmd = [
            sys.executable,
            self.kpi_script_path,
            "--sensor-id", sensor_id,
            "--data-path", data_path,
            "--server-port", str(server_port)
        ]
        
        try:
            process = subprocess.Popen(
                cmd,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True
            )
            
            logger.info(f"Launched KPI process (PID: {process.pid}) for sensor {sensor_id}")
            return process
            
        except Exception as e:
            logger.error(f"Error launching KPI process: {e}")
            raise
            
    def check_process_status(self, process: subprocess.Popen) -> Dict[str, Any]:
        """Check the status of a launched process"""
        return {
            "pid": process.pid,
            "returncode": process.returncode,
            "poll": process.poll()
        }
        
    def get_process_output(self, process: subprocess.Popen) -> Tuple[str, str]:
        """Get stdout and stderr from a process"""
        stdout, stderr = process.communicate()
        return stdout, stderr 