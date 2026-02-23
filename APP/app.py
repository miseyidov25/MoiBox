import tkinter as tk
from tkinter import ttk

class App:
    def __init__(self, root):
        self.root = root
        self.root.title("MoiBox")
        self.root.geometry("200x200")
        self.root.resizable(0,0)      # disable maximize/minimize buttons

        # make grid expand so frame/button centering works
        self.root.grid_columnconfigure(0, weight=1)
        self.root.grid_rowconfigure(0, weight=1)

        # Create frame
        frame = ttk.Frame(root, padding="20")
        frame.grid(row=0, column=0, sticky=(tk.W, tk.E, tk.N, tk.S))
        frame.grid_columnconfigure(0, weight=1)

        # center window after widgets are laid out
        self.root.update_idletasks()
        w = self.root.winfo_width()
        h = self.root.winfo_height()
        x = (self.root.winfo_screenwidth() // 2) - (w // 2)
        y = (self.root.winfo_screenheight() // 2) - (h // 2)
        self.root.geometry(f"{w}x{h}+{x}+{y}")
        
        # Connect button
        self.connect_btn = ttk.Button(
            frame, 
            text="Connect", 
            command=self.on_connect
        )
        self.connect_btn.grid(row=0, column=0, pady=10, sticky=(tk.W, tk.E))
        
        # Start button
        self.start_btn = ttk.Button(
            frame, 
            text="Start", 
            command=self.on_start
        )
        self.start_btn.grid(row=1, column=0, pady=10, sticky=(tk.W, tk.E))
        
        # Restart button
        self.restart_btn = ttk.Button(
            frame, 
            text="Restart", 
            command=self.on_restart
        )
        self.restart_btn.grid(row=2, column=0, pady=10, sticky=(tk.W, tk.E))
    
    def on_connect(self):
        print("Connect button clicked")
        # Add functionality here
    
    def on_start(self):
        print("Start button clicked")
        # Add functionality here
    
    def on_restart(self):
        print("Restart button clicked")
        # Add functionality here

if __name__ == "__main__":
    root = tk.Tk()
    app = App(root)
    root.mainloop()