# VASE — Vidhu's AmbiSonic Engine

A real-time binaural spatial audio engine written in C++ that makes mono audio sound like it's spinning around your head in 3D space. Built from scratch with minimal dependencies — only [miniaudio](https://miniaud.io) for audio I/O.

## Demo

Feed any audio file in and hear it orbit continuously around your head through standard stereo headphones. No special hardware required.

## How it works

The pipeline encodes mono audio into First Order Ambisonics (FOA), rotates the sound field using a continuously advancing LFO, then decodes to binaural stereo via HRTF convolution:

```
mono audio
    ↓
SH Encoder  — encodes to 4 FOA channels (W, X, Y, Z) using spherical harmonic basis functions
    ↓
Rotation    — LFO advances azimuth each block, set_direction() updates encoding coefficients
    ↓
HRTF Decoder — decodes FOA to 8 virtual speaker signals via decode matrix,
               convolves each with left/right HRTF using overlap-save FFT convolution
    ↓
binaural stereo (L / R) → headphones
```

Every component is implemented from scratch:

- **FFT** — Cooley-Tukey radix-2 with precomputed twiddle and bit-reversal tables
- **Overlap-save convolution** — block-by-block FFT convolution with history buffer for continuous output
- **Spherical harmonic encoding** — FOA basis functions evaluated at source direction
- **Binaural decoding** — virtual loudspeaker decode matrix + HRTF convolution
- **HRTF dataset** — SADIE II KU100 dummy head, 9201 directions at 48kHz

## Dependencies

