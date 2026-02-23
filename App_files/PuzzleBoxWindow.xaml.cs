using System;
using System.IO.Ports;
using System.Windows;
using System.Collections.ObjectModel;
using System.Threading;

namespace PuzzleBoxApp
{
    public partial class PuzzleBoxWindow : Window
    {
        private SerialPort serialPort;
        private Dictionary<string, PuzzleInfo> beaconPuzzleMap;
        
        public class PuzzleInfo
        {
            public int Id { get; set; }
            public string Name { get; set; }
            public string Content { get; set; }
            public string Difficulty { get; set; }
        }
        
        public PuzzleBoxWindow()
        {
            InitializeComponent();
            InitializeBeaconMap();
            InitializeSerialPort();
            StartListening();
        }
        
        private void InitializeBeaconMap()
        {
            beaconPuzzleMap = new Dictionary<string, PuzzleInfo>
            {
                {
                    "beacon-uuid-1",
                    new PuzzleInfo
                    {
                        Id = 1,
                        Name = "Math Challenge",
                        Content = "Solve: 15 + 27 = ?",
                        Difficulty = "Easy"
                    }
                },
                {
                    "beacon-uuid-2",
                    new PuzzleInfo
                    {
                        Id = 2,
                        Name = "Logic Puzzle",
                        Content = "If A=1, B=2, C=3...\nWhat is Z?",
                        Difficulty = "Medium"
                    }
                }
            };
        }
        
        private void InitializeSerialPort()
        {
            serialPort = new SerialPort("COM3", 9600);
            serialPort.DataReceived += SerialPort_DataReceived;
        }
        
        private void StartListening()
        {
            try
            {
                serialPort.Open();
                serialPort.WriteLine("AT+DISC");
            }
            catch (Exception ex)
            {
                MessageBox.Show($"Serial Port Error: {ex.Message}");
            }
        }
        
        private void SerialPort_DataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            string data = serialPort.ReadLine();
            
            if (data.StartsWith("BEACON:"))
            {
                string beaconUuid = data.Substring(7).Trim();
                Dispatcher.Invoke(() => HandleBeaconDetected(beaconUuid));
            }
        }
        
        private void HandleBeaconDetected(string beaconUuid)
        {
            if (beaconPuzzleMap.TryGetValue(beaconUuid, out var puzzleInfo))
            {
                PuzzleNameLabel.Content = puzzleInfo.Name;
                PuzzleContentTextBox.Text = puzzleInfo.Content;
                DifficultyLabel.Content = $"Difficulty: {puzzleInfo.Difficulty}";
            }
        }
    }
}