#include <wmmintrin.h>

#include <array>

using namespace std;

using u8 = unsigned char;
using Row = array<u8, 4>;
using FlatState = array<u8, 16>;

// -----------------------------------------------------------------------------------------------------------------------------------

// Reason for using template -> `_mm_aeskeygenassist_si128` requires `rcon` to be an immediate
template<int rcon>
inline __m128i single_step_key_inversion(__m128i k) {
    // K4' = K4 xor K3
    // K3' = K3 xor K2
    // K2' = K2 xor K1
    // K1' = K1 xor SubWord(RotWord(K4')) xor RCON
    __m128i i, j;

    i = _mm_slli_si128(k, 4);                       // i <- [K3, K2, K1, 0]
    k = _mm_xor_si128(k, i);                        // k <- [K4, K3, K2, K1] xor [K3, K2, K1, 0] = [K4', K3', K2', K1]
    
    j = _mm_aeskeygenassist_si128(k, rcon);         // l <- [SubWord(RotWord(K4')) xor RCON, .., .., ..]
    j = _mm_srli_si128(j, 12);                      // l <- [0, 0, 0, SubWord(RotWord(K4')) xor RCON]
    k = _mm_xor_si128(k, j);                        // k <- [K4', K3', K2', K1 xor SubWord(RotWord(K4')) xor RCON]

    return k;
}

auto key_schedule_from_last_round_key(__m128i k10) {
    array<__m128i, 11> keys;
    
    keys[10] = k10;
    keys[ 9] = single_step_key_inversion<0x36>(keys[10]);
    keys[ 8] = single_step_key_inversion<0x1b>(keys[ 9]);
    keys[ 7] = single_step_key_inversion<0x80>(keys[ 8]);
    keys[ 6] = single_step_key_inversion<0x40>(keys[ 7]);
    keys[ 5] = single_step_key_inversion<0x20>(keys[ 6]);
    keys[ 4] = single_step_key_inversion<0x10>(keys[ 5]);
    keys[ 3] = single_step_key_inversion<0x08>(keys[ 4]);
    keys[ 2] = single_step_key_inversion<0x04>(keys[ 3]);
    keys[ 1] = single_step_key_inversion<0x02>(keys[ 2]);
    keys[ 0] = single_step_key_inversion<0x01>(keys[ 1]);

    return keys;
}

__m128i get_initial_key(__m128i k10) {
    return key_schedule_from_last_round_key(k10)[0];
}

__m128i decrypt(__m128i m, __m128i k10) {
    auto keys = key_schedule_from_last_round_key(k10);
    
    m  = _mm_xor_si128(m, keys[10]);

    for (int i = 9; i != 0; --i) {
        __m128i kimc = _mm_aesimc_si128(keys[i]);
        m = _mm_aesdec_si128(m, kimc);
    }

    m = _mm_aesdeclast_si128(m, keys[0]);

    return m;
}

inline bool check_partial_decryption(__m128i m, __m128i m_, __m128i k10, int fault_mask) {
    // compute the needed keys
    __m128i k9    = single_step_key_inversion<0x36>(k10);
    __m128i k9imc = _mm_aesimc_si128(k9);
    __m128i k8    = single_step_key_inversion<0x1b>(k9);
    __m128i k8imc = _mm_aesimc_si128(k8);

    // partial decryptions
    m  = _mm_xor_si128(m , k10);
    m  = _mm_aesdec_si128(m, k9imc);
    m  = _mm_aesdec_si128(m, k8imc);

    m_  = _mm_xor_si128(m_ , k10);
    m_  = _mm_aesdec_si128(m_, k9imc);
    m_  = _mm_aesdec_si128(m_, k8imc);

    // assessing that the partially decrypted messages are identical apart from the injected fault location
    bool is_valid = ((_mm_movemask_epi8(_mm_cmpeq_epi8(m, m_)) == fault_mask)); // fault_mask = 0xfffe for a fault at position (0, 0)
    
    return is_valid;
}

// Interface -------------------------------------------------------------------------------------------------------------------------

inline __m128i load(FlatState const& X) {
    return _mm_loadu_si128((__m128i *) X.data());
}

FlatState unload(__m128i x) {
    FlatState X;

    _mm_storeu_si128((__m128i *) X.data(), x);

    return X;
}

FlatState get_initial_key(FlatState const& K10) {
    __m128i k = load(K10);
    k = get_initial_key(k);

    FlatState K0 = unload(k);
    
    return K0;
}

FlatState decrypt(FlatState const& Y, FlatState const& K10) {
    __m128i m   = load(Y);
    __m128i k10 = load(K10);
    __m128i p   = decrypt(m, k10);

    FlatState P = unload(p);

    return P;
}

int get_fault_mask(size_t fault_position) {
    auto shift_left = [](int fault_position, int count) {
        fault_position = (fault_position - 4*count) % 16;
        if (fault_position < 0) fault_position += 16;
        return (size_t)fault_position;
    };

    switch (fault_position){
        case 0: case 4: case  8: case 12:
            break;
        case 1: case 5: case  9: case 13:
            fault_position = shift_left(fault_position, 1);
            break;
        case 2: case 6: case 10: case 14:
            fault_position = shift_left(fault_position, 2);
            break;
        case 3: case 7: case 11: case 15:
            fault_position = shift_left(fault_position, 3);
            break;
    }

    int fault_mask = ~(1 << fault_position) & 0b00000000000000001111111111111111;

    return fault_mask;
}

inline bool check_partial_decryption(FlatState const& Y, FlatState const& Y_, FlatState const& K10, int fault_mask) {
    __m128i y   = load(Y);
    __m128i y_  = load(Y_);
    __m128i k10 = load(K10);

    return check_partial_decryption(y, y_, k10, fault_mask);
}