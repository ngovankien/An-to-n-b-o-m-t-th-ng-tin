#include <iostream>
#include <vector>
#include <string>
#include <bitset>
#include <algorithm>
#include <stdexcept>
#include <cstdlib>

using namespace std;

// ========================= HELPER FUNCTIONS =========================

string convert_decimal_to_binary(int decimal) {
    return bitset<4>(decimal).to_string();
}

int convert_binary_to_decimal(const string& binary) {
    int value = 0;
    size_t i;
    for (i = 0; i < binary.size(); i++) {
        value = value * 2 + (binary[i] - '0');
    }
    return value;
}

string Xor(const string& a, const string& b) {
    string result = "";
    size_t i;
    for (i = 0; i < a.size(); i++) {
        result += (a[i] != b[i]) ? '1' : '0';
    }
    return result;
}

bool is_binary_string(const string& s) {
    size_t i;
    for (i = 0; i < s.size(); i++) {
        char c = s[i];
        if (c != '0' && c != '1') return false;
    }
    return true;
}

string pad_to_multiple_of_64(string input) {
    while (input.size() % 64 != 0) {
        input += '0';
    }
    return input;
}

vector<string> split_into_blocks_64(const string& input) {
    vector<string> blocks;
    size_t i;
    for (i = 0; i < input.size(); i += 64) {
        blocks.push_back(input.substr(i, 64));
    }
    return blocks;
}

// ========================= IP / IP^-1 =========================

string initial_permutation(const string& input) {
    const int initial_permutation[64] = {
        58,50,42,34,26,18,10,2,
        60,52,44,36,28,20,12,4,
        62,54,46,38,30,22,14,6,
        64,56,48,40,32,24,16,8,
        57,49,41,33,25,17,9,1,
        59,51,43,35,27,19,11,3,
        61,53,45,37,29,21,13,5,
        63,55,47,39,31,23,15,7
    };

    string permuted = "";
    int i;
    for (i = 0; i < 64; i++) {
        permuted += input[initial_permutation[i] - 1];
    }
    return permuted;
}

string inverse_initial_permutation(const string& input) {
    const int inverse_permutation[64] = {
        40,8,48,16,56,24,64,32,
        39,7,47,15,55,23,63,31,
        38,6,46,14,54,22,62,30,
        37,5,45,13,53,21,61,29,
        36,4,44,12,52,20,60,28,
        35,3,43,11,51,19,59,27,
        34,2,42,10,50,18,58,26,
        33,1,41,9,49,17,57,25
    };

    string permuted = "";
    int i;
    for (i = 0; i < 64; i++) {
        permuted += input[inverse_permutation[i] - 1];
    }
    return permuted;
}

// ========================= KEY GENERATOR =========================

class KeyGenerator {
private:
    string key;
    vector<string> roundKeys;

    static const int pc1[56];
    static const int pc2[48];

    string shift_left_once(const string& key_chunk) {
        return key_chunk.substr(1) + key_chunk[0];
    }

    string shift_left_twice(string key_chunk) {
        key_chunk = shift_left_once(key_chunk);
        return shift_left_once(key_chunk);
    }

public:
    KeyGenerator(const string& input_key) : key(input_key) {}

    void generateRoundKeys() {
        roundKeys.clear();

        string permutedKey = "";
        int i, j;

        for (i = 0; i < 56; ++i) {
            permutedKey += key[pc1[i] - 1];
        }

        string left = permutedKey.substr(0, 28);
        string right = permutedKey.substr(28, 28);

        for (i = 0; i < 16; ++i) {
            if (i == 0 || i == 1 || i == 8 || i == 15) {
                left = shift_left_once(left);
                right = shift_left_once(right);
            } else {
                left = shift_left_twice(left);
                right = shift_left_twice(right);
            }

            string combinedKey = left + right;
            string roundKey = "";

            for (j = 0; j < 48; ++j) {
                roundKey += combinedKey[pc2[j] - 1];
            }

            roundKeys.push_back(roundKey);
        }
    }

