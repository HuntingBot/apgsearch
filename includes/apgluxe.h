
int run_apgluxe(int argc, char *argv[]) {

    /*
    {
    auto tx = apg::sign_message("first_message", "first_password");
    std::cout << "Signed transaction length: " << tx.size() << std::endl;
    auto ty = apg::unsign_message(tx);
    std::cout << ty.first << " " << ty.second << " " << apg::verify_crc32(ty.second) << std::endl;
    }
    {
    auto tx = apg::sign_message("second_message", "first_password");
    std::cout << "Signed transaction length: " << tx.size() << std::endl;
    auto ty = apg::unsign_message(tx);
    std::cout << ty.first << " " << ty.second << " " << apg::verify_crc32(ty.second) << std::endl;
    }
    {
    auto tx = apg::sign_message("third_message", "second_password");
    std::cout << "Signed transaction length: " << tx.size() << std::endl;
    auto ty = apg::unsign_message(tx);
    std::cout << ty.first << " " << ty.second << " " << apg::verify_crc32(ty.second) << std::endl;
    }
    {
    auto tx = apg::sign_message("fourth_message", "second_password");
    std::cout << "Signed transaction length: " << tx.size() << std::endl;
    auto ty = apg::unsign_message(tx);
    std::cout << ty.first << " " << ty.second << " " << apg::verify_crc32(ty.second) << std::endl;
    }

    return 0;
    */

    // Default values:
    int soups_per_haul = 10000000;
    std::string payoshaKey = "#anon";
    std::string seed = reseed("original seed");
    int verifications = -1;
    int parallelisation = 0;
    int local_log = 0;
    bool testing = false;
    int nullargs = 1;
    bool quitByUser = false;
    struct termios ttystate;
    
    // Extract options:
    for (int i = 1; i < argc - 1; i++) {
        if (strcmp(argv[i], "-k") == 0) {
            payoshaKey = argv[i+1];
        } else if (strcmp(argv[i], "-s") == 0) {
            seed = argv[i+1];
        } else if (strcmp(argv[i], "-n") == 0) {
            soups_per_haul = atoi(argv[i+1]);
        } else if (strcmp(argv[i], "-v") == 0) {
            verifications = atoi(argv[i+1]);
        } else if (strcmp(argv[i], "-L") == 0) {
            local_log = atoi(argv[i+1]);
        } else if (strcmp(argv[i], "-t") == 0) {
            testing = true;
        } else if (strcmp(argv[i], "-p") == 0) {
            #ifndef USE_OPEN_MP
            std::cout << "\033[1;31mWarning: apgluxe has not been compiled with OpenMP support.\033[0m" << std::endl;
            #endif
            parallelisation = atoi(argv[i+1]);
        } else if (strcmp(argv[i], "--rule") == 0) {
            std::cout << "\033[1;33mapgluxe " << APG_VERSION << "\033[0m: ";
            std::string desired_rulestring = argv[i+1];
            if (strcmp(RULESTRING, argv[i+1]) == 0) {
                std::cout << "Rule \033[1;34m" << RULESTRING << "\033[0m is correctly configured." << std::endl;
                nullargs += 2;
            } else {
                std::cout << "Rule \033[1;34m" << RULESTRING << "\033[0m does not match desired rule \033[1;34m";
                std::cout << desired_rulestring << "\033[0m." << std::endl;
                execvp("./recompile.sh", argv);
                return 1;
            }
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

    // Disable verification by default if running on a HPC;
    // otherwise verify three hauls per uploaded haul:
    if (verifications < 0) {
        verifications = (parallelisation <= 4) ? 3 : 0;
    }
    
    // turn on non-blocking reads
    tcgetattr(STDIN_FILENO, &ttystate);
    ttystate.c_lflag &= ~ICANON;
    ttystate.c_cc[VMIN] = 1;
    tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);

    std::cout << "\nGreetings, this is \033[1;33mapgluxe " << APG_VERSION;
    std::cout << "\033[0m, configured for \033[1;34m" << RULESTRING << "/";
    std::cout << SYMMETRY << "\033[0m.\n" << std::endl;

    std::cout << "\033[32;1mLifelib version:\033[0m " << LIFELIB_VERSION << std::endl;
    std::cout << "\033[32;1mCompiler version:\033[0m " << __VERSION__ << std::endl;
    std::cout << "\033[32;1mPython version:\033[0m " << PYTHON_VERSION << std::endl;

    std::cout << std::endl;


    #ifdef LIFECOIN

    {
    SoupSearcher soup;
    apg::lifetree<uint32_t, BITPLANES> lt(LIFETREE_MEM);
    apg::base_classifier<BITPLANES> cfier(&lt, RULESTRING);
    int64_t coediff = soup.censusSoup("n_CvtBYspKyzAx19112245", "", cfier);
    std::cout << "Coe ship difficulty: " << coediff << std::endl;
    }

    #endif

    while (!quitByUser) {
        if (verifications > 0) {
            std::cout << "Peer-reviewing hauls:\n" << std::endl;
            // Verify some hauls:
            for (int j = 0; j < verifications; j++) {
                verifySearch(payoshaKey);
            }
            std::cout << "\nPeer-review complete; proceeding search.\n" << std::endl;
        }

        // Run the search:
        std::cout << "Using seed " << seed << std::endl;
        if (parallelisation > 0) {
            #ifdef USE_OPEN_MP
            quitByUser = parallelSearch(soups_per_haul, parallelisation, payoshaKey, seed, local_log);
            #else
            quitByUser = runSearch(soups_per_haul, payoshaKey, seed, local_log, false);
            #endif
        } else {
            quitByUser = runSearch(soups_per_haul, payoshaKey, seed, local_log, testing);
        }
        seed = reseed(seed);

        if (testing) { break; }
    }

    // turn on blocking reads
    tcgetattr(STDIN_FILENO, &ttystate);
    ttystate.c_lflag |= ICANON;
    tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);

    return quitByUser ? 1 : 0;
}