import os
import pandas as pd
import numpy as np
import plotly.express as px
import plotly.graph_objects as go
from plotly.subplots import make_subplots
from typing import List, Dict, Tuple, Optional, Any
from abc import ABC, abstractmethod
from csv_data_loader import DataPreparationFacade, DataPreparationFactory


class Visualizer(ABC):
    """Abstract base class for visualization strategies"""
    @abstractmethod
    def create_visualization(self, data: Any, output_path: str) -> None:
        """Create and save visualization"""
        pass


class DifferenceScatterVisualizer(Visualizer):
    """Visualizer for creating scatter plots of differences"""
    def __init__(self, min_diff_threshold: float = 0.01):
        self.min_diff_threshold = min_diff_threshold
    
    def create_visualization(self, data: Dict, output_path: str) -> None:
        """Create scatter plots for significant differences"""
        input_df = data['input_df']
        output_df = data['output_df']
        scan_index_col = data['scan_index_col']
        rdd_cols = data['rdd_cols']
        
        # Create output directory if it doesn't exist
        os.makedirs(output_path, exist_ok=True)
        
        # Create a summary plot with all significant differences
        self._create_summary_plot(input_df, output_df, scan_index_col, rdd_cols, output_path)
        
        # Create individual plots for a sample of columns
        self._create_individual_plots(input_df, output_df, scan_index_col, rdd_cols, output_path)
    
    def _create_summary_plot(self, input_df: pd.DataFrame, output_df: pd.DataFrame, 
                            scan_index_col: str, rdd_cols: List[str], output_path: str) -> None:
        """Create a summary plot showing all significant differences"""
        # Calculate differences and find significant ones
        all_significant_diffs = []
        
        for col in rdd_cols:
            diffs = output_df[col].values - input_df[col].values
            significant_indices = np.where(np.abs(diffs) >= self.min_diff_threshold)[0]
            
            if len(significant_indices) > 0:
                for idx in significant_indices:
                    all_significant_diffs.append({
                        scan_index_col: input_df[scan_index_col].iloc[idx],
                        'column': col,
                        'input_value': input_df[col].iloc[idx],
                        'output_value': output_df[col].iloc[idx],
                        'difference': diffs[idx]
                    })
        
        if not all_significant_diffs:
            print("No significant differences found.")
            return
        
        # Create DataFrame from all significant differences
        diff_df = pd.DataFrame(all_significant_diffs)
        
        # Create scatter plot
        fig = px.scatter(diff_df, x=scan_index_col, y='difference', color='column',
                        hover_data=['input_value', 'output_value'],
                        title=f'Significant Differences (≥{self.min_diff_threshold}) Across All RDD Columns',
                        labels={'difference': 'Difference (Output - Input)'},
                        height=800)
        
        fig.update_layout(
            xaxis_title=scan_index_col,
            yaxis_title='Difference (Output - Input)',
            legend_title='RDD Column'
        )
        
        # Save the plot
        summary_path = os.path.join(output_path, 'summary_differences.html')
        fig.write_html(summary_path)
        print(f"Summary plot saved to {summary_path}")
    
    def _create_individual_plots(self, input_df: pd.DataFrame, output_df: pd.DataFrame, 
                                scan_index_col: str, rdd_cols: List[str], output_path: str) -> None:
        """Create individual plots for a sample of columns"""
        # Sample columns for individual plots (max 10)
        sample_size = min(10, len(rdd_cols))
        sample_cols = rdd_cols[:sample_size]
        
        for col in sample_cols:
            # Create a subplot with 2 rows
            fig = make_subplots(rows=2, cols=1, 
                                subplot_titles=[f'{col} Values', f'{col} Differences'],
                                vertical_spacing=0.15)
            
            # Add traces for input and output values
            fig.add_trace(
                go.Scatter(x=input_df[scan_index_col], y=input_df[col], mode='markers', name='Input',
                        marker=dict(color='blue', size=8)),
                row=1, col=1
            )
            
            fig.add_trace(
                go.Scatter(x=output_df[scan_index_col], y=output_df[col], mode='markers', name='Output',
                        marker=dict(color='red', size=8)),
                row=1, col=1
            )
            
            # Calculate differences
            diffs = output_df[col].values - input_df[col].values
            
            # Add trace for differences
            fig.add_trace(
                go.Scatter(x=input_df[scan_index_col], y=diffs, mode='markers',
                        marker=dict(
                            color=diffs,
                            colorscale='RdBu',
                            cmin=-max(abs(diffs)) if len(diffs) > 0 else -1,
                            cmax=max(abs(diffs)) if len(diffs) > 0 else 1,
                            colorbar=dict(title='Difference'),
                            size=10
                        )),
                row=2, col=1
            )
            
            # Update layout
            fig.update_layout(
                title_text=f'Comparison for {col}',
                height=800,
                showlegend=True
            )
            
            fig.update_xaxes(title_text=scan_index_col, row=2, col=1)
            fig.update_yaxes(title_text='Difference (Output - Input)', row=2, col=1)
            
            # Save the plot
            col_path = os.path.join(output_path, f'{col}_comparison.html')
            fig.write_html(col_path)
            print(f"Individual plot for {col} saved to {col_path}")


