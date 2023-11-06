import sys
import re
import os

# ctypes - exactly what it says it is. Very useful for working with C.
from ctypes import CDLL, c_ulong, c_long

# grab ptrace func
cPtrace = CDLL("libc.so.6").ptrace
cPtrace.argtypes = (c_ulong, c_ulong, c_ulong, c_ulong) ## tell python its input types
cPtrace.restype = c_long ## tell python its return type

PTRACE_ATTACH = c_ulong(16)
PTRACE_DETACH = c_ulong(17)

## wrap it up in a nice func
def ptrace(request, pid):
    cPid = c_ulong(pid)
    return cPtrace(request, cPid, c_ulong(0), c_ulong(0))

mapsRegex = r"^([a-f0-9]+)\-([a-f0-9]+)\s(...)"

## reads the given procs maps - a list of ptrs mapped by mmap, a TON of things
## we are interested in the stack and heap which is on the list, but we can dump it all
def readMapsInfo(pid):
    filepath = f"/proc/{pid}/maps"
    with open(filepath) as f:
        mapsfile = f.read()
    lines = mapsfile.strip().splitlines()

    infoResults = []

    for line in lines:
        result = re.match(mapsRegex, line)
        if result is None:
            raise Exception(f"Couldn't read map info on line: {line}")
        (startaddr, endaddr, perm) = result.groups()
        infoResults.append((int(startaddr, 16), int(endaddr, 16), perm))

    return infoResults

if len(sys.argv) < 2:
    print(f"usage: python {sys.argv[0]} <pid>")
    exit(1)

pid = sys.argv[1]

mapsinfo = readMapsInfo(pid)

## basically we go through the address ranges mapped for the process and use mem.seek to read the memory,
## before dumping whatever we get into a file
with open(f"dump-{pid}.bin", "wb") as dumpfile:
    ptrace(PTRACE_ATTACH, int(pid)) ## attach & detach, so that the /proc/ virtual file system exists (becomes accessible)
    with open(f"/proc/{pid}/mem", "rb") as mem:
        for startaddr, endaddr, perm in mapsinfo:
            if "r" in perm and "w" in perm:
                chunkLen = endaddr - startaddr
                mem.seek(startaddr, os.SEEK_SET)
                chunk = mem.read(chunkLen)
                dumpfile.write(chunk)
    ptrace(PTRACE_DETACH, int(pid))