    const vector<string>& getRoundKeys() const {
        return roundKeys;
    }
};

const int KeyGenerator::pc1[56] = {
    57,49,41,33,25,17,9,
    1,58,50,42,34,26,18,
    10,2,59,51,43,35,27,
    19,11,3,60,52,44,36,
    63,55,47,39,31,23,15,
    7,62,54,46,38,30,22,
    14,6,61,53,45,37,29,
    21,13,5,28,20,12,4
};

const int KeyGenerator::pc2[48] = {
    14,17,11,24,1,5,
    3,28,15,6,21,10,
    23,19,12,4,26,8,
    16,7,27,20,13,2,
    41,52,31,37,47,55,
    30,40,51,45,33,48,
    44,49,39,56,34,53,
    46,42,50,36,29,32
};

// ========================= DES CLASS =========================

class DES {
private:
    static const int expansion_table[48];
    static const int permutation_tab[32];
    static const int substitution_boxes[8][4][16];

    vector<string> round_keys;

    string process_block(const string& input, const vector<string>& keys) {
        string perm = initial_permutation(input);

        string left = perm.substr(0, 32);
        string right = perm.substr(32, 32);

        int i, j;
        for (i = 0; i < 16; i++) {
            string right_expanded = "";
            for (j = 0; j < 48; j++) {
                right_expanded += right[expansion_table[j] - 1];
            }

            string xored = Xor(right_expanded, keys[i]);

            string res = "";
            for (j = 0; j < 8; j++) {
                string row_bits = "";
                row_bits += xored[j * 6];
                row_bits += xored[j * 6 + 5];
                int row = convert_binary_to_decimal(row_bits);

                string col_bits = xored.substr(j * 6 + 1, 4);
                int col = convert_binary_to_decimal(col_bits);

                int val = substitution_boxes[j][row][col];
                res += convert_decimal_to_binary(val);
            }

            string perm2 = "";
            for (j = 0; j < 32; j++) {
                perm2 += res[permutation_tab[j] - 1];
            }

            string new_right = Xor(perm2, left);
            left = right;
            right = new_right;
        }

        string combined_text = right + left;
        return inverse_initial_permutation(combined_text);
    }

public:
    DES(const vector<string>& keys) : round_keys(keys) {}

    string encrypt_block(const string& input) {
        return process_block(input, round_keys);
    }

    string decrypt_block(const string& input) {
        vector<string> reversed_keys = round_keys;
        reverse(reversed_keys.begin(), reversed_keys.end());
        return process_block(input, reversed_keys);
    }

    string encrypt(const string& plaintext) {
        string padded = pad_to_multiple_of_64(plaintext);
        vector<string> blocks = split_into_blocks_64(padded);

        string ciphertext = "";
        size_t i;
        for (i = 0; i < blocks.size(); i++) {
            ciphertext += encrypt_block(blocks[i]);
        }
        return ciphertext;
    }

    string decrypt(const string& ciphertext) {
        if (ciphertext.size() % 64 != 0) {
            throw runtime_error("Ciphertext must have length multiple of 64 bits.");
        }

        vector<string> blocks = split_into_blocks_64(ciphertext);
        string plaintext = "";
        size_t i;
        for (i = 0; i < blocks.size(); i++) {
            plaintext += decrypt_block(blocks[i]);
        }
        return plaintext;
    }
};

const int DES::expansion_table[48] = {
    32,1,2,3,4,5,4,5,
    6,7,8,9,8,9,10,11,
    12,13,12,13,14,15,16,17,
    16,17,18,19,20,21,20,21,
    22,23,24,25,24,25,26,27,
    28,29,28,29,30,31,32,1
};

const int DES::permutation_tab[32] = {
    16,7,20,21,29,12,28,17,
    1,15,23,26,5,18,31,10,
    2,8,24,14,32,27,3,9,
    19,13,30,6,22,11,4,25
};

