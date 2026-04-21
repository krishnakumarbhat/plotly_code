import sys
import time
import zmq

import calc_pb2 as messages_pb2


def send_request(socket: "zmq.Socket", op: int, a: int, b: int) -> messages_pb2.CalcResponse:
	req = messages_pb2.CalcRequest(op=op, a=int(a), b=int(b))
	socket.send(req.SerializeToString())
	data = socket.recv()
	resp = messages_pb2.CalcResponse()
	resp.ParseFromString(data)
	return resp


def main(server_address: str = "tcp://127.0.0.1:5555") -> None:
	context = zmq.Context.instance()
	socket = context.socket(zmq.REQ)
	socket.connect(server_address)

	try:
		resp1 = send_request(socket, messages_pb2.ADD, 3, 5)
		# do some calculation/work here
		time.sleep(0.4)
		resp2 = send_request(socket, messages_pb2.PING, 0, 0)
		print(f"add(3,5) -> result={resp1.result} error='{resp1.error}'")
		print(f"ping -> last_result={resp2.result} error='{resp2.error}'")
	finally:
		socket.close(0)


if __name__ == "__main__":
	addr = sys.argv[1] if len(sys.argv) > 1 else "tcp://127.0.0.1:5555"
	print(f"[client] connecting to {addr}")
	main(addr)


