import pandas as pd
import matplotlib.pyplot as plt
import os
import re
import sys
import numpy as np

def main():
    # Get the directory path from user input or use current directory
    if len(sys.argv) > 1:
        csv_dir = sys.argv[1]
    else:
        csv_dir = os.getcwd()
    
    print(f"Looking for CSV files in: {csv_dir}")
    
    # Find all CSV files in the directory
    csv_files = [f for f in os.listdir(csv_dir) if f.endswith('.csv')]
    
    if not csv_files:
        print("No CSV files found in the specified directory.")
        return
    
    print(f"Found {len(csv_files)} CSV files: {csv_files}")
    
    # Identify input and output files based on '_r{number}' pattern
    input_file = None
    output_file = None
    
    for file in csv_files:
        if re.search(r'_r\d+', file):
            output_file = os.path.join(csv_dir, file)
        else:
            input_file = os.path.join(csv_dir, file)
    
    if not input_file or not output_file:
        print("Could not identify input and output files. Please ensure one file has '_r{number}' pattern.")
        return
    
    print(f"Input file: {os.path.basename(input_file)}")
    print(f"Output file: {os.path.basename(output_file)}")
    
    try:
        # Read the CSV files
        print("Reading input file...")
        input_df = pd.read_csv(input_file)
        print("Reading output file...")
        output_df = pd.read_csv(output_file)
        
        # Check if scan_index column exists (might be scanindex or scan_index)
        scan_index_col = None
        for col in ['scan_index', 'scanindex', 'SCAN_INDEX', 'ScanIndex']:
            if col in input_df.columns and col in output_df.columns:
                scan_index_col = col
                break
        
        if not scan_index_col:
            print("Could not find scan_index column in both files.")
            print(f"Input columns: {input_df.columns.tolist()[:10]}...")
            print(f"Output columns: {output_df.columns.tolist()[:10]}...")
            return
        
        # Check if scan_index values match
        input_indices = set(input_df[scan_index_col])
        output_indices = set(output_df[scan_index_col])
        
        missing_in_output = input_indices - output_indices
        missing_in_input = output_indices - input_indices
        
        if missing_in_output:
            print(f"Found {len(missing_in_output)} scan indices in input but missing in output.")
            print(f"Examples: {sorted(list(missing_in_output))[:10]}")
            
            # Save missing indices to a file
            missing_output_df = pd.DataFrame({scan_index_col: sorted(list(missing_in_output))})
            missing_output_file = os.path.join(csv_dir, 'missing_in_output.csv')
            missing_output_df.to_csv(missing_output_file, index=False)
            print(f"Missing indices saved to {missing_output_file}")
        
        if missing_in_input:
            print(f"Found {len(missing_in_input)} scan indices in output but missing in input.")
            print(f"Examples: {sorted(list(missing_in_input))[:10]}")
            
            # Save missing indices to a file
            missing_input_df = pd.DataFrame({scan_index_col: sorted(list(missing_in_input))})
            missing_input_file = os.path.join(csv_dir, 'missing_in_input.csv')
            missing_input_df.to_csv(missing_input_file, index=False)
            print(f"Missing indices saved to {missing_input_file}")
        
        # Find rdd1_dindx columns
        rdd_cols = [col for col in input_df.columns if col.startswith('rdd1_dindx_')]
        
        if not rdd_cols:
            print("Could not find any columns starting with 'rdd1_dindx_'.")
            return
        
        print(f"Found {len(rdd_cols)} rdd1_dindx columns. Examples: {rdd_cols[:5]}")
        
        # Get common scan indices for comparison
        common_indices = sorted(list(input_indices.intersection(output_indices)))
        common_df_input = input_df[input_df[scan_index_col].isin(common_indices)].sort_values(by=scan_index_col)
        common_df_output = output_df[output_df[scan_index_col].isin(common_indices)].sort_values(by=scan_index_col)
        
        # Create a directory for plots
        plots_dir = os.path.join(csv_dir, 'rdd_plots')
        os.makedirs(plots_dir, exist_ok=True)
        
        # Plot overview of all rdd1_dindx columns
        plt.figure(figsize=(15, 10))
        
        # Create a heatmap-like visualization for all rdd1_dindx columns
        rdd_data_input = common_df_input[rdd_cols].values
        rdd_data_output = common_df_output[rdd_cols].values
        
        # Calculate differences between input and output
        diff_data = np.abs(rdd_data_input - rdd_data_output)
        
        # Plot heatmap of differences
        plt.subplot(2, 1, 1)
        plt.imshow(diff_data.T, aspect='auto', cmap='hot', interpolation='nearest')
        plt.colorbar(label='Absolute Difference')
        plt.title('Differences between Input and Output RDD1_DINDX Values')
        plt.xlabel('Scan Index Position')
        plt.ylabel('RDD1_DINDX Column Index')
        
        # Plot a few selected columns for detailed view using scatter plots
        selected_cols = rdd_cols[:5]  # First 5 columns
        
        plt.subplot(2, 1, 2)
        for i, col in enumerate(selected_cols):
            plt.scatter(common_df_input[scan_index_col], common_df_input[col], s=5, alpha=0.7, label=f'Input {col}')
            plt.scatter(common_df_output[scan_index_col], common_df_output[col], s=5, alpha=0.7, marker='x', label=f'Output {col}')
        
        plt.legend()
        plt.title('Comparison of Selected RDD1_DINDX Columns (Scatter Plot)')
        plt.xlabel('Scan Index')
        plt.ylabel('Value')
        plt.grid(True)
        
        plt.tight_layout()
        overview_plot = os.path.join(plots_dir, 'rdd_overview_scatter.png')
        plt.savefig(overview_plot)
        print(f"Overview scatter plot saved as {overview_plot}")
        
        # Create individual plots for a sample of columns
        sample_size = min(10, len(rdd_cols))  # Plot up to 10 columns
        sample_cols = rdd_cols[:sample_size]
        
        for col in sample_cols:
            plt.figure(figsize=(12, 6))
            
            # Plot the values as scatter plots
            plt.subplot(2, 1, 1)
            plt.scatter(common_df_input[scan_index_col], common_df_input[col], s=5, color='blue', label='Input')
            plt.scatter(common_df_output[scan_index_col], common_df_output[col], s=5, color='red', label='Output', marker='x')
            plt.legend()
            plt.title(f'Comparison of {col} (Scatter Plot)')
            plt.grid(True)
            
            # Plot the differences as scatter plot, focusing on differences > 0.01
            plt.subplot(2, 1, 2)
            diff = common_df_input[col].values - common_df_output[col].values
            
            # Filter for differences significant to 2 decimal places
            significant_diff_indices = np.where(np.abs(diff) >= 0.01)[0]
            
            if len(significant_diff_indices) > 0:
                plt.scatter(common_df_input[scan_index_col].iloc[significant_diff_indices], 
                            diff[significant_diff_indices], s=5, color='green')
                plt.title(f'Significant Differences (Input - Output) for {col} (Abs Diff >= 0.01)')
            else:
                plt.text(0.5, 0.5, 'No significant differences (Abs Diff >= 0.01)', 
                         horizontalalignment='center', verticalalignment='center', 
                         transform=plt.gca().transAxes, fontsize=12, color='gray')
                plt.title(f'Differences (Input - Output) for {col}')

            plt.axhline(y=0, color='k', linestyle='-', alpha=0.3)
            plt.xlabel('Scan Index')
            plt.ylabel('Difference')
            plt.grid(True)
            
            plt.tight_layout()
            col_plot = os.path.join(plots_dir, f'{col}_comparison_scatter.png')
            plt.savefig(col_plot)
            plt.close()
        
        print(f"Individual column scatter plots saved in {plots_dir}")
        
        # Calculate statistics on differences
        diff_stats = {}
        for col in rdd_cols:
            if col in common_df_input.columns and col in common_df_output.columns:
                diff = common_df_input[col].values - common_df_output[col].values
                diff_stats[col] = {
                    'mean_diff': np.mean(diff),
                    'max_diff': np.max(np.abs(diff)),
                    'std_diff': np.std(diff),
                    'num_nonzero_diff': np.count_nonzero(np.abs(diff) >= 0.01) # Count significant differences
                }
        
        # Find columns with significant differences
        significant_diffs = {col: stats for col, stats in diff_stats.items() 
                            if stats['num_nonzero_diff'] > 0}
        
        if significant_diffs:
            print(f"Found {len(significant_diffs)} columns with differences (>= 0.01) between input and output.")
            
            # Sort by maximum difference
            sorted_diffs = sorted(significant_diffs.items(), 
                                 key=lambda x: x[1]['max_diff'], 
                                 reverse=True)
            
            # Display top differences
            print("\nTop 5 columns with largest differences (Abs Diff >= 0.01):")
            for col, stats in sorted_diffs[:5]:
                print(f"{col}: Max Abs Diff = {stats['max_diff']:.6f}, Mean Diff = {stats['mean_diff']:.6f}, "  
                      f"Significant Diffs = {stats['num_nonzero_diff']}")
            
            # Save difference statistics to CSV
            diff_stats_df = pd.DataFrame({
                'column': list(diff_stats.keys()),
                'mean_diff': [stats['mean_diff'] for stats in diff_stats.values()],
                'max_diff': [stats['max_diff'] for stats in diff_stats.values()],
                'std_diff': [stats['std_diff'] for stats in diff_stats.values()],
                'num_significant_diff': [stats['num_nonzero_diff'] for stats in diff_stats.values()]
            })
            
            diff_stats_df = diff_stats_df.sort_values('max_diff', ascending=False)
            diff_stats_file = os.path.join(csv_dir, 'rdd_diff_statistics_significant.csv')
            diff_stats_df.to_csv(diff_stats_file, index=False)
            print(f"\nDifference statistics saved to {diff_stats_file}")
        else:
            print("No significant differences (Abs Diff >= 0.01) found between input and output RDD1_DINDX values.")
        
        # Show the main plot (overview plot)
        plt.show()
        
    except Exception as e:
        print(f"Error processing files: {e}")
        import traceback
        traceback.print_exc()

if __name__ == "__main__":
    main()