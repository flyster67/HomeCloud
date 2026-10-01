# HomeCloud

A minimal personal cloud server & client to turn a leftover laptop into local network storage.

## Architecture

- **Backend (`backend.c`)**: Fast C engine handling file reading/writing and compression using zlib.
- **Frontend / Network (`homecloud.py`)**: Python socket manager handling TCP connections, routing, and client CLI interface over standard I/O pipes.

## Build

Compile the C backend:

```bash
# Windows (portable standalone binary, bakes in zlib)
gcc backend.c -o backend.exe -lz -static

# Linux
gcc backend.c -o backend -lz
```

## Dependencies

- **Python 3.7+** (built-in standard library only: `socket`, `subprocess`, `struct`, `os`, `sys`).
- **GCC / Clang** (C99+).
- **zlib**:
  - **Linux**: Pre-installed on virtually all distributions (`libz.so`).
  - **Windows**: To compile, you need zlib installed locally (e.g., via MSYS2 `pacman -S mingw-w64-x86_64-zlib`). Adding `-static` during compilation bakes the library directly into `backend.exe`, so you don't need to ship `zlib1.dll` to other computers.

## Usage

Start server on laptop:
```bash
python homecloud.py server ./cloud 7878
```

Connect client from another machine:
```bash
python homecloud.py client <laptop_ip> 7878
```

## Origin
i've had this laptop for a pretty long while,and its specs as a laptop are pretty bad. it has intengrated graphics and only 8gb ram & 500GB space.
But - for something like a server/cloud, that is a lot more than enough.   
so, i took the knowledge i learnt in magshimim & did some homework on libs and decided to make it myself, instead of vibecoding or looking for something that already exists.
