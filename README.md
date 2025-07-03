# wcpp

**wcpp** is a compact, fast C++ implementation of the classic Unix `wc` (word count) utility.
It prints line, word, and byte counts for each file, similar to GNU `wc`, but written from scratch using C++20.

## Features

- Flags:
    - `-l`, `--lines` — count lines
    - `-w`, `--words` — count words
    - `-c`, `--bytes` — count bytes
- Totals for multiple files
- Formatted, aligned output like GNU `wc`

## Build

```bash
git clone https://github.com/MeNToS64exe/wcpp.git
cd wcpp
chmod +x ./install.sh
./install.sh
```

## Usage

```bash
./wcpp [OPTION]... [FILE]... # use flag '--help' for more information
```

If no option is specified, all three counts will be shown.

## TODO

- No `--files0-from=F` and support for stdin (`-`) yet
- No `--chars`, `--max-line-length`, or `--total` mode

## License

GPL-3.0
