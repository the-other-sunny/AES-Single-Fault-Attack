#include <array>
#include <vector>

#include <omp.h>

#include "lookup_tables.hpp"
#include "aes_ni_utils.hpp"

using namespace std;

const int THREADS = 8;

using u8 = unsigned char;
using Row = array<u8, 4>;
using FlatState = array<u8, 16>;

namespace first_stage {
    /**
     * @brief Compute the cartesian product of 4 sets.
     * 
     * @param (a, b, c, d) sets of elements used for the cartesian product
     * @return vector<array<u8, 4>> containing all x, y, z, t for x in `a`, y in `b`, z in `c` and t in `d`.
     */
    vector<array<u8, 4>> cartesian_product(vector<u8> const& a, vector<u8> const& b, vector<u8> const& c, vector<u8> const& d) {
        vector<Row> v;
        
        for (auto& xa : a) for (auto& xb : b) for (auto& xc : c) for (auto& xd : d)
            v.push_back({xa, xb, xc, xd});
        
        return v;
    };

    /**
     * Exhaustive search of all the solutions x in GF(256) of: 
     * 
     *      invS(x + a) + invS(x + b) = c
     * 
     * @param a, b, c equation parameters
     * @return vector<u8> found solutions
     */
    vector<u8> solve_GF256_equation(const u8 a, const u8 b, const u8 c) {
        vector<u8> v;
        
        for (int x = 0; x < 256; ++x)
            if ((INV_SBOX[x ^ a] ^ INV_SBOX[x ^ b]) == c)
                v.push_back(x);
        
        return v;
    }

    /**
     * Given indices (i0, i1, i2, i3) and appropriate factors (f0, f1, f2, f3),
     * return possible values of (K_{i0}, K_{i1}, K_{i2}, K_{i3}).
     * (K being round 10 key)
     * 
     * More exactly, find those values that satisfy the following for some \delta:
     * 
     *      invS(K_{i0} + Y_{i0}) + invS(K_{i0} + Y'_{i0}) = f_0 * \delta
     *      invS(K_{i1} + Y_{i1}) + invS(K_{i1} + Y'_{i1}) = f_1 * \delta
     *      invS(K_{i2} + Y_{i2}) + invS(K_{i2} + Y'_{i2}) = f_2 * \delta
     *      invS(K_{i3} + Y_{i3}) + invS(K_{i3} + Y'_{i3}) = f_3 * \delta
     * 
     * @param Y regular cipher
     * @param Y_ faulted cipher
     * @param ind indices (i0, i1, i2, i3)
     * @param factors factors (f0, f1, f2, f3)
     * @return vector<Row> possible values of partial key (at indices i0, i1, i2, i3)
     */
    vector<Row> partial_key_space_reduction(FlatState const& Y, FlatState const& Y_, array<size_t, 4> ind, array<size_t, 4> factors) {
        vector<Row> result;

        for (unsigned int delta = 1; delta < 256; ++delta) {
            auto v0 = solve_GF256_equation(Y[ind[0]], Y_[ind[0]], MUL[factors[0]][delta]);
            auto v1 = solve_GF256_equation(Y[ind[1]], Y_[ind[1]], MUL[factors[1]][delta]);
            auto v2 = solve_GF256_equation(Y[ind[2]], Y_[ind[2]], MUL[factors[2]][delta]);
            auto v3 = solve_GF256_equation(Y[ind[3]], Y_[ind[3]], MUL[factors[3]][delta]);

            for (auto [k_i0, k_i1, k_i2, k_i3] : cartesian_product(v0, v1, v2, v3))
                result.push_back({k_i0, k_i1, k_i2, k_i3});
        }

        return result;
    }

    /**
     * Return the position of the non-zero column of the differential state
     * after the first MixColumns following fault injection.
     * 
     * @param fault_position 
     * @return size_t differential column position
     */
    size_t get_diff_column(size_t fault_position) {
        // TODO: handle invalid input
        switch (fault_position) {
            case  0: case  5: case 10: case 15: return 0;
            case  4: case  9: case 14: case  3: return 1;
            case  8: case 13: case  2: case  7: return 2;
            case 12: case  1: case  6: case 11: return 3;
        }
    }

    /**
     * @brief Parameters used in first stage reduction.
     * 
     * @param diff_column 
     * @return array<array<size_t, 4>, 4> factors
     */
    array<array<size_t, 4>, 4> get_factors(size_t diff_column) {
        // TODO: handle invalid input
        switch (diff_column) {
            case 0:
                return {{
                    {2, 1, 1, 3},
                    {1, 1, 3, 2},
                    {1, 3, 2, 1},
                    {3, 2, 1, 1}
                }};
            case 1:
                return {{
                    {3, 2, 1, 1},
                    {2, 1, 1, 3},
                    {1, 1, 3, 2},
                    {1, 3, 2, 1}
                }};
            case 2:
                return {{
                    {1, 3, 2, 1},
                    {3, 2, 1, 1},
                    {2, 1, 1, 3},
                    {1, 1, 3, 2}
                }};
            case 3:
                return {{
                    {1, 1, 3, 2},
                    {1, 3, 2, 1},
                    {3, 2, 1, 1},
                    {2, 1, 1, 3}
                }};
        }
    }

