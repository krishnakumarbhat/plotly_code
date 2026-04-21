import pandas as pd
import os
import re
import sys
import json
from abc import ABC, abstractmethod
from typing import Dict, List, Set, Tuple, Optional, Any


class CSVFileStrategy(ABC):
    """Abstract strategy for identifying CSV files"""
    @abstractmethod
    def identify_files(self, directory: str) -> Tuple[Optional[str], Optional[str]]:
        """Identify input and output files in the given directory"""
        pass


class RNumberCSVStrategy(CSVFileStrategy):
    """Strategy to identify files based on _r{number} pattern"""
    def identify_files(self, directory: str) -> Tuple[Optional[str], Optional[str]]:
        csv_files = [f for f in os.listdir(directory) if f.endswith('.csv')]
        input_file = None
        output_file = None
        
        for file in csv_files:
            if re.search(r'_r\d+', file):
                output_file = os.path.join(directory, file)
            else:
                input_file = os.path.join(directory, file)
        
        return input_file, output_file


class JSONConfigStrategy(CSVFileStrategy):
    """Strategy to identify files based on a JSON configuration file"""
    def __init__(self, json_file: str = 'input.json'):
        self.json_file = json_file
    
    def identify_files(self, directory: str) -> Tuple[Optional[str], Optional[str]]:
        json_path = os.path.join(directory, self.json_file)
        
        if not os.path.exists(json_path):
            print(f"JSON configuration file {json_path} not found.")
            return None, None
        
        try:
            with open(json_path, 'r') as f:
                config = json.load(f)
            
            input_files = config.get('INPUT', [])
            output_files = config.get('OUTPUT', [])
            
            input_file = input_files[0] if input_files else None
            output_file = output_files[0] if output_files else None
            
            return input_file, output_file
        except Exception as e:
            print(f"Error reading JSON configuration: {e}")
            return None, None


class CSVDataLoader:
    """Responsible for loading and preparing CSV data"""
    def __init__(self, file_strategy: CSVFileStrategy):
        self.file_strategy = file_strategy
        self.input_df = None
        self.output_df = None
        self.scan_index_col = None
        self.rdd_cols = []
        self.common_df_input = None
        self.common_df_output = None
        self.missing_in_output = set()
        self.missing_in_input = set()
    
    def load_data(self, directory: str) -> bool:
        """Load data from CSV files in the specified directory"""
        input_file, output_file = self.file_strategy.identify_files(directory)
        
        if not input_file or not output_file:
            print("Could not identify input and output files.")
            return False
        
        print(f"Input file: {os.path.basename(input_file)}")
        print(f"Output file: {os.path.basename(output_file)}")
        
        try:
            print("Reading input file...")
            self.input_df = pd.read_csv(input_file)
            print("Reading output file...")
            self.output_df = pd.read_csv(output_file)
            return True
        except Exception as e:
            print(f"Error loading CSV files: {e}")
            return False
    
    def validate_and_prepare_data(self) -> bool:
        """Validate and prepare the loaded data for analysis"""
        if self.input_df is None or self.output_df is None:
            print("Data not loaded. Call load_data() first.")
            return False
        
        # Find common scan_index column
        for col in ['scan_index', 'scanindex', 'SCAN_INDEX', 'ScanIndex']:
            if col in self.input_df.columns and col in self.output_df.columns:
                self.scan_index_col = col
                break
        
        if not self.scan_index_col:
            print("Could not find scan_index column in both files.")
            print(f"Input columns: {self.input_df.columns.tolist()[:10]}...")
            print(f"Output columns: {self.output_df.columns.tolist()[:10]}...")
            return False
        
        # Find rdd1_dindx columns
        self.rdd_cols = [col for col in self.input_df.columns if col.startswith('rdd1_dindx_')]
        
        if not self.rdd_cols:
            print("Could not find any columns starting with 'rdd1_dindx_'.")
            return False
        
        print(f"Found {len(self.rdd_cols)} rdd1_dindx columns. Examples: {self.rdd_cols[:5]}")
        
        # Check for missing indices
        input_indices = set(self.input_df[self.scan_index_col])
        output_indices = set(self.output_df[self.scan_index_col])
        
        self.missing_in_output = input_indices - output_indices
        self.missing_in_input = output_indices - input_indices
        
        # Save missing indices information
        self._save_missing_indices(input_indices, output_indices)
        
        # Prepare common dataframes for comparison
        common_indices = sorted(list(input_indices.intersection(output_indices)))
        self.common_df_input = self.input_df[self.input_df[self.scan_index_col].isin(common_indices)].sort_values(by=self.scan_index_col)
        self.common_df_output = self.output_df[self.output_df[self.scan_index_col].isin(common_indices)].sort_values(by=self.scan_index_col)
        
        return True
    
    def _save_missing_indices(self, input_indices: Set, output_indices: Set) -> None:
        """Save missing indices to CSV files"""
        directory = os.getcwd()
        
        if self.missing_in_output:
            print(f"Found {len(self.missing_in_output)} scan indices in input but missing in output.")
            print(f"Examples: {sorted(list(self.missing_in_output))[:10]}")
            
            missing_output_df = pd.DataFrame({self.scan_index_col: sorted(list(self.missing_in_output))})
            missing_output_file = os.path.join(directory, 'missing_in_output.csv')
            missing_output_df.to_csv(missing_output_file, index=False)
            print(f"Missing indices saved to {missing_output_file}")
        
        if self.missing_in_input:
            print(f"Found {len(self.missing_in_input)} scan indices in output but missing in input.")
            print(f"Examples: {sorted(list(self.missing_in_input))[:10]}")
            
            missing_input_df = pd.DataFrame({self.scan_index_col: sorted(list(self.missing_in_input))})
            missing_input_file = os.path.join(directory, 'missing_in_input.csv')
            missing_input_df.to_csv(missing_input_file, index=False)
            print(f"Missing indices saved to {missing_input_file}")


