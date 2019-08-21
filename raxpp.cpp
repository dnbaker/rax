#include "raxpp.h"

int main() {
    rs::KVStore<int> ks;
    auto it = ks.begin();
    while(it != ks.end()) {
        std::fprintf(stderr, "I am a thing here.\n");
    }
    return 0;
}