    /**
     * Return the possible values of each of the quadruples:
     * 
     *      ( k1, k14, k11,  k8)
     *      ( k5,  k2, k15, k12)
     *      ( k9,  k6,  k3, k16)
     *      (k13, k10,  k7,  k4)
     * 
     * On average, each set of values contains 256 instances, reducing
     * the search space to 2^32 round 10 keys.
     * 
     * @param Y regular cipher
     * @param Y_ faulted cipher
     * @param fault_position index of the fault position in [0, 16)
     * @return array<vector<Row>, 4> 
     */
    array<vector<Row>, 4> reduction(FlatState const& Y, FlatState const& Y_, size_t fault_position) {
        // This should be declared somewhere else...
        constexpr array<array<size_t, 4>, 4> ind {{
            { 0, 13, 10,  7},
            { 4,  1, 14, 11},
            { 8,  5,  2, 15},
            {12,  9,  6,  3}
        }};

        size_t diff_column = get_diff_column(fault_position);
        auto factors   = get_factors(diff_column);

        auto antidiag1 = partial_key_space_reduction(Y, Y_, ind[0], factors[0]); // values of ( k1, k14, k11,  k8)
        auto antidiag2 = partial_key_space_reduction(Y, Y_, ind[1], factors[1]); // values of ( k5,  k2, k15, k12)
        auto antidiag3 = partial_key_space_reduction(Y, Y_, ind[2], factors[2]); // values of ( k9,  k6,  k3, k16)
        auto antidiag4 = partial_key_space_reduction(Y, Y_, ind[3], factors[3]); // values of (k13, k10,  k7,  k4)

        return {antidiag1, antidiag2, antidiag3, antidiag4};
    }
}

namespace second_stage {
    /**
     * @brief Return the key formed of first stage reduction results `ad1`, `ad2`, `ad3`, `ad4`.
     * 
     * @param ad1
     * @param ad2
     * @param ad3
     * @param ad4 
     * @return FlatState key
     */
    inline FlatState make_key(Row const& ad1, Row const& ad2, Row const& ad3, Row const& ad4) {
        // This should be declared somewhere else...
        constexpr array<array<size_t, 4>, 4> ind {{
            { 0, 13, 10,  7},
            { 4,  1, 14, 11},
            { 8,  5,  2, 15},
            {12,  9,  6,  3}
        }};
        
        FlatState K;

        K[ind[0][0]] = ad1[0]; K[ind[0][1]] = ad1[1]; K[ind[0][2]] = ad1[2]; K[ind[0][3]] = ad1[3];
        K[ind[1][0]] = ad2[0]; K[ind[1][1]] = ad2[1]; K[ind[1][2]] = ad2[2]; K[ind[1][3]] = ad2[3];
        K[ind[2][0]] = ad3[0]; K[ind[2][1]] = ad3[1]; K[ind[2][2]] = ad3[2]; K[ind[2][3]] = ad3[3];
        K[ind[3][0]] = ad4[0]; K[ind[3][1]] = ad4[1]; K[ind[3][2]] = ad4[2]; K[ind[3][3]] = ad4[3];

        return K;
    }

    /**
     * @brief Reduce the possible round 10 keys to 256 instances on average.
     * 
     * @param Y regular ciphertext
     * @param Y_ faulted ciphertext
     * @param fault_position
     * @param stage1_results
     * @return vector<FlatState> 
     */
    vector<FlatState> reduction(FlatState const& Y, FlatState const& Y_, size_t fault_position, array<vector<Row>, 4> const& stage1_results) {
        vector<FlatState> found_keys;
        
        int fault_mask = get_fault_mask(fault_position);
        auto [antidiags1, antidiags2, antidiags3, antidiags4] = stage1_results;

        omp_set_num_threads(THREADS);

        #pragma omp parallel for
        for (auto const& ad1 : antidiags1)
            for (auto const& ad2 : antidiags2)
                for (auto const& ad3 : antidiags3)
                    for (auto const& ad4 : antidiags4)
                    {
                        FlatState K10 = make_key(ad1, ad2, ad3, ad4);
                        if (check_partial_decryption(Y, Y_, K10, fault_mask))
                        #pragma omp critical
                        {
                            found_keys.push_back(K10);
                        }
                    }
        
        return found_keys;
    }
}

namespace third_stage {
    /**
     * @brief Find the key used to encrypt `plaintext`.
     * 
     * @param ciphertext 
     * @param plaintext 
     * @param stage2_results 
     * @return vector<FlatState> key
     */
    vector<FlatState> reduction(FlatState const& ciphertext, FlatState const& plaintext, vector<FlatState> const& stage2_results) {
        vector<FlatState> valid_keys;

        for (auto K10 : stage2_results)
            if (decrypt(ciphertext, K10) == plaintext) {
                auto K0 = get_initial_key(K10);
                valid_keys.push_back(K0);
            }
        
        return valid_keys;
    }
}