# osu-lazer-beatmap-import

Osu! Lazer allows you to import beatmaps from a previous installation of osu! however if you have a slow or perhaps external drive with a lot of beatmaps, it may not be able to import most of them. This tool written in C for Apple Silicon Macs efficiently zips each beatmap in the folder and opens them to ensure they all get imported properly. The ones that don't can easily be identified and dealt with afterwards.

## Features

- **Fast native C implementation** using libzip for efficient zip creation
- **Pause/Resume functionality** for handling large collections
- **Graceful stopping** to avoid data corruption
- **Progress tracking** with real-time status updates
- **Modular codebase** with proper project structure
- **Signal handling** for remote control

## Project Structure

```
├── src/                    # Source files
│   ├── main.c             # Main entry point
│   ├── beatmap_processor.c # Core beatmap processing logic
│   ├── zip_utils.c        # Zip file creation utilities
│   ├── signal_handler.c   # Signal handling for pause/stop
│   └── utils.c            # General utility functions
├── include/               # Header files
│   ├── beatmap_processor.h
│   ├── zip_utils.h
│   ├── signal_handler.h
│   └── utils.h
├── build/                 # Build artifacts
├── Makefile              # Build configuration
└── README.md
```

## Dependencies

- **libzip**: For efficient zip file creation
- **zlib**: Compression library (dependency of libzip)

On macOS with Homebrew:
```bash
brew install libzip
```

## Building

```bash
make clean
make
```

This will create the `lazer-beatmap-import` executable.

## Usage

```bash
./lazer-beatmap-import [beatmap directory]
```

**Note**: Directory path must NOT contain trailing slash

### Controls

- **Ctrl+C or SIGTERM**: Stop processing gracefully
- **SIGUSR1**: Pause processing
- **SIGUSR2**: Resume processing

### Example Signals

```bash
# Get the process ID when running
./lazer-beatmap-import /path/to/beatmaps &
PID=$!

# Pause processing
kill -USR1 $PID

# Resume processing  
kill -USR2 $PID

# Stop gracefully
kill -TERM $PID
```

## Output

The tool creates a `beatmap_output` directory in the current working directory containing `.osz` files for each beatmap. Each beatmap is automatically opened after creation to trigger the import process in osu! Lazer.

## Performance

This version uses native C with libzip instead of system calls for zip creation, providing:
- Faster processing of large beatmap collections
- Better error handling and reporting
- Lower system overhead
- More reliable zip file creation
