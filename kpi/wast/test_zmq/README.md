### Basic ZMQ + Protobuf Calculator (Add / Multiply)

This demo shows a minimal REQ/REP ZeroMQ service that accepts protobuf-encoded requests to perform addition or multiplication.

### Install

- Create/activate a virtual environment (optional but recommended)
- Install dependencies:
```bash
pip install -r requirements.txt
```

### Run (automatic)

- Start server and run client automatically (your preference: server launched via subprocess [[memory:6711369]]):
```bash
python integrate.py
```

### Run (manual)

- In one terminal:
```bash
python server_type.py
```
- In another terminal:
```bash
python clinettype.py
```

### Files
- `calc.proto`: Protobuf schema
- `calc_pb2.py`: Generated Python messages (created automatically by the launcher)
- `server_type.py`: ZeroMQ REP server
- `clinettype.py`: ZeroMQ REQ client
- `integrate.py`: Convenience launcher that spawns the server then runs the client

### Notes
- Default bind/connect address: `tcp://127.0.0.1:5555`
- If you have `protoc`, you could swap `messages_pb2.py` with generated output from a `.proto` file. This project uses a small hand-written module to avoid requiring codegen.
