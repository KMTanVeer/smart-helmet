"""
Crash Data Visualization Script
================================

This script reads crash data from the Smart Helmet CSV file
and generates various graphs for analysis.

Requirements:
    pip install pandas matplotlib folium

Usage:
    1. Download crashes.csv from ESP32 SPIFFS
    2. Place in same directory as this script
    3. Run: python visualize_crashes.py

Output:
    - crash_acceleration.png - Acceleration over time
    - crash_gyro.png - Gyroscope over time  
    - crash_battery.png - Battery levels at crashes
    - crash_map.html - Interactive map of crash locations
    - crash_combined.png - Combined analysis dashboard
"""

import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
from datetime import datetime, timedelta

def load_crash_data(filename='crashes.csv'):
    """Load crash data from CSV file"""
    try:
        df = pd.read_csv(filename)
        print(f"✅ Loaded {len(df)} crash records")
        return df
    except FileNotFoundError:
        print(f"❌ Error: {filename} not found")
        print("Please download crashes.csv from ESP32 SPIFFS first")
        return None
    except Exception as e:
        print(f"❌ Error loading file: {e}")
        return None

def plot_acceleration(df, output='crash_acceleration.png'):
    """Plot acceleration magnitude over crash events"""
    plt.figure(figsize=(12, 6))
    plt.plot(range(len(df)), df['AccMag'], marker='o', linestyle='-', color='red', linewidth=2, markersize=8)
    plt.axhline(y=2.5, color='orange', linestyle='--', label='Detection Threshold (2.5g)')
    plt.xlabel('Crash Event Number', fontsize=12)
    plt.ylabel('Acceleration Magnitude (g)', fontsize=12)
    plt.title('Crash Detection - Acceleration Analysis', fontsize=14, fontweight='bold')
    plt.grid(True, alpha=0.3)
    plt.legend()
    plt.tight_layout()
    plt.savefig(output, dpi=300)
    print(f"✅ Saved: {output}")
    plt.close()

def plot_gyroscope(df, output='crash_gyro.png'):
    """Plot gyroscope magnitude over crash events"""
    plt.figure(figsize=(12, 6))
    plt.plot(range(len(df)), df['GyroMag'], marker='s', linestyle='-', color='blue', linewidth=2, markersize=8)
    plt.axhline(y=120.0, color='cyan', linestyle='--', label='Detection Threshold (120°/s)')
    plt.xlabel('Crash Event Number', fontsize=12)
    plt.ylabel('Gyroscope Magnitude (°/s)', fontsize=12)
    plt.title('Crash Detection - Rotation Analysis', fontsize=14, fontweight='bold')
    plt.grid(True, alpha=0.3)
    plt.legend()
    plt.tight_layout()
    plt.savefig(output, dpi=300)
    print(f"✅ Saved: {output}")
    plt.close()

def plot_battery(df, output='crash_battery.png'):
    """Plot battery levels at crash times"""
    plt.figure(figsize=(12, 6))
    colors = ['green' if x > 50 else 'orange' if x > 20 else 'red' for x in df['Battery%']]
    plt.bar(range(len(df)), df['Battery%'], color=colors, alpha=0.7)
    plt.axhline(y=20, color='red', linestyle='--', label='Low Battery (20%)', alpha=0.5)
    plt.xlabel('Crash Event Number', fontsize=12)
    plt.ylabel('Battery Level (%)', fontsize=12)
    plt.title('Battery Status at Crash Detection', fontsize=14, fontweight='bold')
    plt.grid(True, alpha=0.3, axis='y')
    plt.legend()
    plt.ylim(0, 105)
    plt.tight_layout()
    plt.savefig(output, dpi=300)
    print(f"✅ Saved: {output}")
    plt.close()

