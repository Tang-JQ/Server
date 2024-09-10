#include <iostream>
#include <string>

int findSubstringAfterPos(const std::string& S, const std::string& T, int pos) {
    // Adjust pos to be zero-indexed
    pos--;

    // Check if pos is within the bounds of the string S
    if (pos < 0 || pos >= S.length()) {
        return 0;
    }

    // Find the substring T starting from pos
    std::string subS = S.substr(pos);

    // Find the position of T in the substring
    size_t found = subS.find(T);

    if (found != std::string::npos) {
        // Return the position relative to the original string S
        return found + pos + 1;
    }
    else {
        return 0;
    }
}

int main() {
    std::string S, T;
    int pos;

    while (std::cin >> S >> T >> pos) {
        std::cout << findSubstringAfterPos(S, T, pos) << std::endl;
        return 0;
    }

    return 0;
}
