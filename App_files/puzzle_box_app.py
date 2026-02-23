import tkinter as tk
from tkinter import ttk
import serial
import threading
import json
from bleak import BleakScanner
import asyncio

class PuzzleBoxApp:
    def __init__(self, root):
        self.root = root
        self.root.title("Puzzle Box - iBeacon Detector")
        self.root.geometry("800x600")
        
        self.current_puzzle = None
        self.detected_beacons = {}
        
        # Serial connection to FRDM-MCXA153
        self.ser = serial.Serial('COM3', 9600, timeout=1)
        
        # iBeacon to Puzzle mapping
        self.beacon_puzzle_map = {
            'beacon-uuid-1': {'puzzle': 1, 'name': 'Math Challenge', 'difficulty': 'Easy'},
            'beacon-uuid-2': {'puzzle': 2, 'name': 'Logic Puzzle', 'difficulty': 'Medium'},
            'beacon-uuid-3': {'puzzle': 3, 'name': 'Memory Game', 'difficulty': 'Hard'},
        }
        
        self.setup_ui()
        self.start_listening()
    
    def setup_ui(self):
        # Header
        header = ttk.Label(self.root, text="Puzzle Box - iBeacon Detector", 
                          font=("Arial", 18, "bold"))
        header.pack(pady=10)
        
        # Status frame
        status_frame = ttk.LabelFrame(self.root, text="Detected Beacons")
        status_frame.pack(padx=10, pady=10, fill=tk.BOTH, expand=False)
        
        self.beacon_text = tk.Text(status_frame, height=6, width=80)
        self.beacon_text.pack(padx=5, pady=5)
        
        # Puzzle frame
        puzzle_frame = ttk.LabelFrame(self.root, text="Current Puzzle")
        puzzle_frame.pack(padx=10, pady=10, fill=tk.BOTH, expand=True)
        
        self.puzzle_label = ttk.Label(puzzle_frame, text="No beacon detected", 
                                      font=("Arial", 14))
        self.puzzle_label.pack(pady=20)
        
        self.puzzle_content = tk.Text(puzzle_frame, height=15, width=80)
        self.puzzle_content.pack(padx=5, pady=5, fill=tk.BOTH, expand=True)
        
        # Control frame
        control_frame = ttk.Frame(self.root)
        control_frame.pack(pady=10)
        
        ttk.Button(control_frame, text="Start Scan", 
                  command=self.start_scan).pack(side=tk.LEFT, padx=5)
        ttk.Button(control_frame, text="Stop Scan", 
                  command=self.stop_scan).pack(side=tk.LEFT, padx=5)
    
    def start_listening(self):
        threading.Thread(target=self.listen_to_device, daemon=True).start()
    
    def listen_to_device(self):
        while True:
            try:
                if self.ser.in_waiting:
                    line = self.ser.readline().decode('utf-8').strip()
                    if line.startswith('BEACON:'):
                        beacon_uuid = line.split(':')[1]
                        self.handle_beacon_detected(beacon_uuid)
                    elif line.startswith('PUZZLE:'):
                        puzzle_id = int(line.split(':')[1])
                        self.display_puzzle(puzzle_id)
            except Exception as e:
                print(f"Error reading from device: {e}")
    
    def handle_beacon_detected(self, beacon_uuid):
        if beacon_uuid in self.beacon_puzzle_map:
            puzzle_info = self.beacon_puzzle_map[beacon_uuid]
            
            # Update UI
            self.detected_beacons[beacon_uuid] = puzzle_info
            self.update_beacon_display()
            self.display_puzzle(puzzle_info['puzzle'])
            
            # Send to microcontroller
            self.ser.write(f"BEACON_DETECTED:{beacon_uuid}\r\n".encode())
    
    def update_beacon_display(self):
        self.beacon_text.config(state=tk.NORMAL)
        self.beacon_text.delete(1.0, tk.END)
        
        for beacon_uuid, info in self.detected_beacons.items():
            self.beacon_text.insert(tk.END, 
                f"UUID: {beacon_uuid}\nPuzzle: {info['name']} ({info['difficulty']})\n\n")
        
        self.beacon_text.config(state=tk.DISABLED)
    
    def display_puzzle(self, puzzle_id):
        puzzles = {
            1: {
                'name': 'Math Challenge',
                'content': 'Solve: 15 + 27 = ?\nAnswer the question and press submit.'
            },
            2: {
                'name': 'Logic Puzzle',
                'content': 'If A=1, B=2, C=3...\nWhat is Z?\nThink logically!'
            },
            3: {
                'name': 'Memory Game',
                'content': 'Remember the sequence:\n1-3-5-7-9\nRepeat it back to continue.'
            }
        }
        
        if puzzle_id in puzzles:
            puzzle = puzzles[puzzle_id]
            self.puzzle_label.config(text=puzzle['name'])
            
            self.puzzle_content.config(state=tk.NORMAL)
            self.puzzle_content.delete(1.0, tk.END)
            self.puzzle_content.insert(tk.END, puzzle['content'])
            self.puzzle_content.config(state=tk.DISABLED)
            
            self.current_puzzle = puzzle_id
    
    def start_scan(self):
        self.ser.write(b"AT+DISC\r\n")
    
    def stop_scan(self):
        self.ser.write(b"AT+DISC\r\n")

if __name__ == '__main__':
    root = tk.Tk()
    app = PuzzleBoxApp(root)
    root.mainloop()