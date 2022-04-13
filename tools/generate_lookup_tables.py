import argparse

import numpy as np
import galois

GF256 = galois.GF(order=2**8, irreducible_poly='x^8 + x^4 + x^3 + x + 1')
GF2   = galois.GF(2)
A     = GF2(np.array([[1, 1, 1, 1, 1, 0, 0, 0],
                      [0, 1, 1, 1, 1, 1, 0, 0],
                      [0, 0, 1, 1, 1, 1, 1, 0],
                      [0, 0, 0, 1, 1, 1, 1, 1],
                      [1, 0, 0, 0, 1, 1, 1, 1],
                      [1, 1, 0, 0, 0, 1, 1, 1],
                      [1, 1, 1, 0, 0, 0, 1, 1],
                      [1, 1, 1, 1, 0, 0, 0, 1]]))
INV_A = np.linalg.inv(A)
B     = GF2(np.array([0, 1, 1, 0, 0, 0, 1, 1]))


def mul(x, y):
    return int(GF256(x) * GF256(y))

def S(x):
    x = GF256(x)
    x = x**-1 if x != 0 else x
    X = x.vector()
    Y = A@X + B
    y = GF256.Vector(Y)
    y = int(y)
    
    return y

def inv_S(x):
    x = GF256(x)
    X = x.vector()
    Y = INV_A @ (X + B)
    y = GF256.Vector(Y)
    y = y**-1 if y != 0 else y
    y = int(y)
    
    return y

def format(byte):
    return f'0x{byte:02x}'

def print_func_table(f):
    ''''Prints the table of f: 8-bit int -> 8-bit int'''
    for i in range(0, 256, 16):
        s = ', '.join(format(f(j)) for j in range(i, i+16)) + ','
        print(s)

def print_multiplication_table(p):
    print(f'Multiply by {p}:')
    print_func_table(lambda x: mul(x, p))
    print()

def print_SBOX():
    print('S-box:')
    print_func_table(S)
    print()


def print_InvSBox():
    print('Inverse S-box:')
    print_func_table(inv_S)
    print()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('-M', nargs='+', help='multiplication table(s) to print', type=int)
    parser.add_argument('-S', help='print S-box', action='store_true')
    parser.add_argument('-iS', help='print inverse S-box', action='store_true')

    args = parser.parse_args()
    
    if args.M:
        for operand in args.M:
            print_multiplication_table(operand)
    
    if args.S:
        print_SBOX()
    
    if args.iS:
        print_InvSBox()

if __name__ == '__main__':
    main()
