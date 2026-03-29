import sys
import serial
import pyqtgraph as pg
from PyQt6.QtWidgets import QApplication, QMainWindow, QVBoxLayout, QWidget
from PyQt6.QtCore import QTimer

class RealTimePlotter(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("ADS1115 Real-Time Voltage Plotter")
        self.resize(800, 600)

        # 1. Set up the GUI layout
        central_widget = QWidget()
        self.setCentralWidget(central_widget)
        layout = QVBoxLayout(central_widget)

        # 2. Set up the Plot
        self.plot_graph = pg.PlotWidget()
        layout.addWidget(self.plot_graph)
        self.plot_graph.setBackground('w') # White background
        self.plot_graph.setTitle("Channel 0 vs Channel 1", color="k", size="15pt")
        self.plot_graph.setLabel('left', 'Voltage (V)', color="k")
        self.plot_graph.setLabel('bottom', 'Time (Seconds)', color="k")
        self.plot_graph.addLegend()

        # Create two lines for the two channels
        pen0 = pg.mkPen(color=(255, 0, 0), width=2) # Red for Ch0
        pen1 = pg.mkPen(color=(0, 0, 255), width=2) # Blue for Ch1
        self.line0 = self.plot_graph.plot([], [], name="Channel 0", pen=pen0)
        self.line1 = self.plot_graph.plot([], [], name="Channel 1", pen=pen1)

        # 3. Data Storage
        self.time_data = []
        self.ch0_data = []
        self.ch1_data = []
        self.max_points = 1000 # How many points to show on screen at once

        # 4. Serial Port Setup
        # IMPORTANT: Change this to match your STM32's port (e.g., 'COM3' on Windows, '/dev/ttyACM0' on Mac/Linux)
        self.serial_port = 'COM5' 
        self.baud_rate = 115200
        
        try:
            self.ser = serial.Serial(self.serial_port, self.baud_rate, timeout=0.01)
            print(f"Connected to {self.serial_port} at {self.baud_rate} baud.")
        except Exception as e:
            print(f"ERROR: Could not open serial port {self.serial_port}. Is it correct? Is the Arduino Monitor closed?")
            self.ser = None

        # 5. Timer to update the plot rapidly
        self.timer = QTimer()
        self.timer.timeout.connect(self.update_plot)
        self.timer.start(10) # Triggers every 10 milliseconds

    def update_plot(self):
        if not self.ser or not self.ser.in_waiting:
            return

        try:
            # Read all available lines in the buffer
            while self.ser.in_waiting:
                line = self.ser.readline().decode('utf-8').strip()
                
                # We expect 5 values: ch0_raw, ch0_volts, ch1_raw, ch1_volts, timestamp
                parts = line.split(',')
                if len(parts) == 5:
                    ch0_v = float(parts[1])
                    ch1_v = float(parts[3])
                    time_s = float(parts[4])

                    self.time_data.append(time_s)
                    self.ch0_data.append(ch0_v)
                    self.ch1_data.append(ch1_v)

            # Trim the lists so the graph scrolls instead of squeezing infinitely
            if len(self.time_data) > self.max_points:
                self.time_data = self.time_data[-self.max_points:]
                self.ch0_data = self.ch0_data[-self.max_points:]
                self.ch1_data = self.ch1_data[-self.max_points:]

            # Update the lines on the graph
            self.line0.setData(self.time_data, self.ch0_data)
            self.line1.setData(self.time_data, self.ch1_data)

        except Exception as e:
            # Ignore occasional garbled serial data during startup
            pass 

if __name__ == '__main__':
    app = QApplication(sys.argv)
    main = RealTimePlotter()
    main.show()
    sys.exit(app.exec())