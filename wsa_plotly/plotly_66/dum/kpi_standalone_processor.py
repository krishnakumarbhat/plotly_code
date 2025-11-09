#!/usr/bin/env python3
"""
Standalone KPI Processor
Can be executed as a separate process for KPI processing
"""

import argparse
import sys
import os
import logging
import time
from pathlib import Path

# Add parent directory to path for imports
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from a_persistence_layer.kpi_hdf_parser import KPIHDFParser
from a_persistence_layer.kpi_zmq_communication import KPIZMQClient, MessageType, KPIMessage
from c_business_layer.kpi_factory import KPIFactory
from d_presentation_layer.kpi_html_gen import KPIHTMLGenerator

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)

def main():
    """Main function for standalone KPI processing"""
    parser = argparse.ArgumentParser(description='Standalone KPI Processor')
    parser.add_argument('--sensor-id', required=True, help='Sensor ID')
    parser.add_argument('--data-path', required=True, help='Path to HDF data file')
    parser.add_argument('--server-port', type=int, default=5555, help='ZMQ server port')
    parser.add_argument('--output-dir', help='Output directory for HTML reports')
    
    args = parser.parse_args()
    
    try:
        logger.info(f"Starting KPI processing for sensor {args.sensor_id}")
        
        # Initialize components
        hdf_parser = KPIHDFParser()
        kpi_factory = KPIFactory()
        html_generator = KPIHTMLGenerator()
        client = KPIZMQClient(port=args.server_port)
        
        # Parse HDF data for KPI
        logger.info(f"Parsing HDF file: {args.data_path}")
        kpi_data = hdf_parser.parse_sensor_data_for_kpi(args.data_path, args.sensor_id)
        
        # Validate data completeness
        is_complete, missing_fields = hdf_parser.validate_kpi_data_completeness(kpi_data)
        if not is_complete:
            logger.warning(f"Missing KPI data fields: {missing_fields}")
            
        # Preprocess data
        logger.info("Preprocessing KPI data")
        processed_data = hdf_parser.preprocess_kpi_data(kpi_data)
        
        # Generate KPI metrics
        logger.info("Calculating KPI metrics")
        kpi_metrics = {}
        
        # Alignment KPI
        if "alignment_stream" in processed_data and processed_data["alignment_stream"]:
            logger.info("Processing alignment KPI")
            alignment_kpi = kpi_factory.create_kpi("alignment")
            alignment_metrics = alignment_kpi.calculate_kpi(processed_data["alignment_stream"])
            kpi_metrics.update(alignment_metrics)
            
        # Detection KPI
        if "detection_stream" in processed_data and processed_data["detection_stream"]:
            logger.info("Processing detection KPI")
            detection_kpi = kpi_factory.create_kpi("detection")
            detection_metrics = detection_kpi.calculate_kpi(processed_data["detection_stream"])
            kpi_metrics.update(detection_metrics)
            
        # Tracker KPI
        if "tracker_stream" in processed_data and processed_data["tracker_stream"]:
            logger.info("Processing tracker KPI")
            tracker_kpi = kpi_factory.create_kpi("tracker")
            tracker_metrics = tracker_kpi.calculate_kpi(processed_data["tracker_stream"])
            kpi_metrics.update(tracker_metrics)
            
        # Generate HTML report
        logger.info("Generating HTML report")
        report_data = {
            "sensor_id": args.sensor_id,
            "kpi_metrics": kpi_metrics,
            "data_summary": hdf_parser.get_kpi_data_summary(processed_data),
            "processing_timestamp": time.time(),
            "missing_fields": missing_fields
        }
        
        # Determine output path
        if args.output_dir:
            output_dir = Path(args.output_dir)
            output_dir.mkdir(parents=True, exist_ok=True)
            html_report_path = output_dir / f"kpi_report_{args.sensor_id}_{int(time.time())}.html"
        else:
            # Use temporary file
            import tempfile
            with tempfile.NamedTemporaryFile(mode='w', suffix='.html', delete=False) as f:
                html_report_path = Path(f.name)
                
        # Generate and save HTML report
        html_content = html_generator.generate_kpi_report(report_data)
        with open(html_report_path, 'w', encoding='utf-8') as f:
            f.write(html_content)
            
        logger.info(f"HTML report generated: {html_report_path}")
        
        # Notify completion
        logger.info("Notifying completion to InteractivePlot")
        response = client.notify_kpi_complete(args.sensor_id, str(html_report_path))
        
        if response.get("status") == "success":
            logger.info("Successfully notified InteractivePlot of KPI completion")
        else:
            logger.warning(f"Failed to notify InteractivePlot: {response}")
            
        # Print summary
        print(f"KPI processing completed for sensor {args.sensor_id}")
        print(f"HTML report: {html_report_path}")
        print(f"KPI metrics calculated: {len(kpi_metrics)}")
        print(f"Data completeness: {is_complete}")
        
        return 0
        
    except Exception as e:
        logger.error(f"Error in KPI processing: {e}")
        
        # Try to send error notification
        try:
            client.send_message(KPIMessage(
                message_type=MessageType.ERROR,
                sensor_id=args.sensor_id,
                error_message=str(e)
            ))
        except:
            pass
            
        return 1
        
    finally:
        client.close()

if __name__ == "__main__":
    sys.exit(main()) 