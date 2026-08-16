#!/usr/bin/env python3
"""
Trivial Parallel Block-Mode SSS CLI Wrapper
Hardcoded to max ssss block size (128 hex chars / 64 bytes) with concurrent execution.
"""

import argparse
import binascii
import subprocess
import sys
import os
from concurrent.futures import ThreadPoolExecutor, as_completed

CHUNK_SIZE = 128  # Maximum hex characters ssss supports natively in -x mode
DEFAULT_T = 3
DEFAULT_N = 5

def process_split_chunk(c_idx, chunk, t, n):
    cmd = f"echo -n '{chunk}' | ssss-split -t {t} -n {n} -x -q"
    try:
        output = subprocess.check_output(cmd, shell=True, text=True).strip().split('\n')
        shard_parts = {}
        for line in output:
            if '-' in line:
                idx, share_hex = line.split('-', 1)
                shard_parts[int(idx)] = share_hex
        return c_idx, shard_parts
    except subprocess.CalledProcessError as e:
        raise RuntimeError(f"Error in ssss-split on chunk {c_idx}: {e}")

def split_file(input_path, t=DEFAULT_T, n=DEFAULT_N):
    if not os.path.exists(input_path):
        print(f"Error: Input file '{input_path}' not found.", file=sys.stderr)
        sys.exit(1)

    with open(input_path, 'rb') as f:
        hex_data = binascii.hexlify(f.read()).decode('utf-8')

    total_chunks = (len(hex_data) + CHUNK_SIZE - 1) // CHUNK_SIZE
    print(f"[*] Splitting {os.path.getsize(input_path)} bytes into {total_chunks} blocks ({t}-of-{n} threshold)...")

    chunks = [(i, hex_data[i:i + CHUNK_SIZE]) for i in range(0, len(hex_data), CHUNK_SIZE)]
    shards = {i: [""] * total_chunks for i in range(1, n + 1)}

    with ThreadPoolExecutor() as executor:
        futures = {executor.submit(process_split_chunk, idx, chunk, t, n): idx for idx, (_, chunk) in enumerate(chunks)}
        
        for future in as_completed(futures):
            c_idx, shard_parts = future.result()
            for s_id, s_hex in shard_parts.items():
                shards[s_id][c_idx] = s_hex

    for i in range(1, n + 1):
        out_name = f"{input_path}.shard{i}"
        with open(out_name, 'w') as f:
            f.write(f"{i}-" + "".join(shards[i]) + "\n")
        print(f"[+] Written: {out_name}")

def process_recon_chunk(c_idx, chunk_inputs, t):
    combine_input = "".join(chunk_inputs)
    cmd = ["ssss-combine", "-t", str(t), "-x", "-q"]
    process = subprocess.run(cmd, input=combine_input.encode(), capture_output=True)
    if process.returncode != 0:
        raise RuntimeError(f"Error in ssss-combine on block {c_idx}: {process.stderr.decode()}")
    return c_idx, process.stdout.decode('utf-8').strip()

def reconstruct_file(share_paths, output_path, t=DEFAULT_T):
    loaded_data = []
    active_shards = []

    for path in share_paths:
        if not os.path.exists(path):
            print(f"Error: Share file '{path}' not found.", file=sys.stderr)
            sys.exit(1)
        with open(path, 'r') as f:
            line = f.read().strip()
            if '-' not in line:
                print(f"Error: Invalid share format in '{path}'.", file=sys.stderr)
                sys.exit(1)
            idx_str, share_hex = line.split('-', 1)
            active_shards.append(int(idx_str))
            loaded_data.append(share_hex)

    if len(active_shards) < t:
        print(f"Error: Provided {len(active_shards)} shares, but threshold requires at least {t}.", file=sys.stderr)
        sys.exit(1)

    total_chunks = len(loaded_data[0]) // CHUNK_SIZE
    print(f"[*] Reconstructing file using {len(active_shards)} shards ({total_chunks} blocks)...")

    reconstructed_chunks = ["" for _ in range(total_chunks)]

    with ThreadPoolExecutor() as executor:
        futures = {}
        for c in range(total_chunks):
            chunk_inputs = []
            for idx, shard_hex in zip(active_shards, loaded_data):
                chunk = shard_hex[c * CHUNK_SIZE : (c + 1) * CHUNK_SIZE]
                chunk_inputs.append(f"{idx}-{chunk}\n")
            futures[executor.submit(process_recon_chunk, c, chunk_inputs, t)] = c

        for future in as_completed(futures):
            c_idx, hex_chunk = future.result()
            reconstructed_chunks[c_idx] = hex_chunk

    reconstructed_hex = "".join(reconstructed_chunks)

    try:
        binary_data = binascii.unhexlify(reconstructed_hex)
    except Exception as e:
        print(f"Error unhexlify: {e}. Reconstructed data may be corrupted or shares mismatched.", file=sys.stderr)
        sys.exit(1)

    with open(output_path, 'wb') as f:
        f.write(binary_data)
    print(f"[+] Successfully reconstructed file to: {output_path}")

def main():
    parser = argparse.ArgumentParser(description="Parallelized block-mode ssss wrapper.")
    subparsers = parser.add_subparsers(dest="command", required=True)

    # Split
    p_split = subparsers.add_parser("split", help="Split file into 5 shards (3 required)")
    p_split.add_argument("input", help="Path to input file")
    p_split.add_argument("-t", type=int, default=DEFAULT_T, help="Threshold (default: 3)")
    p_split.add_argument("-n", type=int, default=DEFAULT_N, help="Total shares (default: 5)")

    # Reconstruct
    p_recon = subparsers.add_parser("reconstruct", help="Reconstruct file from shards")
    p_recon.add_argument("shares", nargs="+", help="Path to shard files")
    p_recon.add_argument("-o", "--output", required=True, help="Path to output reconstructed file")
    p_recon.add_argument("-t", type=int, default=DEFAULT_T, help="Threshold (default: 3)")

    args = parser.parse_args()

    if args.command == "split":
        split_file(args.input, args.t, args.n)
    elif args.command == "reconstruct":
        reconstruct_file(args.shares, args.output, args.t)

if __name__ == '__main__':
    main()