def plot_combined_dashboard(df, output='crash_combined.png'):
    """Create a combined dashboard with multiple plots"""
    fig, axes = plt.subplots(2, 2, figsize=(16, 12))
    fig.suptitle('Smart Helmet - Crash Analysis Dashboard', fontsize=16, fontweight='bold')
    
    # Plot 1: Acceleration
    axes[0, 0].plot(range(len(df)), df['AccMag'], marker='o', color='red', linewidth=2)
    axes[0, 0].axhline(y=2.5, color='orange', linestyle='--', alpha=0.7)
    axes[0, 0].set_xlabel('Crash Event')
    axes[0, 0].set_ylabel('Acceleration (g)')
    axes[0, 0].set_title('Acceleration Magnitude')
    axes[0, 0].grid(True, alpha=0.3)
    
    # Plot 2: Gyroscope
    axes[0, 1].plot(range(len(df)), df['GyroMag'], marker='s', color='blue', linewidth=2)
    axes[0, 1].axhline(y=120.0, color='cyan', linestyle='--', alpha=0.7)
    axes[0, 1].set_xlabel('Crash Event')
    axes[0, 1].set_ylabel('Rotation (°/s)')
    axes[0, 1].set_title('Gyroscope Magnitude')
    axes[0, 1].grid(True, alpha=0.3)
    
    # Plot 3: Battery
    colors = ['green' if x > 50 else 'orange' if x > 20 else 'red' for x in df['Battery%']]
    axes[1, 0].bar(range(len(df)), df['Battery%'], color=colors, alpha=0.7)
    axes[1, 0].set_xlabel('Crash Event')
    axes[1, 0].set_ylabel('Battery (%)')
    axes[1, 0].set_title('Battery Level at Crash')
    axes[1, 0].grid(True, alpha=0.3, axis='y')
    
    # Plot 4: Statistics
    axes[1, 1].axis('off')
    stats_text = f"""
    Crash Statistics
    ================
    
    Total Crashes: {len(df)}
    
    Acceleration:
      • Average: {df['AccMag'].mean():.2f}g
      • Maximum: {df['AccMag'].max():.2f}g
      • Minimum: {df['AccMag'].min():.2f}g
    
    Gyroscope:
      • Average: {df['GyroMag'].mean():.1f}°/s
      • Maximum: {df['GyroMag'].max():.1f}°/s
      • Minimum: {df['GyroMag'].min():.1f}°/s
    
    Battery:
      • Average: {df['Battery%'].mean():.1f}%
      • Lowest: {df['Battery%'].min():.0f}%
      • Highest: {df['Battery%'].max():.0f}%
    """
    axes[1, 1].text(0.1, 0.5, stats_text, fontsize=11, verticalalignment='center', 
                    family='monospace', bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.3))
    
    plt.tight_layout()
    plt.savefig(output, dpi=300)
    print(f"✅ Saved: {output}")
    plt.close()

def create_map(df, output='crash_map.html'):
    """Create an interactive map of crash locations"""
    try:
        import folium
    except ImportError:
        print("⚠️  Warning: folium not installed")
        print("Install with: pip install folium")
        return
    
    # Calculate center of map
    center_lat = df['Latitude'].mean()
    center_lon = df['Longitude'].mean()
    
    # Create map
    crash_map = folium.Map(location=[center_lat, center_lon], zoom_start=13)
    
    # Add markers for each crash
    for idx, row in df.iterrows():
        popup_text = f"""
        <b>Crash #{idx + 1}</b><br>
        Time: {row['Timestamp']}ms<br>
        Acceleration: {row['AccMag']:.2f}g<br>
        Gyroscope: {row['GyroMag']:.1f}°/s<br>
        Battery: {row['Battery%']}%<br>
        <a href="https://maps.google.com/?q={row['Latitude']},{row['Longitude']}" target="_blank">Open in Google Maps</a>
        """
        
        folium.Marker(
            location=[row['Latitude'], row['Longitude']],
            popup=folium.Popup(popup_text, max_width=250),
            icon=folium.Icon(color='red', icon='info-sign'),
            tooltip=f"Crash #{idx + 1}"
        ).add_to(crash_map)
    
    # Save map
    crash_map.save(output)
    print(f"✅ Saved: {output}")
    print(f"   Open {output} in a web browser to view interactive map")

def print_summary(df):
    """Print summary statistics"""
    print("\n" + "="*50)
    print("CRASH DATA SUMMARY")
    print("="*50)
    print(f"Total Crashes Recorded: {len(df)}")
    print(f"\nAcceleration Statistics:")
    print(f"  Average: {df['AccMag'].mean():.2f}g")
    print(f"  Max: {df['AccMag'].max():.2f}g")
    print(f"  Min: {df['AccMag'].min():.2f}g")
    print(f"\nGyroscope Statistics:")
    print(f"  Average: {df['GyroMag'].mean():.1f}°/s")
    print(f"  Max: {df['GyroMag'].max():.1f}°/s")
    print(f"  Min: {df['GyroMag'].min():.1f}°/s")
    print(f"\nBattery Statistics:")
    print(f"  Average: {df['Battery%'].mean():.1f}%")
    print(f"  Lowest: {df['Battery%'].min():.0f}%")
    print(f"  Highest: {df['Battery%'].max():.0f}%")
    print(f"\nLocation Range:")
    print(f"  Latitude: {df['Latitude'].min():.6f} to {df['Latitude'].max():.6f}")
    print(f"  Longitude: {df['Longitude'].min():.6f} to {df['Longitude'].max():.6f}")
    print("="*50 + "\n")

def main():
    """Main function"""
    print("\n" + "="*50)
    print("Smart Helmet - Crash Data Visualization")
    print("="*50 + "\n")
    
    # Load data
    df = load_crash_data('crashes.csv')
    if df is None or len(df) == 0:
        print("❌ No data to visualize")
        return
    
    # Print summary
    print_summary(df)
    
    # Generate plots
    print("Generating visualizations...\n")
    plot_acceleration(df)
    plot_gyroscope(df)
    plot_battery(df)
    plot_combined_dashboard(df)
    create_map(df)
    
    print("\n✅ All visualizations generated successfully!")
    print("\nGenerated files:")
    print("  • crash_acceleration.png")
    print("  • crash_gyro.png")
    print("  • crash_battery.png")
    print("  • crash_combined.png")
    print("  • crash_map.html")

if __name__ == "__main__":
    main()
