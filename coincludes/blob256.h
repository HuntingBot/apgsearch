#pragma once

#include <stdint.h>
#include <string.h>

namespace cgold {

    struct blob256 {

        public:
        uint8_t data[32];

        blob256() {

            memset(data, 0, 32);

        }

    };

    int compare(const blob256 &x, const blob256 &y) {
        return memcmp(x.data, y.data, 32);
    }

    bool operator<(const blob256& x, const blob256& y) {
        return (compare(x, y) < 0);
    }

    bool operator>(const blob256& x, const blob256& y) {
        return (compare(x, y) > 0);
    }

    bool operator==(const blob256& x, const blob256& y) {
        return (compare(x, y) == 0);
    }

    bool operator<=(const blob256& x, const blob256& y) {
        return (compare(x, y) <= 0);
    }

    bool operator>=(const blob256& x, const blob256& y) {
        return (compare(x, y) >= 0);
    }

    bool operator!=(const blob256& x, const blob256& y) {
        return (compare(x, y) != 0);
    }

}
