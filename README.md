# CAN Vehicle Message Simulator & Parser

This project implements a CAN message parser with DBC-style signal definitions, a simulator application, manually specified test cases, and a lightweight web dashboard for real-time visualization.
The goal is to demonstrate how raw CAN frames are decoded, validated, simulated, and exposed to higher-level applications.
---

## Features

- CAN frame parsing using DBC-like signal metadata
- Signal decoding with scaling and offset
- Range validation with warning generation
- Simulation mode for real-time CAN data generation
- Test mode with predefined parser test cases
- Lightweight HTTP web server (C-based) / Web dashboard for live vehicle parameters
- Warning indication for out-of-range values
---

## Supported CAN Signals

| Signal              | CAN ID | Range        | Unit |
|---------------------|--------|--------------|------|
| Motor RPM           | 0x101  | 0 – 10000    | rpm  |
| Vehicle Speed       | 0x102  | 0 – 120      | km/h |
| Battery SOC         | 0x103  | 0 – 100      | %    |
| Battery Voltage     | 0x104  | 0 – 100      | V    |
| Motor Temperature   | 0x105  | 0 – 150      | °C   |
---

## Modes of Operation

### 1. Test Mode
Runs predefined test cases to validate:
- Correct parsing
- Scaling accuracy
- Range validation
- Error handling for invalid CAN frames

**Output:**  
- Structured logs in the terminal (decoded values, warnings, errors)

### 2. Simulation Mode
Continuously generates CAN frames to simulate vehicle behavior.

- Motor RPM exceeds the maximum limit to trigger warnings
- Real-time decoded values are sent to the web dashboard
- Warnings are generated for out-of-range values
---

## How Data Flows

1. CAN frames are generated (simulation mode) or injected (test mode)
2. Frames are decoded by the parser using signal definitions
3. Decoded values are validated and stored in a shared data model
4. The web server exposes this data via a `/data` endpoint
5. The dashboard fetches data periodically and updates the UI
---

## Running the Project

1. Build using the command -> pio run
2. Run using the command -> pio run -t exec
3. Terminal will display : 
      Select Mode
      1 → Run Parser Test Cases
      2 → Run CAN Simulation
4. A Message logger with timestamps is also created to keep the logs.

### Learning Outcomes
1. CAN protocol fundamentals
2. DBC-style signal interpretation
3. Embedded C networking (HTTP server)
4. Backend–frontend data integration
5. Simulation-based validation of automotive signals
