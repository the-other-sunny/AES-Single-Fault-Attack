# AES-Single-Fault-Attack

## Compilation

```console
g++ -Wall -std=c++17 -O3 -march=native -fopenmp src/main.cpp -I src -o aes-single-fault-attack
```

## Usage

```console
aes-single-fault-attack regular_cipher faulted_cipher fault_position [plaintext]
```

## Example

```console
$ aes-single-fault-attack 01758006f6c57ea32b4e7d6d065f86f1 37c093ea09426cc92d0835b887de4306 45d4cf7faa60c648973ff03eb18aa2d3 8
1e4229783f73e10991fd40d0779f98a6
```
## Credits

This project was inspired by the paper:

[Differential Fault Analysis of the Advanced Encryption
Standard using a Single Fault](https://eprint.iacr.org/2009/575.pdf)

Although the first step described in the paper is implemented the exact same way here, the second one is done by partially decrypting the regular and faulted ciphers instead of solving the equations presented in the paper.
