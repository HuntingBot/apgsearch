#include "blockheader.h"
#include "mining.h"
#include "cvm.h"

void print_testing_info() {

    std::string genesis_seed = cgold::Blockheader(0).prevblock_seed();
    std::cerr << genesis_seed << std::endl;

    SoupSearcher ss;
    apg::lifetree<uint32_t, BITPLANES> lt(LIFETREE_MEM);
    apg::base_classifier<BITPLANES> cfier(&lt, RULESTRING);
    auto p = ss.censusSoup(genesis_seed, "", cfier);
    std::cerr << p.first << " : " << p.second << std::endl;

    std::cerr << "nanotime: " << cgold::nanotime() << std::endl;

}

#ifdef FULLNODE



#else

int main(int argc, char* argv[]) {

    if (apg::rule2int(RULESTRING) != 0) {
        std::cerr << "Abort: apgsearch rule does not match lifelib rule" << std::endl;
        return 1;
    }

    print_testing_info();

    std::string modus_operandi = "";
    if (argc >= 2) {
        modus_operandi = argv[1];
    }

    if ((modus_operandi == "mine") && (argc >= 5)) {
        // Remove first argument:
        // return run_apgluxe(argc - 1, argv + 1);
        int parallelism = std::stoll(argv[2]);
        std::string pkey = argv[3];
        std::string addr = argv[4];
        greedy_mine(parallelism, pkey, addr);

    } else if (modus_operandi == "difficulty") {
        std::cerr << "Enter apgcodes line-by-line, with Ctrl+D to exit" << std::endl;
        std::string apgcode;
        SoupSearcher ss;
        while (std::getline(std::cin, apgcode)) {
            std::string rep = ss.representative(apgcode);
            auto difficulty = ss.get_difficulty(apgcode);
            std::cout << apgcode << " : " << difficulty << " : " << rep << std::endl;
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