const int DES::substitution_boxes[8][4][16] = {
    {
        {14,4,13,1,2,15,11,8,3,10,6,12,5,9,0,7},
        {0,15,7,4,14,2,13,1,10,6,12,11,9,5,3,8},
        {4,1,14,8,13,6,2,11,15,12,9,7,3,10,5,0},
        {15,12,8,2,4,9,1,7,5,11,3,14,10,0,6,13}
    },
    {
        {15,1,8,14,6,11,3,4,9,7,2,13,12,0,5,10},
        {3,13,4,7,15,2,8,14,12,0,1,10,6,9,11,5},
        {0,14,7,11,10,4,13,1,5,8,12,6,9,3,2,15},
        {13,8,10,1,3,15,4,2,11,6,7,12,0,5,14,9}
    },
    {
        {10,0,9,14,6,3,15,5,1,13,12,7,11,4,2,8},
        {13,7,0,9,3,4,6,10,2,8,5,14,12,11,15,1},
        {13,6,4,9,8,15,3,0,11,1,2,12,5,10,14,7},
        {1,10,13,0,6,9,8,7,4,15,14,3,11,5,2,12}
    },
    {
        {7,13,14,3,0,6,9,10,1,2,8,5,11,12,4,15},
        {13,8,11,5,6,15,0,3,4,7,2,12,1,10,14,9},
        {10,6,9,0,12,11,7,13,15,1,3,14,5,2,8,4},
        {3,15,0,6,10,1,13,8,9,4,5,11,12,7,2,14}
    },
    {
        {2,12,4,1,7,10,11,6,8,5,3,15,13,0,14,9},
        {14,11,2,12,4,7,13,1,5,0,15,10,3,9,8,6},
        {4,2,1,11,10,13,7,8,15,9,12,5,6,3,0,14},
        {11,8,12,7,1,14,2,13,6,15,0,9,10,4,5,3}
    },
    {
        {12,1,10,15,9,2,6,8,0,13,3,4,14,7,5,11},
        {10,15,4,2,7,12,9,5,6,1,13,14,0,11,3,8},
        {9,14,15,5,2,8,12,3,7,0,4,10,1,13,11,6},
        {4,3,2,12,9,5,15,10,11,14,1,7,6,0,8,13}
    },
    {
        {4,11,2,14,15,0,8,13,3,12,9,7,5,10,6,1},
        {13,0,11,7,4,9,1,10,14,3,5,12,2,15,8,6},
        {1,4,11,13,12,3,7,14,10,15,6,8,0,5,9,2},
        {6,11,13,8,1,4,10,7,9,5,0,15,14,2,3,12}
    },
    {
        {13,2,8,4,6,15,11,1,10,9,3,14,5,0,12,7},
        {1,15,13,8,10,3,7,4,12,5,6,11,0,14,9,2},
        {7,11,4,1,9,12,14,2,0,6,10,13,15,3,5,8},
        {2,1,14,7,4,10,8,13,15,12,9,0,3,5,6,11}
    }
};

// ========================= TRIPLE DES =========================

class TripleDES {
private:
    DES des1;
    DES des2;
    DES des3;

public:
    TripleDES(const vector<string>& k1, const vector<string>& k2, const vector<string>& k3)
        : des1(k1), des2(k2), des3(k3) {}

    string encrypt(const string& plaintext) {
        string padded = pad_to_multiple_of_64(plaintext);
        vector<string> blocks = split_into_blocks_64(padded);

        string ciphertext = "";
        size_t i;
        for (i = 0; i < blocks.size(); i++) {
            string step1 = des1.encrypt_block(blocks[i]);
            string step2 = des2.decrypt_block(step1);
            string step3 = des3.encrypt_block(step2);
            ciphertext += step3;
        }
        return ciphertext;
    }

    string decrypt(const string& ciphertext) {
        if (ciphertext.size() % 64 != 0) {
            throw runtime_error("Ciphertext must have length multiple of 64 bits.");
        }

        vector<string> blocks = split_into_blocks_64(ciphertext);
        string plaintext = "";
        size_t i;
        for (i = 0; i < blocks.size(); i++) {
            string step1 = des3.decrypt_block(blocks[i]);
            string step2 = des2.encrypt_block(step1);
            string step3 = des1.decrypt_block(step2);
            plaintext += step3;
        }
        return plaintext;
    }
};

