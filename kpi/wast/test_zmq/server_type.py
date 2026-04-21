import sys

import zmq

import calc_pb2 as messages_pb2


def run_server(bind_address: str = "tcp://127.0.0.1:5555") -> None:
	context = zmq.Context.instance()
	socket = context.socket(zmq.REP)
	socket.bind(bind_address)
	result_value = None

	try:
		while True:
			data = socket.recv()
			request = messages_pb2.CalcRequest()
			try:
				request.ParseFromString(data)
			except Exception as parse_exc:  # noqa: BLE001
				response = messages_pb2.CalcResponse(result=0, error=f"parse error: {parse_exc}")
				socket.send(response.SerializeToString())
				continue
			
			try:
				if request.op == messages_pb2.ADD:
					result_value = int(request.a) + int(request.b)
					response = messages_pb2.CalcResponse(result=result_value, error="")
				elif request.op == messages_pb2.MUL:
					result_value = int(request.a) * int(request.b)
					response = messages_pb2.CalcResponse(result=result_value, error="")
				elif request.op == messages_pb2.PING:
					response = messages_pb2.CalcResponse(result=int(result_value or 0), error="")
				else:
					raise ValueError(f"unknown operation: {request.op}")
			except Exception as exc:  # noqa: BLE001
				response = messages_pb2.CalcResponse(result=0, error=str(exc))

			socket.send(response.SerializeToString())
	finally:
		socket.close(0)


if __name__ == "__main__":
	addr = sys.argv[1] if len(sys.argv) > 1 else "tcp://127.0.0.1:5555"
	print(f"[server] binding on {addr}")
	run_server(addr)


