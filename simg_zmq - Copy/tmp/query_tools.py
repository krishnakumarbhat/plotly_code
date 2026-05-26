#!/usr/bin/env python3
import json, socket

s = socket.create_connection(('127.0.0.1', 9100), timeout=10)
s.sendall((json.dumps({'action': 'tools'}) + '\n').encode())
print(s.recv(65536).decode())
s.close()
