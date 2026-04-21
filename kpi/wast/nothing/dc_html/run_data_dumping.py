#!/usr/bin/env python3
"""
Simple script to run data dumping functionality.
This script directly uses the DataDumper class to extract and save the specified datasets.
"""

import os
import sys
import logging
from d_business_layer.data_dumper import DataDumper

def setup_logging():
    """Setup logging configuration."""
    logging.basicConfig(
        level=logging.INFO,
        format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
        handlers=[
            logging.StreamHandler(),
            logging.FileHandler('data_dumping.log')
        ]
    )

def main():
    """Main function to run data dumping."""
    setup_logging()
    logger = logging.getLogger(__name__)
    
    # Configuration
    hdf_file = "hdf_dc.h5"
    output_dir = "html"
    
    # Check if HDF file exists
    if not os.path.exists(hdf_file):
        logger.error(f"HDF file not found: {hdf_file}")
        print(f"❌ HDF file not found: {hdf_file}")
        return False
    
    logger.info(f"Starting data dumping process with file: {hdf_file}")
    print(f"📁 Processing HDF file: {hdf_file}")
    
    # Create output directory
    os.makedirs(output_dir, exist_ok=True)
    
    try:
        # Create DataDumper instance
        dumper = DataDumper(hdf_file, output_dir)
        
        # Dump all data
        logger.info("Dumping Tracker_Information/OLP data...")
        print("🔄 Dumping Tracker_Information/OLP data...")
        tracker_success = dumper.dump_tracker_data()
        
        logger.info("Dumping OSI_Ground_Truth/Object data...")
        print("🔄 Dumping OSI_Ground_Truth/Object data...")
        osi_success = dumper.dump_osi_data()
        
        # Report results
        logger.info("=== Data Dumping Results ===")
        print("\n=== Data Dumping Results ===")
        
        if tracker_success:
            logger.info("✅ Tracker_Information/OLP data dumped successfully")
            print("✅ Tracker_Information/OLP data dumped successfully")
        else:
            logger.error("❌ Failed to dump Tracker_Information/OLP data")
            print("❌ Failed to dump Tracker_Information/OLP data")
        
        if osi_success:
            logger.info("✅ OSI_Ground_Truth/Object data dumped successfully")
            print("✅ OSI_Ground_Truth/Object data dumped successfully")
        else:
            logger.error("❌ Failed to dump OSI_Ground_Truth/Object data")
            print("❌ Failed to dump OSI_Ground_Truth/Object data")
        
        # Check output files
        track_file = os.path.join(output_dir, "Track.txt")
        osi_file = os.path.join(output_dir, "OSI track.txt")
        
        if os.path.exists(track_file):
            size = os.path.getsize(track_file)
            logger.info(f"📄 Track.txt created: {size} bytes")
            print(f"📄 Track.txt created: {size} bytes")
        
        if os.path.exists(osi_file):
            size = os.path.getsize(osi_file)
            logger.info(f"📄 OSI track.txt created: {size} bytes")
            print(f"📄 OSI track.txt created: {size} bytes")
        
        success = tracker_success and osi_success
        if success:
            logger.info("🎉 All data dumping completed successfully!")
            print("🎉 All data dumping completed successfully!")
        else:
            logger.error("⚠️ Some data dumping operations failed")
            print("⚠️ Some data dumping operations failed")
        
        return success
        
    except Exception as e:
        logger.error(f"Error during data dumping: {e}")
        print(f"❌ Error during data dumping: {e}")
        return False

if __name__ == "__main__":
    success = main()
    if not success:
        sys.exit(1) 