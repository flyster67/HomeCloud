#!/usr/bin/env python3
"""homecloud.py — HomeCloud socket manager

Handles TCP networking and delegates file I/O & compression
to the C backend process via pipes (stdin/stdout).

Usage:
    python homecloud.py server [share_dir] [port]
    python homecloud.py client <server_ip> [port]
"""

import sys
import os
import socket
import struct
import subprocess

DEFAULT_PORT = 7878

# Protocol opcodes
OP_REQUEST_FILE = 1
OP_SEND_FILE    = 2
OP_DATA         = 10
OP_ERROR        = 20
OP_OK           = 21


# -----------------------------------------------------------------------------
# C Backend Process Interface
# -----------------------------------------------------------------------------

class Backend:
    def __init__(self):
        script_dir = os.path.dirname(os.path.abspath(__file__))
        exe = os.path.join(script_dir, "backend.exe" if os.name == "nt" else "backend")
        if not os.path.exists(exe):
            print(f"[!] Backend executable not found at {exe}")
            print("    Build with: gcc backend.c -o backend.exe -lz")
            sys.exit(1)

        self.proc = subprocess.Popen(
            [exe],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=None,
        )

    def _send_cmd(self, cmd: str):
        """Send command line to backend."""
        self.proc.stdin.write((cmd + "\n").encode())
        self.proc.stdin.flush()

    def _read_line(self) -> str:
        """Read text status line from backend."""
        line = self.proc.stdout.readline()
        if not line:
            raise RuntimeError("Backend process terminated")
        return line.decode().rstrip("\n\r")

    def _read_bytes(self, n: int) -> bytes:
        """Read n raw bytes from backend pipe."""
        # TODO: read until n bytes collected
        return b""

    def read_file(self, path: str) -> bytes | None:
        """Request file contents from backend via READ <path>."""
        # TODO: send READ, parse OK/ERROR, read payload
        return None

    def write_file(self, path: str, data: bytes) -> bool:
        """Write file data to path via WRITE <size> <path>."""
        # TODO: ensure parent dir exists, send WRITE header + data, return status
        return False

    def compress(self, filename: str, data: bytes) -> tuple[bool, bytes]:
        """Ask backend to compress data if beneficial."""
        # TODO: send COMPRESS, handle COMPRESSED or RAW response
        return False, data

    def decompress(self, original_size: int, data: bytes) -> bytes | None:
        """Ask backend to decompress payload back to original size."""
        # TODO: send DECOMPRESS, read reconstructed bytes
        return None

    def quit(self):
        """Clean shutdown of backend."""
        try:
            self._send_cmd("QUIT")
            self.proc.wait(timeout=2)
        except Exception:
            self.proc.kill()


# -----------------------------------------------------------------------------
# Socket Helpers
# -----------------------------------------------------------------------------

def send_all(sock: socket.socket, data: bytes):
    """Send all bytes across socket."""
    # TODO: sock.sendall(data)
    pass

def recv_all(sock: socket.socket, n: int) -> bytes:
    """Receive exact byte length from socket."""
    # TODO: loop sock.recv until n bytes received
    return b""

def send_val(sock: socket.socket, val: int, size: int):
    """Pack and send big-endian integer (size: 1, 4, 8 bytes)."""
    # TODO: struct.pack with !B, !I, or !Q
    pass

def recv_val(sock: socket.socket, size: int) -> int:
    """Receive and unpack big-endian integer."""
    # TODO: recv_all and struct.unpack
    return 0


# -----------------------------------------------------------------------------
# File Transfer Wire Logic
# Header format: [opcode: 1B][name_len: 4B][name][compressed: 1B]
#                [orig_size: 8B][payload_size: 8B][payload]
# -----------------------------------------------------------------------------

def send_file_over_socket(sock: socket.socket, backend: Backend, filepath: str):
    """Read via backend, compress if possible, send over socket."""
    # TODO: read file, compress, write header, send payload
    pass

def recv_file_from_socket(sock: socket.socket, backend: Backend, save_dir: str):
    """Receive file from socket, decompress if needed, write via backend."""
    # TODO: read header, receive payload, decompress if needed, save
    pass


# -----------------------------------------------------------------------------
# Server Mode
# -----------------------------------------------------------------------------

def handle_client(client: socket.socket, backend: Backend, share_dir: str):
    """Dispatch client request based on opcode."""
    # TODO: parse opcode (OP_REQUEST_FILE / OP_SEND_FILE) and handle
    pass

def run_server(port: int, share_dir: str):
    """Initialize server listener and accept loop."""
    # TODO: setup socket, bind, listen, and dispatch connections
    pass


# -----------------------------------------------------------------------------
# Client Mode
# -----------------------------------------------------------------------------

def run_client(host: str, port: int):
    """Terminal menu for requesting or uploading files."""
    # TODO: menu loop (1: request file, 2: send file, 0: exit)
    pass


# -----------------------------------------------------------------------------
# CLI Entry Point
# -----------------------------------------------------------------------------

def main():
    if len(sys.argv) < 2:
        print("Usage:")
        print(f"  python {sys.argv[0]} server [share_dir] [port]")
        print(f"  python {sys.argv[0]} client <server_ip> [port]")
        sys.exit(1)

    mode = sys.argv[1]
    port = DEFAULT_PORT

    if mode == "server":
        share_dir = sys.argv[2] if len(sys.argv) >= 3 else "./cloud"
        if len(sys.argv) >= 4:
            port = int(sys.argv[3])
        run_server(port, share_dir)

    elif mode == "client":
        if len(sys.argv) < 3:
            print("Client mode requires server IP.")
            sys.exit(1)
        host = sys.argv[2]
        if len(sys.argv) >= 4:
            port = int(sys.argv[3])
        run_client(host, port)

    else:
        print(f"Unknown mode: {mode}")
        sys.exit(1)

if __name__ == "__main__":
    main()
