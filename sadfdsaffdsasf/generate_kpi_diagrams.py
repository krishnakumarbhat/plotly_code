#!/usr/bin/env python3
"""
Generate KPI diagrams from Graphviz DOT files
"""

import subprocess
import os
from pathlib import Path

def generate_diagram(dot_file, output_format='png'):
    """Generate diagram from DOT file using Graphviz"""
    dot_path = Path(dot_file)
    if not dot_path.exists():
        print(f"Error: {dot_file} not found")
        return False
    
    output_file = dot_path.with_suffix(f'.{output_format}')
    
    try:
        # Run graphviz to generate the diagram
        result = subprocess.run([
            'dot', f'-T{output_format}', str(dot_path), '-o', str(output_file)
        ], capture_output=True, text=True, check=True)
        
        print(f"Generated: {output_file}")
        return True
        
    except subprocess.CalledProcessError as e:
        print(f"Error generating {output_file}: {e}")
        print(f"stderr: {e.stderr}")
        return False
    except FileNotFoundError:
        print("Error: Graphviz 'dot' command not found. Please install Graphviz.")
        return False

def main():
    """Generate all KPI diagrams"""
    base_dir = Path(__file__).parent
    
    diagrams = [
        'kpi_data_flow_diagram.dot',
        'kpi_parser_loop_diagram.dot', 
        'kpi_state_transitions_diagram.dot'
    ]
    
    print("Generating KPI diagrams...")
    
    for diagram in diagrams:
        dot_file = base_dir / diagram
        success = generate_diagram(dot_file, 'png')
        if success:
            # Also generate SVG for scalability
            generate_diagram(dot_file, 'svg')
    
    print("\nDiagram generation complete!")
    print("Files generated:")
    for diagram in diagrams:
        base_name = Path(diagram).stem
        print(f"  - {base_name}.png")
        print(f"  - {base_name}.svg")

if __name__ == "__main__":
    main()
