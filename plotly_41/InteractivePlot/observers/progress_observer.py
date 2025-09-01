"""
Progress observers for Interactive Plot System.

This module provides classes for tracking and reporting progress.
"""
import sys
import time
import threading
from typing import List

from InteractivePlot.interfaces.base_interfaces import IProgressObserver, IProgressReporter


class ProgressReporter(IProgressReporter):
    """
    Progress reporter that notifies registered observers.
    
    This class implements the Observer pattern for progress reporting.
    """
    
    def __init__(self):
        """Initialize the progress reporter."""
        self.observers: List[IProgressObserver] = []
        self.progress = 0.0
        self.message = ""
    
    def register_observer(self, observer: IProgressObserver) -> None:
        """
        Register a progress observer.
        
        Args:
            observer: The progress observer to register
        """
        if observer not in self.observers:
            self.observers.append(observer)
    
    def unregister_observer(self, observer: IProgressObserver) -> None:
        """
        Unregister a progress observer.
        
        Args:
            observer: The progress observer to unregister
        """
        if observer in self.observers:
            self.observers.remove(observer)
    
    def notify_observers(self, progress: float, message: str) -> None:
        """
        Notify all registered observers of progress.
        
        Args:
            progress: Progress percentage (0-100)
            message: Progress message
        """
        self.progress = progress
        self.message = message
        
        for observer in self.observers:
            observer.update(progress, message)


class ConsoleProgressObserver(IProgressObserver):
    """
    Console-based progress observer that displays progress updates.
    """
    
    def __init__(self, update_interval: float = 0.5):
        """
        Initialize the console progress observer.
        
        Args:
            update_interval: Minimum interval between updates (in seconds)
        """
        self.last_update_time = 0
        self.update_interval = update_interval
    
    def update(self, progress: float, message: str) -> None:
        """
        Update progress information and display it.
        
        Args:
            progress: Progress percentage (0-100)
            message: Progress message
        """
        # Limit the update frequency
        current_time = time.time()
        if current_time - self.last_update_time < self.update_interval:
            return
            
        self.last_update_time = current_time
        
        # Clear the current line
        sys.stdout.write("\r" + " " * 80 + "\r")
        
        # Format and display the progress
        progress_bar = self._create_progress_bar(progress)
        sys.stdout.write(f"{progress_bar} {progress:.1f}% - {message}")
        sys.stdout.flush()
    
    def _create_progress_bar(self, progress: float, width: int = 20) -> str:
        """
        Create a text-based progress bar.
        
        Args:
            progress: Progress percentage (0-100)
            width: Width of the progress bar
            
        Returns:
            String representation of the progress bar
        """
        filled_width = int(width * progress / 100)
        bar = "█" * filled_width + "░" * (width - filled_width)
        return f"[{bar}]"


class ProgressAnimator:
    """
    Animated progress indicator for long-running operations.
    
    This class provides a spinning animation to indicate that processing is ongoing.
    """
    
    def __init__(self, progress_reporter: ProgressReporter):
        """
        Initialize the progress animator.
        
        Args:
            progress_reporter: The progress reporter to monitor
        """
        self.progress_reporter = progress_reporter
        self.running = False
        self.thread = None
        self.animation_chars = ["|", "/", "-", "\\"]
    
    def _animate(self, message: str):
        """
        Display an animation while processing is ongoing.
        
        Args:
            message: Message to display with the animation
        """
        i = 0
        while self.running:
            # Only show animation if progress is < 100%
            if self.progress_reporter.progress < 100:
                animation_char = self.animation_chars[i % len(self.animation_chars)]
                sys.stdout.write(f"\r{animation_char} {message}")
                sys.stdout.flush()
                i += 1
                time.sleep(0.1)
            else:
                time.sleep(0.1)
    
    def start(self, message: str = "Processing..."):
        """
        Start the animation.
        
        Args:
            message: Message to display with the animation
        """
        self.running = True
        self.thread = threading.Thread(target=self._animate, args=(message,))
        self.thread.daemon = True
        self.thread.start()
    
    def stop(self, final_message: str = "Completed"):
        """
        Stop the animation.
        
        Args:
            final_message: Final message to display
        """
        self.running = False
        if self.thread:
            self.thread.join(timeout=0.5)
            self.thread = None
            
        # Clear the line and display the final message
        sys.stdout.write("\r" + " " * 80 + "\r")
        sys.stdout.write(f"{final_message}\n")
        sys.stdout.flush() 