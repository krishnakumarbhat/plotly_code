#!/usr/bin/env python3
"""
Minimal script to run data dumping functionality using SOLID principles.
Extracts and dumps specified datasets from HDF files using DataModelStorage format.
"""

import os
import sys
import logging
from d_business_layer.data_dumper import DataDumper
from d_business_layer.utils import LoggerSetup
from c_data_storage.config_storage import ConfigurationManager
from c_data_storage.interfaces import ILogger

def create_logger() -> ILogger:
    """Create and configure logger instance."""
    logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
    return LoggerSetup("html")

def main():
    """Main function to run data dumping with SOLID principles."""
    # Setup logging
    logger = create_logger()
    
    # Configuration
    hdf_file = "hdf_dc.h5"
    output_dir = "html"
    
    # Check if HDF file exists
    if not os.path.exists(hdf_file):
        logger.error(f"HDF file not found: {hdf_file}")
        return False
    
    logger.info(f"Processing HDF file: {hdf_file}")
    
    try:
        # Create configuration manager
        config_manager = ConfigurationManager()
        
        # Create DataDumper with dependency injection
        dumper = DataDumper(
            hdf_file_path=hdf_file,
            output_dir=output_dir,
            config=config_manager,
            logger=logger
        )
        
        # Dump all data
        results = dumper.dump_all_data()
        
        # Report results
        success = all(results.values())
        if success:
            logger.info("✅ All data dumped successfully!")
            
            # Display storage information
            storage_info = dumper.get_storage_info()
            logger.info("📊 Storage Information:")
            for source, info in storage_info.items():
                logger.info(f"   {source}: {info['scan_indices']} scan indices, {info['signals']} signals")
        else:
            logger.error("❌ Some data dumping operations failed")
            for operation, result in results.items():
                status = "✅" if result else "❌"
                logger.info(f"   {status} {operation}: {'Success' if result else 'Failed'}")
        
        return success
        
    except Exception as e:
        logger.error(f"Error during data dumping: {e}")
        return False

if __name__ == "__main__":
    success = main()
    if not success:
        sys.exit(1) 