// ========================= MAIN =========================

int main() {
    try {
        int choice;
        cout << "===== MENU DES TEST V2 =====" << endl;
        cout << "1. DES Encrypt/Decrypt" << endl;
        cout << "2. TripleDES Encrypt/Decrypt" << endl;
        cout << "Choose: ";
        cin >> choice;

        if (choice == 1) {
            string plaintext, key;
            cout << "Enter plaintext (binary string): ";
            cin >> plaintext;
            cout << "Enter key (64 bits binary): ";
            cin >> key;

            cout << "Da nhap xong." << endl;
            cout << "plaintext length = " << plaintext.size() << endl;
            cout << "key length = " << key.size() << endl;

            if (!is_binary_string(plaintext) || !is_binary_string(key)) {
                cout << "Error: Input must be binary strings only." << endl;
                system("pause");
                return 1;
            }

            if (key.size() != 64) {
                cout << "Error: DES key must be exactly 64 bits." << endl;
                system("pause");
                return 1;
            }

            cout << "Bat dau sinh round key..." << endl;
            KeyGenerator keygen(key);
            keygen.generateRoundKeys();
            cout << "Sinh round key xong." << endl;

            DES des(keygen.getRoundKeys());
            cout << "Bat dau ma hoa..." << endl;
            string ciphertext = des.encrypt(plaintext);
            cout << "Ma hoa xong." << endl;

            cout << "Bat dau giai ma..." << endl;
            string decrypted = des.decrypt(ciphertext);
            cout << "Giai ma xong." << endl;

            cout << endl;
            cout << "Ciphertext: " << ciphertext << endl;
            cout << "Decrypted : " << decrypted << endl;
        }
        else if (choice == 2) {
            string plaintext, key1, key2, key3;
            cout << "Enter plaintext (binary string): ";
            cin >> plaintext;
            cout << "Enter key1 (64 bits binary): ";
            cin >> key1;
            cout << "Enter key2 (64 bits binary): ";
            cin >> key2;
            cout << "Enter key3 (64 bits binary): ";
            cin >> key3;

            cout << "Da nhap xong." << endl;
            cout << "plaintext length = " << plaintext.size() << endl;
            cout << "key1 length = " << key1.size() << endl;
            cout << "key2 length = " << key2.size() << endl;
            cout << "key3 length = " << key3.size() << endl;

            if (!is_binary_string(plaintext) ||
                !is_binary_string(key1) ||
                !is_binary_string(key2) ||
                !is_binary_string(key3)) {
                cout << "Error: Input must be binary strings only." << endl;
                system("pause");
                return 1;
            }

            if (key1.size() != 64 || key2.size() != 64 || key3.size() != 64) {
                cout << "Error: Each TripleDES key must be exactly 64 bits." << endl;
                system("pause");
                return 1;
            }

            cout << "Bat dau sinh round key..." << endl;
            KeyGenerator kg1(key1);
            KeyGenerator kg2(key2);
            KeyGenerator kg3(key3);

            kg1.generateRoundKeys();
            kg2.generateRoundKeys();
            kg3.generateRoundKeys();
            cout << "Sinh round key xong." << endl;

            TripleDES tdes(kg1.getRoundKeys(), kg2.getRoundKeys(), kg3.getRoundKeys());

            cout << "Bat dau ma hoa..." << endl;
            string ciphertext = tdes.encrypt(plaintext);
            cout << "Ma hoa xong." << endl;

            cout << "Bat dau giai ma..." << endl;
            string decrypted = tdes.decrypt(ciphertext);
            cout << "Giai ma xong." << endl;

            cout << endl;
            cout << "TripleDES Ciphertext: " << ciphertext << endl;
            cout << "TripleDES Decrypted : " << decrypted << endl;
        }
        else {
            cout << "Invalid choice." << endl;
        }
    }
    catch (exception& e) {
        cout << "Runtime error: " << e.what() << endl;
    }

    system("pause");
    return 0;
}
