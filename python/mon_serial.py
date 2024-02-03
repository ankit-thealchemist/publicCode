from serial.tools import miniterm

def start_miniterm(port='/dev/ttyACM0', baudrate=115200):
    miniterm.Miniterm(port=port, baudrate=baudrate).mainloop()

if __name__ == "__main__":
    start_miniterm()
