#include "reductions.hpp"

#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>

using namespace std;

FlatState string_to_state(string const& s) {
    FlatState X;
    
    auto hex_to_int = [](string s) {
        int out;
        istringstream(s) >> hex >> out;
        return out;
    };

    for (int i = 0; i < 16; ++i) {
        auto couple_of_hex_chars = s.substr(2*i, 2);
        X[i] = hex_to_int(couple_of_hex_chars);
    }

    return X;
}

ostream& operator<<(ostream& os, FlatState const& state) {
    for (int x : state)
        os << setw(2) << setfill('0') << hex << x;
    
    os << setw(0) << setfill(' ') << dec; // back to default formatting

    return os;
}

void crack(string const& regular_ciphertext, string const& faulted_ciphertext, size_t fault_position, string const& plaintext) {
    auto X = string_to_state(plaintext);
    auto Y = string_to_state(regular_ciphertext);
    auto Y_ = string_to_state(faulted_ciphertext);

    auto stage1_results = first_stage::reduction(Y, Y_, fault_position);
    auto stage2_results = second_stage::reduction(Y, Y_, fault_position, stage1_results);
    auto stage3_results = third_stage::reduction(Y, X, stage2_results);

    for (auto const& key : stage3_results)
        cout << key << endl;
}

void crack(string const& regular_ciphertext, string const& faulted_ciphertext, size_t fault_position) {
    auto Y = string_to_state(regular_ciphertext);
    auto Y_ = string_to_state(faulted_ciphertext);

    auto stage1_results = first_stage::reduction(Y, Y_, fault_position);
    auto stage2_results = second_stage::reduction(Y, Y_, fault_position, stage1_results);

    for (auto const& K10 : stage2_results)
        cout << get_initial_key(K10) << endl;
}

int main(int argc, char* argv[]) {
    string plaintext, regular_ciphertext, faulted_ciphertext;
    size_t fault_position;

    istringstream(argv[1]) >> regular_ciphertext;
    istringstream(argv[2]) >> faulted_ciphertext;
    istringstream(argv[3]) >> fault_position;

    if (argc == 4) {
        crack(regular_ciphertext, faulted_ciphertext, fault_position);
    } else if (argc == 5) {
        istringstream(argv[4]) >> plaintext;
        crack(regular_ciphertext, faulted_ciphertext, fault_position, plaintext);
    } else {
        cout << "Usage: aes-single-fault-attack regular_cipher faulted_cipher fault_position [plaintext]" << endl;
    }

    return 0;
};
