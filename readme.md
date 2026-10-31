# not-LZMA
not-LZMA is a lossless data compressor written in C. It is an educational implementation of the LZMA algorithm, featuring LZSS, an arithmetic range coder and a simple context model for literals.

# Features
* LZSS with a circular buffer and hash chains
* Range coder with proper carry propagation
* 1 byte adaptive context model for literals, simple adaptive models for matches
* Pure C11

# Architecture
*   `src/search_window.c` — circular buffer, hash chains, match finding.
*   `src/lzss_parser.c` — LZ77 parser (produces tokens).
*   `src/lzss_decoder.c` — LZ77 decoder (applies tokens).
*   `src/models.c` — context models (literals, lengths, distances).
*   `src/range_coder.c` / `range_decoder.c` — entropy coding.
*   `src/lzma.c` — top-level (connects parser, models, and range coder).
*   `src/main.c` — CLI interface.
*   `tests/test_lzma_roundtrip.c` — round-trip test runner.

# Build
```bash
mkdir build
cd build
cmake ..
cmake --build .
```

# Usage
## CLI
The archiver supports the next commands:
./build/notlzma compress input.txt output.notlzma
./build/notlzma decompress output.notlzma restored.txt

## Testing
The project includes an automated round-trip test that compresses and decompresses every file in tests/data/, then verifies that the output exactly matches the original.

Run the test from the project root directory!
./build/test_lzma_roundtrip

You can add your own test data by adding files into the test/data/ directory

# Benchmarks
  [OK] war_and_peace.txt: 2609729 B -> 770194 B (29.5%) | C: 1.327s, D: 0.386s
  [OK] enwik8: 100000000 B -> 36088544 B (36.1%) | C: 67.982s, D: 17.722s

# Plans
* [ ] slots + bits for distance encoding
* [ ] rep-distances
* [ ] optimal parser
* [ ] bit-tree models
* [ ] multi-file archiver
