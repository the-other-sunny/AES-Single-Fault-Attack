import numpy as np

from aes_utils.lookup_tables import RCON, SBOX, INV_SBOX, MUL_BY_1, MUL_BY_2, MUL_BY_3, MUL_BY_9, MUL_BY_11, MUL_BY_13, MUL_BY_14

def hex2square(hex_string):
    return np.array(
            bytearray.fromhex(hex_string),
            dtype=np.uint8
        ).reshape((4, 4), order='F')

def square2hex(square):
    return bytes(square.flatten(order='F')).hex()

def rot_word(word):
    return word[[1, 2, 3, 0]]

def sub_word(word):
    return SBOX[word]

def rcon(i):
    return np.array([RCON[i], 0, 0, 0], dtype=np.uint8)

def _key_expansion_aux(prev_key, round):
    key = np.empty((4, 4), dtype=np.uint8)

    key[:, 0] = prev_key[:, 0] ^ sub_word(rot_word(prev_key[:, 3])) ^ rcon(round)
    key[:, 1] = prev_key[:, 1] ^ key[:, 0]
    key[:, 2] = prev_key[:, 2] ^ key[:, 1]
    key[:, 3] = prev_key[:, 3] ^ key[:, 2]

    return key

def key_expansion(key, rounds=10):
    yield key
    for round in range(1, rounds+1):
        key = _key_expansion_aux(key, round)
        yield key

def _inv_key_expansion_aux(key, round):
    prev_key = np.empty((4, 4), dtype=np.uint8)

    prev_key[:, 3] = key[:, 3] ^ key[:, 2]
    prev_key[:, 2] = key[:, 2] ^ key[:, 1]
    prev_key[:, 1] = key[:, 1] ^ key[:, 0]
    
    prev_key[:, 0] = key[:, 0] ^ sub_word(rot_word(prev_key[:, 3])) ^ rcon(round)

    return prev_key

def inv_key_expansion(key, rounds=10):
    yield key
    for round in reversed(range(1, rounds+1)):
        key = _inv_key_expansion_aux(key, round)
        yield key

def sub_bytes(state):
    return SBOX[state]

def inv_sub_bytes(state):
    return INV_SBOX[state]

def shift_rows(state):
    return np.array([
            [state[0, 0], state[0, 1], state[0, 2], state[0, 3]], # << 0
            [state[1, 1], state[1, 2], state[1, 3], state[1, 0]], # << 1
            [state[2, 2], state[2, 3], state[2, 0], state[2, 1]], # << 2
            [state[3, 3], state[3, 0], state[3, 1], state[3, 2]], # << 3
        ], dtype=np.uint8)

def inv_shift_rows(state):
    return np.array([
            [state[0, 0], state[0, 1], state[0, 2], state[0, 3]], # >> 0
            [state[1, 3], state[1, 0], state[1, 1], state[1, 2]], # >> 1
            [state[2, 2], state[2, 3], state[2, 0], state[2, 1]], # >> 2
            [state[3, 1], state[3, 2], state[3, 3], state[3, 0]], # >> 3
        ], dtype=np.uint8)

def _mix_column(c):
    return np.array([
            MUL_BY_2[c[0]] ^ MUL_BY_3[c[1]] ^ MUL_BY_1[c[2]] ^ MUL_BY_1[c[3]],
            MUL_BY_1[c[0]] ^ MUL_BY_2[c[1]] ^ MUL_BY_3[c[2]] ^ MUL_BY_1[c[3]],
            MUL_BY_1[c[0]] ^ MUL_BY_1[c[1]] ^ MUL_BY_2[c[2]] ^ MUL_BY_3[c[3]],
            MUL_BY_3[c[0]] ^ MUL_BY_1[c[1]] ^ MUL_BY_1[c[2]] ^ MUL_BY_2[c[3]]
        ], dtype=np.uint8)

def mix_columns(state):
    result = np.empty((4, 4), dtype=np.uint8)

    for j in range(4):
        result[:, j] = _mix_column(state[:, j])
    
    return result

def _inv_mix_column(c):
    return np.array([
            MUL_BY_14[c[0]] ^ MUL_BY_11[c[1]] ^ MUL_BY_13[c[2]] ^ MUL_BY_9 [c[3]],
            MUL_BY_9 [c[0]] ^ MUL_BY_14[c[1]] ^ MUL_BY_11[c[2]] ^ MUL_BY_13[c[3]],
            MUL_BY_13[c[0]] ^ MUL_BY_9 [c[1]] ^ MUL_BY_14[c[2]] ^ MUL_BY_11[c[3]],
            MUL_BY_11[c[0]] ^ MUL_BY_13[c[1]] ^ MUL_BY_9 [c[2]] ^ MUL_BY_14[c[3]]
        ], dtype=np.uint8)

def inv_mix_columns(state):
    result = np.empty((4, 4), dtype=np.uint8)

    for j in range(4):
        result[:, j] = _inv_mix_column(state[:, j])
    
    return result

def AES128_encrypt(state, key):
    keys = list(key_expansion(key, 10))
    
    state = state ^ keys[0]

    for round in range(1, 10):
        state = sub_bytes(state)
        state = shift_rows(state)
        state = mix_columns(state)
        state = state ^ keys[round]
    
    state = sub_bytes(state)
    state = shift_rows(state)
    state = state ^ keys[10]

    return state

def AES128_decrypt(state, key):
    keys = list(key_expansion(key, 10))

    state = state ^ keys[10]
    state = inv_shift_rows(state)
    state = inv_sub_bytes(state)

    for round in reversed(range(1, 10)):
        state = state ^ keys[round]
        state = inv_mix_columns(state)
        state = inv_shift_rows(state)
        state = inv_sub_bytes(state)
    
    state = state ^ keys[0]

    return state