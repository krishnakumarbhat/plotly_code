#!/usr/bin/env python3
"""
Script to generate the DataPrep flow diagram using Graphviz.
"""

import subprocess
import sys
import os
from pathlib import Path

def generate_diagram():
    """Generate the DataPrep flow diagram from DOT file."""
    
    # Check if Graphviz is installed
    try:
        subprocess.run(['dot', '-V'], capture_output=True, check=True)
        print("✓ Graphviz is installed")
    except (subprocess.CalledProcessError, FileNotFoundError):
        print("❌ Graphviz is not installed or not in PATH")
        print("\nTo install Graphviz:")
        print("  Windows: Download from https://graphviz.org/download/")
        print("  macOS: brew install graphviz")
        print("  Ubuntu/Debian: sudo apt-get install graphviz")
        print("  CentOS/RHEL: sudo yum install graphviz")
        return False
    
    # Input and output file paths
    dot_file = Path("data_prep_flow_diagram.dot")
    output_file = Path("data_prep_flow_diagram.png")
    
    if not dot_file.exists():
        print(f"❌ DOT file not found: {dot_file}")
        return False
    
    try:
        # Generate PNG diagram
        cmd = [
            'dot', 
            '-Tpng', 
            '-o', str(output_file),
            str(dot_file)
        ]
        
        print(f"Generating diagram: {dot_file} → {output_file}")
        result = subprocess.run(cmd, capture_output=True, text=True, check=True)
        
        if output_file.exists():
            print(f"✓ Diagram generated successfully: {output_file}")
            print(f"  File size: {output_file.stat().st_size / 1024:.1f} KB")
            return True
        else:
            print("❌ Diagram file was not created")
            return False
            
    except subprocess.CalledProcessError as e:
        print(f"❌ Error generating diagram: {e}")
        if e.stderr:
            print(f"Error details: {e.stderr}")
        return False
    except Exception as e:
        print(f"❌ Unexpected error: {e}")
        return False

def generate_svg_diagram():
    """Generate SVG version of the diagram."""
    
    dot_file = Path("data_prep_flow_diagram.dot")
    output_file = Path("data_prep_flow_diagram.svg")
    
    if not dot_file.exists():
        print(f"❌ DOT file not found: {dot_file}")
        return False
    
    try:
        # Generate SVG diagram
        cmd = [
            'dot', 
            '-Tsvg', 
            '-o', str(output_file),
            str(dot_file)
        ]
        
        print(f"Generating SVG diagram: {dot_file} → {output_file}")
        result = subprocess.run(cmd, capture_output=True, text=True, check=True)
        
        if output_file.exists():
            print(f"✓ SVG diagram generated successfully: {output_file}")
            print(f"  File size: {output_file.stat().st_size / 1024:.1f} KB")
            return True
        else:
            print("❌ SVG diagram file was not created")
            return False
            
    except subprocess.CalledProcessError as e:
        print(f"❌ Error generating SVG diagram: {e}")
        if e.stderr:
            print(f"Error details: {e.stderr}")
        return False
    except Exception as e:
        print(f"❌ Unexpected error: {e}")
        return False

def main():
    """Main function to generate diagrams."""
    print("DataPrep Flow Diagram Generator")
    print("=" * 40)
    
    # Generate PNG diagram
    success_png = generate_diagram()
    
    # Generate SVG diagram
    success_svg = generate_svg_diagram()
    
    if success_png or success_svg:
        print("\n" + "=" * 40)
        print("Generated files:")
        if Path("data_prep_flow_diagram.png").exists():
            print("  📊 data_prep_flow_diagram.png")
        if Path("data_prep_flow_diagram.svg").exists():
            print("  📊 data_prep_flow_diagram.svg")
        print("\nYou can now view the diagram to understand the DataPrep flow!")
    else:
        print("\n❌ Failed to generate any diagrams")
        sys.exit(1)

if __name__ == "__main__":
    main() 