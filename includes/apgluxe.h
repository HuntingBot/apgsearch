#ifdef _WIN32
#include <winsock.h>
#endif

int run_apgluxe(int argc, char *argv[]) {

    if (apg::rule2int(RULESTRING) != 0) {
        std::cerr << "Abort: apgsearch rule does not match lifelib rule" << std::endl;
        return 1;
    }

    #ifdef _WIN32
    WSAData wsaData;
    WSAStartup(MAKEWORD(1, 1), &wsaData);
    #endif

    // Default values:
    int64_t soups_per_haul = 10000000;
    std::string payoshaKey = "#anon";
    std::string seed = reseed("original seed");
    int parallelisation = 0;

    #ifdef STDIN_SYM
    int local_log = 1;
    bool testing = true;
    int verifications = 0;
    #else
    int local_log = 0;
    bool testing = false;
    int verifications = -1;
    #endif

    int nullargs = 1;
    bool quitByUser = false;

    int iterations = 0;

    bool consumed_rule = false;
    
    // Extract options:
    for (int i = 1; i < argc - 1; i++) {
        if (strcmp(argv[i], "-k") == 0) {
            payoshaKey = argv[i+1];
        } else if (strcmp(argv[i], "-s") == 0) {
            seed = argv[i+1];
        } else if (strcmp(argv[i], "-n") == 0) {
            soups_per_haul = atoll(argv[i+1]);
        } else if (strcmp(argv[i], "-v") == 0) {
            verifications = atoi(argv[i+1]);
        } else if (strcmp(argv[i], "-i") == 0) {
            iterations = atoi(argv[i+1]);
        } else if (strcmp(argv[i], "-L") == 0) {
            local_log = atoi(argv[i+1]);
        } else if (strcmp(argv[i], "-t") == 0) {
            testing = atoi(argv[i+1]);
            if (testing) { iterations = 1; }
        } else if (strcmp(argv[i], "-p") == 0) {
            parallelisation = atoi(argv[i+1]);
        } else if (strcmp(argv[i], "--rule") == 0) {
            if (!consumed_rule) {
                std::cout << "\033[1;33mapgluxe " << APG_VERSION << "\033[0m: ";
                std::string desired_rulestring = argv[i+1];
                if (strcmp(RULESTRING, argv[i+1]) == 0) {
                    std::cout << "Rule \033[1;34m" << RULESTRING << "\033[0m is correctly configured." << std::endl;
                } else {
                    std::cout << "Rule \033[1;34m" << RULESTRING << "\033[0m does not match desired rule \033[1;34m";
                    std::cout << desired_rulestring << "\033[0m." << std::endl;
                    execvp("./recompile.sh", argv);
                    return 1;
                }
            }
            nullargs += 2;
            consumed_rule = true;
        } else if (strcmp(argv[i], "--symmetry") == 0) {
            std::cout << "\033[1;33mapgluxe " << APG_VERSION << "\033[0m: ";
            std::string desired_symmetry = argv[i+1];
            if (strcmp(SYMMETRY, argv[i+1]) == 0) {
                std::cout << "Symmetry \033[1;34m" << SYMMETRY << "\033[0m is correctly configured." << std::endl;
                nullargs += 2;
            } else {
                std::cout << "Symmetry \033[1;34m" << SYMMETRY << "\033[0m does not match desired symmetry \033[1;34m";
                std::cout << desired_symmetry << "\033[0m." << std::endl;
                execvp("./recompile.sh", argv);
                return 1;
            }
        }
    }

    if ((argc == nullargs) && (argc > 1)) { return 0; }

    if (argc == 1) {
        std::cout << "Please enter number of soups per haul (minimum 10000000): ";
        std::cin >> soups_per_haul;
        if (soups_per_haul < 10000000) { soups_per_haul = 10000000; }
        std::cout << "Please enter payosha256 key (e.g. '#anon'): ";
        std::cin >> payoshaKey;
        while ((payoshaKey.substr(0, 1) == "'") || (payoshaKey[0] == '"')) {
            payoshaKey = payoshaKey.substr(1);
            payoshaKey = payoshaKey.substr(0, payoshaKey.length() - 1);
        }
        #ifndef __CYGWIN__
        std::cout << "Please enter number of CPU threads to use (e.g. 4): ";
        std::cin >> parallelisation;
        #endif
    }

    #ifdef __CYGWIN__
    if (parallelisation > 0) {
        std::cout << "Warning: parallelisation disabled on Cygwin." << std::endl;
        parallelisation = 0;
    }
    #endif

    // Disable verification by default if running on a HPC;
    // otherwise verify three hauls per uploaded haul:
    if (verifications < 0) {
        verifications = (parallelisation <= 4) ? 5 : 0;
    }
    
    std::cout << "\nGreetings, this is \033[1;33mapgluxe " << APG_VERSION;
    std::cout << "\033[0m, configured for \033[1;34m" << RULESTRING << "/";
    std::cout << SYMMETRY << "\033[0m.\n" << std::endl;

    std::cout << "\033[32;1mLifelib version:\033[0m " << LIFELIB_VERSION << std::endl;
    std::cout << "\033[32;1mCompiler version:\033[0m " << __VERSION__ << std::endl;
    std::cout << "\033[32;1mPython version:\033[0m " << PYTHON_VERSION << std::endl;

    std::cout << std::endl;

    if (soups_per_haul <= 0) {
        std::cout << "Soups per haul, " << soups_per_haul << ", must be positive." << std::endl;
        return 1;
    }

    if (soups_per_haul > 100000000000ll) {
        std::cout << "Soups per haul reduced to maximum of 10^11" << std::endl;
        soups_per_haul = 100000000000ll;
    }

    while (!quitByUser) {
        if (verifications > 0) {
            std::cout << "Peer-reviewing hauls:\n" << std::endl;
            // Verify some hauls:
            for (int j = 0; j < verifications; j++) {
                bool earlyquit = verifySearch(payoshaKey);
                if (earlyquit) { break; }
            }
            std::cout << "\nPeer-review complete; proceeding search.\n" << std::endl;
        }

        // Run the search:
        std::cout << "Using seed " << seed << std::endl;
        if (parallelisation > 0) {
            quitByUser = parallelSearch(soups_per_haul, parallelisation, payoshaKey, seed, local_log);
        } else {
            quitByUser = runSearch(soups_per_haul, payoshaKey, seed, local_log, testing);
        }
        seed = reseed(seed);
        std::cout << "New seed: " << seed << "; iterations = " << iterations << "; quitByUser = " << quitByUser << std::endl;

        iterations -= 1;
        if (iterations == 0) { break; }
    }

    std::cout << "Terminating..." << std::endl;

    return 0;
}
