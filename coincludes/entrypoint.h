#include "blockheader.h"
#include "cvm.h"

#ifdef FULLNODE



#else

int main(int argc, char* argv[]) {

    /*
    std::string genesis_seedroot = cgold::Blockheader(0, 0).prevblock_seed().substr(0, 48);
    char genesis_hash[33]; std::memset(genesis_hash, 0, 33);
    std::cerr << genesis_seedroot << std::endl;
    std::cerr << cgold::human_unreadable(genesis_seedroot, ((uint8_t*) genesis_hash)) << std::endl;
    std::cerr << genesis_hash << std::endl;
    */

    std::string modus_operandi = "";
    if (argc >= 2) {
        modus_operandi = argv[1];
    }

    if (modus_operandi == "mine") {
        // Remove first argument:
        return run_apgluxe(argc - 1, argv + 1);
    } else if (modus_operandi == "difficulty") {
        std::cerr << "Enter apgcodes line-by-line, with Ctrl+D to exit" << std::endl;
        std::string apgcode;
        SoupSearcher ss;
        while (std::getline(std::cin, apgcode)) {
            std::string rep = ss.representative(apgcode);
            int64_t difficulty = ss.get_difficulty(apgcode);
            std::cout << apgcode << " : " << difficulty << " : " << apgcode << std::endl;
        }
    } else if (modus_operandi == "addrgen") {
        std::cerr << "Enter passwords line-by-line, with Ctrl+D to exit" << std::endl;
        cgold::addrgen();
    } else {
        std::cerr << "Usage: ./lifecoin MODUS_OPERANDI [OPTIONS]" << std::endl;
        std::cerr << "    where MODUS_OPERANDI is one of the following:\n" << std::endl;

        std::cerr << "    mine : run the apgluxe soup searcher" << std::endl;
        std::cerr << "    addrgen : convert password(s) in stdin to address(es) in stdout" << std::endl;
        return 1;
    }

    return 0;
}

#endif
