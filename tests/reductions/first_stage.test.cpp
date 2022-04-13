#include "reductions.hpp"

#include <cassert>
#include <algorithm>
#include <iostream>

using namespace std;

namespace first_stage {
namespace test {
    namespace single_case {
        constexpr array<array<size_t, 4>, 4> ind {{
            { 0, 13, 10,  7},
            { 4,  1, 14, 11},
            { 8,  5,  2, 15},
            {12,  9,  6,  3}
        }};

        void reduction(FlatState const& Y, FlatState const& Y_, size_t fault_position, FlatState const& K) {
            auto [antidiag1, antidiag2, antidiag3, antidiag4] = first_stage::reduction(Y, Y_, fault_position);
            
            auto it1 = find(antidiag1.begin(), antidiag1.end(),
                Row {K[ind[0][0]], K[ind[0][1]], K[ind[0][2]], K[ind[0][3]]}
            );
            auto it2 = find(antidiag2.begin(), antidiag2.end(),
                Row {K[ind[1][0]], K[ind[1][1]], K[ind[1][2]], K[ind[1][3]]}
            );
            auto it3 = find(antidiag3.begin(), antidiag3.end(),
                Row {K[ind[2][0]], K[ind[2][1]], K[ind[2][2]], K[ind[2][3]]}
            );
            auto it4 = find(antidiag4.begin(), antidiag4.end(),
                Row {K[ind[3][0]], K[ind[3][1]], K[ind[3][2]], K[ind[3][3]]}
            );

            assert (it1 != antidiag1.end());
            assert (it2 != antidiag2.end());
            assert (it3 != antidiag3.end());
            assert (it4 != antidiag4.end());
        }
    }

    void reduction() {
        int diff_column;
        FlatState Y, Y_, K;
        size_t fault_position;

        cout << "Testing `first_stage_reduction`..." << endl;
        //----------------------------------------------------------------------------------------------------
        cout << "\tTest 1... ";
        
        Y  = {0x8e, 0x21, 0x2b, 0x34, 0x96, 0xfb, 0xee, 0xa4, 0x5f, 0x18, 0x9b, 0x50, 0xd5, 0xb2, 0xf8, 0x8a};
        Y_ = {0x22, 0x0a, 0x7d, 0xde, 0xb1, 0xa0, 0x9c, 0x32, 0xd2, 0xb4, 0x4e, 0xe9, 0x00, 0xdb, 0x1a, 0xcb};
        K  = {0x13, 0xfd, 0x12, 0x19, 0x0b, 0x2d, 0xc4, 0xcc, 0x2b, 0x56, 0x60, 0xb6, 0xc1, 0xed, 0xc1, 0x71};
        diff_column = 0;
        fault_position = 0;
        
        single_case::reduction(Y, Y_, fault_position, K);
        cout << "passed !" << endl;
        //----------------------------------------------------------------------------------------------------
        cout << "\tTest 2... ";
        
        Y  = {0x0d, 0xe9, 0xdd, 0x2d, 0xdb, 0xd0, 0x0a, 0xdc, 0x77, 0x73, 0xae, 0xab, 0x0a, 0x8b, 0x2c, 0x1b};
        Y_ = {0xc9, 0xc1, 0x13, 0x3f, 0x11, 0x3c, 0xaf, 0xad, 0x07, 0x36, 0x41, 0xf7, 0x6c, 0x33, 0x45, 0x20};
        K  = {0x47, 0x35, 0x0e, 0x0f, 0x84, 0xa2, 0xa2, 0x6b, 0xc9, 0xa6, 0x09, 0xec, 0xee, 0x44, 0xf4, 0xc5};
        diff_column = 0;
        fault_position = 0;
        
        single_case::reduction(Y, Y_, fault_position, K);
        cout << "passed !" << endl;
        //----------------------------------------------------------------------------------------------------
        cout << "\tTest 3... ";
        
        Y  = {0x19, 0x31, 0x66, 0xa2, 0xf6, 0x40, 0xd1, 0xdf, 0xfd, 0x27, 0xeb, 0x66, 0x43, 0xef, 0xd7, 0xfd};
        Y_ = {0xcc, 0x48, 0x0c, 0xdf, 0x5d, 0x39, 0x5b, 0x80, 0xc3, 0xf3, 0xb4, 0x8b, 0xb0, 0x7a, 0xa8, 0x27};
        K  = {0xa1, 0xfd, 0xe6, 0xd5, 0xb3, 0xfc, 0xb6, 0xcf, 0xf8, 0x9c, 0xfb, 0x08, 0x29, 0xaf, 0xfe, 0x2a};
        diff_column = 1;
        fault_position = 4;
        
        single_case::reduction(Y, Y_, fault_position, K);
        cout << "passed !" << endl;
        //----------------------------------------------------------------------------------------------------
        cout << "\tTest 4... ";

        Y  = {0xe8, 0x0c, 0xb3, 0xeb, 0x27, 0x77, 0xf8, 0xd0, 0x22, 0x13, 0x93, 0x14, 0xe2, 0xad, 0x5e, 0xba};
        Y_ = {0xa5, 0x89, 0xa1, 0xfe, 0xa2, 0x9b, 0x09, 0x5f, 0x47, 0x11, 0x66, 0xf6, 0xc9, 0x48, 0xc9, 0xbb};
        K  = {0xa6, 0x04, 0x50, 0x7b, 0xeb, 0xb3, 0xb9, 0x99, 0xb6, 0xe9, 0x53, 0x84, 0xe7, 0x6a, 0xfb, 0x0a};
        diff_column = 2;
        fault_position = 8;
        
        single_case::reduction(Y, Y_, fault_position, K);
        cout << "passed !" << endl;
        //----------------------------------------------------------------------------------------------------
        cout << "\tTest 5... ";
        
        Y  = {0x42, 0x38, 0x5f, 0x91, 0x85, 0xf2, 0x5b, 0xcc, 0xe4, 0xf0, 0x32, 0xbf, 0xbb, 0xf9, 0x22, 0xc9};
        Y_ = {0x4e, 0x14, 0x54, 0x5a, 0x48, 0x90, 0xdb, 0xc1, 0x4a, 0x85, 0x1e, 0xfa, 0xf0, 0x33, 0xd0, 0x6d};
        K  = {0x21, 0xcb, 0xab, 0x34, 0x6f, 0x27, 0x2c, 0xb7, 0xe0, 0x59, 0xb8, 0x3c, 0xdc, 0xe9, 0x9b, 0x68};
        diff_column = 3;
        fault_position = 12;
        
        single_case::reduction(Y, Y_, fault_position, K);
        cout << "passed !" << endl;
        //----------------------------------------------------------------------------------------------------
    }
}
}

int main() {
    first_stage::test::reduction();
    return 0;
}