| Dependency                      | Version | Purpose                                 |
| ------------------------------- | ------- | --------------------------------------- |
| [miniaudio](https://miniaud.io) | 0.11+   | Cross-platform audio I/O, file decoding |

That's it. No FFTW, no libspatializer, no game engine SDK.

## Building

**Requirements**

- CMake 3.20+
- C++17 compiler (clang++ or g++)
- Python 3 + numpy + scipy (one-time HRTF generation only)

**Clone and build**

```bash
git clone https://github.com/vidhu-vv/VASE
cd VASE
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 4
```

**Generate the HRTF header (first time only)**

Download the SADIE II dataset:

```bash
curl -L -o data/SADIE2.zip "https://zenodo.org/records/10886409/files/D2.zip?download=1"
unzip data/SADIE2.zip -d data/SADIE2
```

Generate `src/hrtf_data.h`:

```bash
pip install numpy scipy
python tools/gen_hrtf_header.py
```

Then rebuild:

```bash
cmake --build build --parallel 4
```

## Usage

```bash
./build/VASE path/to/audio.mp3
```

Supports any format miniaudio can decode — WAV, MP3, FLAC. Output is always stereo 48kHz through your default audio device. Use headphones for the spatial effect.

Press Enter to quit.

## Project structure

```
VASE/
├── CMakeLists.txt
├── deps/
│   └── miniaudio.h               — single-header audio library
├── src/
│   ├── main.cpp                  — entry point, miniaudio device + callback
│   ├── ambi_engine.h / .cpp      — pipeline coordinator
│   ├── fft.h / .cpp              — Cooley-Tukey radix-2 FFT
│   ├── fft_convolver.h / .cpp    — overlap-save FFT convolution
│   ├── sh_encoder.h / .cpp       — spherical harmonic FOA encoder
│   ├── hrtf_decoder.h / .cpp     — virtual loudspeaker binaural decoder
│   ├── hrtf_data.h               — SADIE II HRTF table (auto-generated)
│   └── rotation.h / .cpp         — LFO phase accumulator
├── data/
│   └── test.wav                  — example audio file
└── tools/
    └── gen_hrtf_header.py        — converts SADIE II WAVs → hrtf_data.h
```

## Signal processing pipeline in depth

### FFT (`fft.h / fft.cpp`)

Implements the Cooley-Tukey radix-2 decimation-in-time FFT. A single call to `fft_precompute(N)` at startup builds two lookup tables — twiddle factors ($W^k_N = e^{-j2\pi k/N}$) and bit-reversed indices — so no trigonometry is computed at runtime. Both `fft()` and `ifft()` operate in-place on a `Complex[]` array of size N (must be a power of two).

### Overlap-save convolution (`fft_convolver.h / .cpp`)

Each `Convolver` instance holds the precomputed FFT of one HRTF impulse response (`H[k]`), a history buffer of M-1 samples, and a working buffer. Every `process()` call:

1. Builds an extended block: `[history (M-1) | input (L) | zero pad]` of length FFT_SIZE
2. FFTs the extended block → `X[k]`
3. Pointwise multiplies `X[k] * H[k]`
4. IFFTs back to time domain
5. Discards the first M-1 contaminated samples, writes L valid samples to output
6. Saves last M-1 input samples as history for the next block

With M=256 and L=512, FFT_SIZE=1024.

### Spherical harmonic encoder (`sh_encoder.h / .cpp`)

Encodes a mono source at direction (azimuth φ, elevation θ) into four FOA channels using the real-valued spherical harmonic basis functions:

```
W = 0.7071
X = cos(θ) · cos(φ)
Y = cos(θ) · sin(φ)
Z = sin(θ)
```

`set_direction()` precomputes these four coefficients. `process()` multiplies every input sample by them — four multiplications per sample.

### HRTF decoder (`hrtf_decoder.h / .cpp`)

Places 8 virtual speakers on a sphere (cube arrangement, ±35° elevation, 45°/135°/225°/315° azimuth). For each speaker a decode matrix row is computed from the same SH basis functions, normalised by `N_FOA_CHANNELS / N_SPEAKERS`. Each block:

1. Matrix multiply: FOA channels → 8 mono speaker signals (dot product per sample)
2. Convolve each speaker signal with its left and right HRTF via `Convolver`
3. Sum all left outputs → L channel, all right outputs → R channel

16 `Convolver` instances run simultaneously — one per speaker per ear.

### HRTF dataset (`hrtf_data.h`)

Generated from the SADIE II KU100 dataset (Neumann dummy head, 48kHz, 24-bit). `gen_hrtf_header.py` reads the WAV files, converts SADIE II's anti-clockwise azimuth convention to standard clockwise, truncates IRs to 256 samples, and writes a static C array `hrtf_irs[9201][2][256]`. `find_closest_hrtf()` does a linear search over the direction table to find the nearest measured direction to any requested azimuth/elevation.

### Rotation (`rotation.h / .cpp`)

A phase accumulator that advances by `2π · f_spin · L / f_s` radians per block, wraps at 2π, and returns the current azimuth in degrees. At 0.5 Hz with block size 512 and sample rate 48kHz, the increment is 1.92°/block and one full orbit takes ~2 seconds.

## HRTF dataset

This project uses the **SADIE II Database** (Subject D2, KU100 Neumann dummy head).

> Amengual Garí, S. V., Kearney, G., Nando Ferrer, J., & McKenzie, T. (2024). SADIE II: A Binaural Database Including Measurements of Head-Related Transfer Functions and Headphone Impulse Responses. Zenodo. <https://doi.org/10.5281/zenodo.10886409>

Licensed under Apache 2.0.

## References

- Cooley, J.W. & Tukey, J.W. (1965). _An Algorithm for the Machine Calculation of Complex Fourier Series_. Mathematics of Computation.
- Gardner, W.G. (1995). _Efficient Convolution without Input-Output Delay_. JAES 43(3).
- Zotter, F. & Frank, M. (2019). _Ambisonics: A Practical 3D Audio Theory_. Springer Open.
- Ivanic, J. & Ruedenberg, K. (1996). _Rotation Matrices for Real Spherical Harmonics_. Journal of Physical Chemistry A.
- Møller, H. (1992). _Fundamentals of binaural technology_. Applied Acoustics.

## License

MIT
