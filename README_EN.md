# QtreeComp - QuadTree Image Compression

This project was developed as part of a programming course at Université Gustave Eiffel in Paris. The implementation was carried out by Gonzalo Torras and Mario Lepage.

## Project Overview

This project implements a hierarchical image compression scheme based on the QuadTree data structure. The main goal is to compress grayscale images (PGM format) by exploiting spatial redundancy and uniformity in image blocks.

## Features

- Supports compression and decompression of PGM images (binary format P5).
- Images must be square with dimensions \(2^n \times 2^n\).
- Compression can be lossless or lossy, depending on user options.
- The output is a custom `.qtc` format file.

## Usage

### Encoding (Compression)

```sh
codec -c -i PGM/input.pgm -o QTC/output.qtc
```

### Decoding (Decompression)
```sh
codec -u -i QTC/input.qtc -o PGM/output.pgm
```

### Options

- `-c`: Encode (compress)
- `-u`: Decode (decompress)
- `-i <file>`: Input file (.pgm or .qtc)
- `-o <file>`: Output file (.qtc or .pgm)
- `-a <alpha>`: Lossy compression parameter (for encoding)
- `-g`: Generate segmentation grid (PGM output)
- `-h`: Help
- `-v`: Verbose mode

## File Formats

- Input: PGM (Portable Gray Map, binary P5)
- Output: QTC (custom QuadTree compressed format)

## Build Instructions

- The project is written in standard C.
- Use the provided Makefile to build the shared library (`libqtc.so`) and the executable (`codec`).
- All modules should be independently compilable.