class DataPreparationFacade:
    """Facade pattern to simplify the data preparation process"""
    def __init__(self, strategy: CSVFileStrategy = None, use_json: bool = False, json_file: str = 'input.json'):
        if strategy is None:
            if use_json:
                strategy = JSONConfigStrategy(json_file)
            else:
                strategy = RNumberCSVStrategy()
        self.loader = CSVDataLoader(strategy)
    
    def prepare_data(self, directory: str = None) -> Optional[CSVDataLoader]:
        """Prepare data for analysis"""
        if directory is None:
            directory = os.getcwd()
        
        print(f"Looking for CSV files in: {directory}")
        
        if not self.loader.load_data(directory):
            return None
        
        if not self.loader.validate_and_prepare_data():
            return None
        
        return self.loader


class DataPreparationFactory:
    """Factory for creating data preparation strategies"""
    @staticmethod
    def create_facade(strategy_type: str = 'auto', json_file: str = 'input.json') -> DataPreparationFacade:
        """Create a data preparation facade with the specified strategy"""
        if strategy_type.lower() == 'json':
            return DataPreparationFacade(JSONConfigStrategy(json_file))
        elif strategy_type.lower() == 'pattern':
            return DataPreparationFacade(RNumberCSVStrategy())
        else:  # 'auto' - try JSON first, then pattern
            # Check if JSON file exists
            if os.path.exists(os.path.join(os.getcwd(), json_file)):
                return DataPreparationFacade(JSONConfigStrategy(json_file))
            else:
                return DataPreparationFacade(RNumberCSVStrategy())


# Example usage when run directly
if __name__ == "__main__":
    # This file can be run directly for testing
    # Use the factory to create the appropriate facade
    factory = DataPreparationFactory()
    
    # Try automatic strategy selection (will use JSON if available, otherwise pattern)
    facade = factory.create_facade('auto')
    data_loader = facade.prepare_data()
    
    if data_loader:
        print("Data preparation successful!")
        print(f"Common data shape: {data_loader.common_df_input.shape}")
        print(f"Found {len(data_loader.rdd_cols)} RDD columns")
        
        if data_loader.missing_in_output:
            print(f"Missing in output: {len(data_loader.missing_in_output)} indices")
        if data_loader.missing_in_input:
            print(f"Missing in input: {len(data_loader.missing_in_input)} indices")
    else:
        print("Data preparation failed.")