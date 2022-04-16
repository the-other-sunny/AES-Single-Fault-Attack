# AES-Single-Fault-Attack

## Overview

A tool to perform differential fault analysis on `AES-128`.

The fault is supposed to be injected at round 8 entry.  
(Or, actually, anywhere between round 7 `MixColumns` output and round 8 `MixColumns` input as it can be brought back to the round 8 entry situation by adapting the fault position)

The key search space is first reduced to 258 candidates (on average) with the help of the regular cipher, the faulted cipher and the fault position.

Providing the plaintext will allow reducing the search space further to a single key.  

## Compilation

```console
g++ -Wall -std=c++17 -O3 -march=native -fopenmp src/main.cpp -I src -o aes-single-fault-attack
```

## Usage

```console
aes-single-fault-attack regular_cipher faulted_cipher fault_position [plaintext]
```

The regular cipher, faulted cipher and plaintext are provided as 32 characters big endian hex strings.  

The fault position is 0-based and follow row-major order as depicted below:  

```text
0 4  8 12
1 5  9 13
2 6 10 14
3 7 11 15
```

## Examples

```console
$ aes-single-fault-attack 37c093ea09426cc92d0835b887de4306 45d4cf7faa60c648973ff03eb18aa2d3 8 01758006f6c57ea32b4e7d6d065f86f1
1e4229783f73e10991fd40d0779f98a6
```

```console
$ aes-single-fault-attack 37c093ea09426cc92d0835b887de4306 45d4cf7faa60c648973ff03eb18aa2d3 8
5757068142559a7bf4ced45caf7e0379
b8e1bb4595c1eab82a8915ee75ab97fa
...
1e4229783f73e10991fd40d0779f98a6
...
b24918e3086f7adcc8b2822a20700457
4152c0056806ed90fcf986ec1aebcc85
```
## Credits

This project was inspired by the paper:

[Differential Fault Analysis of the Advanced Encryption
Standard using a Single Fault](https://eprint.iacr.org/2009/575.pdf)

Although the first step described in the paper is implemented as is, the second one is conducted here by partially decrypting the regular and faulted ciphers instead of solving the equations presented in the paper.
