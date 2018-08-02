#include "cryptography.h"
#include "blob256.h"
#include <map>

namespace cgold {

    struct CVM {

        public:

        std::map<blob256, bytevec> subroutines;

        void register_subroutine(bytevec &sub) {

            blob256 hash;

            SHA256 ctx = SHA256();
            ctx.init();
            ctx.update(sub.data(), sub.size());
            ctx.final(hash.data);
            
            subroutines.emplace(hash, sub);

        }

    };


}
