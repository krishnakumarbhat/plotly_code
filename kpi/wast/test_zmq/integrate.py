import subprocess
import sys
import time


def _compile_proto() -> None:
	try:
		import grpc_tools.protoc as protoc  # type: ignore
	except Exception:
		# Fallback to module invocation if available
		cmd = [sys.executable, "-m", "grpc_tools.protoc", "-I.", "--python_out=.", "calc.proto"]
		subprocess.check_call(cmd)
		return

	args = [
		"protoc",
		"-I.",
		"--python_out=.",
		"calc.proto",
	]
	# grpc_tools.protoc.main expects a list like sys.argv
	ret = protoc.main(args)
	if ret != 0:
		raise RuntimeError(f"protoc failed with exit code {ret}")


def main() -> None:
	addr = "tcp://127.0.0.1:5555"
	_compile_proto()
	server_cmd = [sys.executable, "server_type.py", addr]
	print(f"[integrate] starting server: {' '.join(server_cmd)}")
	server_proc = subprocess.Popen(server_cmd)
	try:
		# Give server a moment to bind
		time.sleep(0.5)
		client_cmd = [sys.executable, "clinettype.py", addr]
		print(f"[integrate] running client: {' '.join(client_cmd)}")
		subprocess.check_call(client_cmd)
	finally:
		print("[integrate] terminating server")
		server_proc.terminate()
		try:
			server_proc.wait(timeout=2)
		except Exception:  # noqa: BLE001
			server_proc.kill()


if __name__ == "__main__":
	main()


