#include <stdint.h>
#include <string.h>

namespace cgold {

    struct blob256 {

        public:
        uint8_t x[32];

        blob256() {

            memset(x, 0, 32);

        }

    };

    bool operator<(const blob256& x, const blob256& y) {
        return (memcmp(x, y, 32) < 0);
    }

    bool operator>(const blob256& x, const blob256& y) {
        return (memcmp(x, y, 32) > 0);
    }

    bool operator==(const blob256& x, const blob256& y) {
        return (memcmp(x, y, 32) == 0);
    }

    bool operator<=(const blob256& x, const blob256& y) {
        return (memcmp(x, y, 32) <= 0);
    }

    bool operator>=(const blob256& x, const blob256& y) {
        return (memcmp(x, y, 32) >= 0);
    }

    bool operator!=(const blob256& x, const blob256& y) {
        return (memcmp(x, y, 32) != 0);
    }

}