class DifferenceAnalyzer:
    """Analyzer for calculating and reporting differences"""
    def __init__(self, min_diff_threshold: float = 0.01):
        self.min_diff_threshold = min_diff_threshold
    
    def analyze_differences(self, input_df: pd.DataFrame, output_df: pd.DataFrame, 
                           rdd_cols: List[str]) -> pd.DataFrame:
        """Analyze differences between input and output dataframes"""
        results = []
        
        for col in rdd_cols:
            diffs = output_df[col].values - input_df[col].values
            significant_diffs = diffs[np.abs(diffs) >= self.min_diff_threshold]
            
            if len(significant_diffs) > 0:
                results.append({
                    'column': col,
                    'mean_diff': np.mean(significant_diffs),
                    'max_diff': np.max(np.abs(significant_diffs)),
                    'std_diff': np.std(significant_diffs),
                    'significant_count': len(significant_diffs),
                    'total_count': len(diffs),
                    'significant_percentage': (len(significant_diffs) / len(diffs)) * 100
                })
        
        # Create DataFrame from results
        if results:
            results_df = pd.DataFrame(results)
            results_df = results_df.sort_values(by='significant_count', ascending=False)
            return results_df
        else:
            return pd.DataFrame(columns=['column', 'mean_diff', 'max_diff', 'std_diff', 
                                        'significant_count', 'total_count', 'significant_percentage'])


class VisualizationFacade:
    """Facade pattern to simplify the visualization and analysis process"""
    def __init__(self, min_diff_threshold: float = 0.01, strategy_type: str = 'auto', json_file: str = 'input.json'):
        self.min_diff_threshold = min_diff_threshold
        self.visualizer = DifferenceScatterVisualizer(min_diff_threshold)
        self.analyzer = DifferenceAnalyzer(min_diff_threshold)
        self.strategy_type = strategy_type
        self.json_file = json_file
    
    def process_and_visualize(self, directory: str = None) -> None:
        """Process data and create visualizations"""
        # Prepare data using the factory pattern
        factory = DataPreparationFactory()
        data_facade = factory.create_facade(self.strategy_type, self.json_file)
        data_loader = data_facade.prepare_data(directory)
        
        if not data_loader:
            print("Data preparation failed. Cannot proceed with visualization.")
            return
        
        # Create output directory for plots
        plots_dir = os.path.join(os.getcwd(), 'rdd_plots')
        os.makedirs(plots_dir, exist_ok=True)
        
        # Analyze differences
        print("\nAnalyzing differences...")
        diff_stats = self.analyzer.analyze_differences(
            data_loader.common_df_input, 
            data_loader.common_df_output, 
            data_loader.rdd_cols
        )
        
        # Save difference statistics
        if not diff_stats.empty:
            stats_file = os.path.join(os.getcwd(), 'rdd_diff_statistics_significant.csv')
            diff_stats.to_csv(stats_file, index=False)
            print(f"Difference statistics saved to {stats_file}")
            print(f"Found {len(diff_stats)} columns with significant differences (≥{self.min_diff_threshold})")
            print(f"Top 5 columns with most significant differences:")
            print(diff_stats.head(5)[["column", "significant_count", "significant_percentage", "max_diff"]])
        else:
            print(f"No significant differences (≥{self.min_diff_threshold}) found in any column.")
        
        # Create visualizations
        print("\nCreating visualizations...")
        data = {
            'input_df': data_loader.common_df_input,
            'output_df': data_loader.common_df_output,
            'scan_index_col': data_loader.scan_index_col,
            'rdd_cols': data_loader.rdd_cols
        }
        self.visualizer.create_visualization(data, plots_dir)
        
        print(f"\nVisualization complete. All plots saved to {plots_dir}")


# Example usage when run directly
if __name__ == "__main__":
    import argparse
    
    # Set up command line argument parsing
    parser = argparse.ArgumentParser(description='CSV Analyzer for RDD Data')
    parser.add_argument('--dir', type=str, help='Directory containing CSV files', default=None)
    parser.add_argument('--threshold', type=float, help='Minimum difference threshold', default=0.01)
    parser.add_argument('--strategy', type=str, choices=['auto', 'json', 'pattern'], 
                        help='File identification strategy', default='auto')
    parser.add_argument('--json', type=str, help='JSON configuration file name', default='input.json')
    
    args = parser.parse_args()
    
    print(f"\nCSV Analyzer for RDD Data")
    print(f"=========================")
    print(f"Strategy: {args.strategy}")
    print(f"Threshold: {args.threshold}")
    if args.strategy in ['auto', 'json']:
        print(f"JSON Config: {args.json}")
    print(f"Directory: {args.dir if args.dir else 'Current directory'}")
    print(f"=========================")
    
    # Create and run the visualization facade with the specified parameters
    viz_facade = VisualizationFacade(
        min_diff_threshold=args.threshold,
        strategy_type=args.strategy,
        json_file=args.json
    )
    
    viz_facade.process_and_visualize(args.dir)