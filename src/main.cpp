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

void crack(string const& regular_ciphertext, string const& faulted_ciphertext, size_t fault_position, string const& plaintext = "") {
    vector<FlatState> found_keys;
    
    auto Y = string_to_state(regular_ciphertext);
    auto Y_ = string_to_state(faulted_ciphertext);

    auto stage1_results = first_stage::reduction(Y, Y_, fault_position);
    auto stage2_results = second_stage::reduction(Y, Y_, fault_position, stage1_results);
    
    if (plaintext != "") {
        auto X = string_to_state(plaintext);
        found_keys = third_stage::reduction(Y, X, stage2_results);  
    } else { 
        for (auto key : stage2_results)
            found_keys.push_back(get_initial_key(key));
    }
    
    for (auto key : found_keys)
            cout << key << endl;
}

int main(int argc, char* argv[]) {
    string regular_ciphertext, faulted_ciphertext, plaintext;
    size_t fault_position;

    if (argc != 4 && argc != 5) {
        cout << "Usage: aes-single-fault-attack regular_cipher faulted_cipher fault_position [plaintext]" << endl;
        return 1;
    }

    istringstream(argv[1]) >> regular_ciphertext;
    istringstream(argv[2]) >> faulted_ciphertext;
    istringstream(argv[3]) >> fault_position;
    if (argc == 5) istringstream(argv[4]) >> plaintext;

    // TODO: add checks to sanitize and validate input
    crack(regular_ciphertext, faulted_ciphertext, fault_position, plaintext);

    return 0;
};
