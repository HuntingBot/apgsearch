#pragma once

/*
 * This contains everything necessary for performing a soup search.
 */
class SoupSearcher {

public:

    std::map<std::string, long long> census;
    std::map<std::string, std::vector<std::string> > alloccur;
    uint64_t tilesProcessed;

    void aggregate(std::map<std::string, long long> *newcensus, std::map<std::string, std::vector<std::string> > *newoccur) {

        std::map<std::string, long long>::iterator it;
        for (it = newcensus->begin(); it != newcensus->end(); it++)
        {
            std::string apgcode = it->first;
            long long quantity = it->second;
            census[apgcode] += quantity;

        }

        std::map<std::string, std::vector<std::string> >::iterator it2;
        for (it2 = newoccur->begin(); it2 != newoccur->end(); it2++)
        {
            std::string apgcode = it2->first;
            std::vector<std::string> occurrences = it2->second;
            for (unsigned int i = 0; i < occurrences.size(); i++) {
                if (alloccur[apgcode].size() < 10) {
                    alloccur[apgcode].push_back(occurrences[i]);
                }
            }
        }
    }

    #ifdef STANDARD_LIFE

    void methudetect(UPATTERN &pat, apg::base_classifier<BITPLANES> &cfier, std::string seedroot, std::string suffix) {

        int fpop = pat.totalPopulation();

        if (fpop >= 3000) {
            std::cerr << "Soup " << (seedroot + suffix) << " has a final population of \033[1;34m";
            std::cerr << fpop << "\033[0m cells." << std::endl;
            std::ostringstream ss;
            ss << "megasized_" << (fpop / 100) << "h";
            std::string apgcode = ss.str();
            census[apgcode] += 1;
            alloccur[apgcode].push_back(suffix);
        }

        bool nonempty = pat.nonempty();

        int estgen = pat.gensElapsed;
        if (estgen >= (nonempty ? 24000 : 500)) {
            std::cerr << "Soup " << (seedroot + suffix) << " lasts an estimated \033[1;34m";
            std::cerr << estgen << "\033[0m generations; rerunning..." << std::endl;

            estgen = 0;

            std::vector<apg::bitworld> vbw = apg::hashsoup(seedroot + suffix, SYMMETRY);
            apg::pattern origsoup(cfier.lab, cfier.lab->fromplanes(vbw), RULESTRING);

            auto popseq = get_popseq(origsoup, pat.gensElapsed + 8000, 1);

            for (uint64_t p = 1; p < 4000; p++) {
                bool period_found = true;
                for (uint64_t i = pat.gensElapsed; i < pat.gensElapsed + 8000; i++) {
                    if (popseq[i] != popseq[i - p]) { period_found = false; break; }
                }
                if (!period_found) { continue; }
                for (uint64_t i = pat.gensElapsed + 8000 - (p + 1); i > 0; i--) {
                    if (popseq[i] != popseq[i + p]) {
                        estgen = i + 1;
                        std::cerr << "Soup " << (seedroot + suffix) << " actually lasts \033[1;34m";
                        std::cerr << estgen << "\033[0m generations." << std::endl;
                        break;
                    }
                }
                if (period_found) { break; }
            }

            if ((!nonempty) && (estgen >= 500)) {
                std::ostringstream ss;
                ss << "messless_" << (estgen / 100) << "h";
                std::string apgcode = ss.str();
                census[apgcode] += 1;
                alloccur[apgcode].push_back(suffix);
            }

            if (estgen >= 25000) {
                std::ostringstream ss;
                ss << "methuselah_" << (estgen / 1000) << "k";
                std::string apgcode = ss.str();
                census[apgcode] += 1;
                alloccur[apgcode].push_back(suffix);
            }
        }
    }

    #endif

    bool separate(UPATTERN &pat, int duration, int attempt, apg::base_classifier<BITPLANES> &cfier, std::string seedroot, std::string suffix) {

        bool proceedNonetheless = (attempt >= 5);
        std::map<std::string, int64_t> cm;
        cfier.gmax = (1024 << (attempt * 2));

        #ifdef HASHLIFE_ONLY

        cm = cfier.census(pat, duration, &classifyAperiodic);

        #else

        pat.decache();
        pat.advance(0, 1, duration);

        #ifdef INCUBATOR
        INCUBATOR icb;
        apg::copycells(&pat, &icb);

        #ifdef GLIDERS_EXIST
        bool remove_gliders = true;
        #else
        bool remove_gliders = false;
        #endif

        #ifdef STANDARD_LIFE
        bool remove_annoyances = true;
        #else
        bool remove_annoyances = false;
        #endif

        cfier.deeppurge(cm, icb, &classifyAperiodic, remove_annoyances, remove_gliders);

        apg::bitworld bwv0;
        icb.to_bitworld(bwv0, 0);

        int64_t n_gliders = bwv0.population() / 5;

        #ifndef HASHLIFE_ONLY
        n_gliders += pat.glider_count;
        #endif

        if (n_gliders > 0) {
            cm["xq4_153"] += n_gliders;
        }

        #else
        std::vector<apg::bitworld> bwv(BITPLANES + 1);
        pat.extractPattern(bwv);
        cfier.census(cm, bwv, &classifyAperiodic, true);
        #endif

        #endif

        cfier.gmax = 1048576;


        bool ignorePathologicals = false;
        int pathologicals = 0;

        for (auto it = cm.begin(); it != cm.end(); ++it) {
            if (it->first[0] == 'z') {
                #ifdef STANDARD_LIFE
                pathologicals += ((attempt <= 2) ? 1 : 0);
                #endif
            } else if (it->first[0] == 'y') {
                ignorePathologicals = true;
            } else if (it->first == "PATHOLOGICAL") {
                pathologicals += 1;
            }
        }

        if (pathologicals > 0) {
            if (proceedNonetheless) {
                if (ignorePathologicals == false) { std::cout << "Pathological object detected!!!" << std::endl; }
            } else {
                return true;
            }
        }

        for (auto it = cm.begin(); it != cm.end(); ++it) {
            std::string apgcode = it->first;
            if ((ignorePathologicals == false) || (apgcode.compare("PATHOLOGICAL") != 0)) {
                census[apgcode] += it->second;
                if (alloccur[apgcode].size() == 0 || alloccur[apgcode].back().compare(suffix) != 0) {
                    if ((suffix.length() < 1920) && (alloccur[apgcode].size() < 10)) {
                        alloccur[apgcode].push_back(suffix);
                    }
                }
            }

            if (census[apgcode] > 10) { continue; }

            #ifdef STANDARD_LIFE
            if ((apgcode[0] == 'x') && (apgcode[1] == 'p')) {
                if ((apgcode[2] != '2') || (apgcode[3] != '_')) {
                    if (apgcode.compare("xp3_co9nas0san9oczgoldlo0oldlogz1047210127401") != 0 && apgcode.compare("xp15_4r4z4r4") != 0) {
                        // Interesting oscillator:
                        std::cout << "Rare oscillator detected: \033[1;31m" << apgcode << "\033[0m" << std::endl;
                    }
                }
            } else if ((apgcode[0] == 'x') && (apgcode[1] == 'q')) {
                if (apgcode.compare("xq4_153") != 0 && apgcode.compare("xq4_6frc") != 0 && apgcode.compare("xq4_27dee6") != 0 && apgcode.compare("xq4_27deee6") != 0) {
                    std::cout << "Rare spaceship detected: \033[1;34m" << apgcode << "\033[0m" << std::endl;
                }
            } else if ((apgcode[0] == 'y') && (apgcode[1] == 'l')) {
                std::cout << "Linear-growth pattern detected: \033[1;32m" << apgcode << "\033[0m" << std::endl;
            } else if ((apgcode[0] == 'z') && (apgcode[1] == 'z')) {
                std::cout << "Chaotic-growth pattern detected: \033[1;32m" << apgcode << "\033[0m" << std::endl;
                ignorePathologicals = true;
            }
            #else
            if ((apgcode[0] == 'y') && (apgcode[1] == 'l')) {
                std::cout << "Linear-growth pattern detected: \033[1;32m" << apgcode << "\033[0m" << std::endl;
            } else if ((apgcode[0] == 'z') && (apgcode[1] == 'z')) {
                std::cout << "Chaotic-growth pattern detected: \033[1;32m" << apgcode << "\033[0m" << std::endl;
            }
            #endif


        }

        #ifdef STANDARD_LIFE
        if ((attempt == 0) && (ignorePathologicals == false)) {
            methudetect(pat, cfier, seedroot, suffix);
        }
        #else
        (void) seedroot;
        #endif

        return false;

    }

    void censusSoup(std::string seedroot, std::string suffix, apg::base_classifier<BITPLANES> &cfier) {

        std::vector<apg::bitworld> vbw = apg::hashsoup(seedroot + suffix, SYMMETRY);

        #ifdef HASHLIFE_ONLY
        apg::pattern pat(cfier.lab, cfier.lab->fromplanes(vbw), RULESTRING);
        #else
        UPATTERN pat;
        pat.insertPattern(vbw);
        #ifdef GLIDERS_EXIST
        pat.extremal_mask = 15;
        #endif
        #endif

        int duration = stabilise3(pat);

        bool failure = true;
        int attempt = 0;

        // Repeat until there are no pathological objects, or until five attempts have elapsed:
        int step = 120;
        while (failure) {

            failure = false;

            if (pat.nonempty()) {

                failure = separate(pat, duration, attempt, cfier, seedroot, suffix);

            #ifdef STANDARD_LIFE
            } else {
                methudetect(pat, cfier, seedroot, suffix);
            #endif
            }

            // Pathological object detected:
            if (failure) {
                attempt += 1;
                #ifdef HASHLIFE_ONLY
                pat = pat[step];
                #else
                pat.clearHistory();
                pat.decache();
                pat.advance(0, 0, step);
                #endif
                step *= 4;
                duration = step * 2;
                if (duration > 7680) { duration = 7680; }
            }
        }

        #ifndef HASHLIFE_ONLY
        tilesProcessed += pat.tilesProcessed;
        #endif

    }


    std::vector<std::pair<long long, std::string> > getSortedList(long long &totobjs) {

        std::vector<std::pair<long long, std::string> > censusList;

        std::map<std::string, long long>::iterator it;
        for (it = census.begin(); it != census.end(); it++)
        {
            if ((it->second != 0) && (it->first != "xs0_0")) {
                censusList.push_back(std::make_pair(it->second, it->first));
                totobjs += it->second;
            }
        }
        std::sort(censusList.begin(), censusList.end());

        return censusList;

    }

    std::string submitResults(std::string payoshakey, std::string root, long long numsoups, int local_log, bool testing) {

        std::string authstring = "testing";

        if (!testing) {
            authstring = authenticate(payoshakey.c_str(), "post_apgsearch_haul");
        }

        // Authentication failed:
        if (authstring.length() == 0)
            return "";

        long long totobjs = 0;

        std::vector<std::pair<long long, std::string> > censusList = getSortedList(totobjs);

        std::ostringstream ss;

        ss << authstring << "\n";
        ss << "@VERSION " << APG_VERSION << "\n";
        ss << "@MD5 " << md5(root) << "\n";
        ss << "@ROOT " << root << "\n";
        ss << "@RULE " << RULESTRING << "\n";
        ss << "@SYMMETRY " << SYMMETRY << "\n";
        ss << "@NUM_SOUPS " << numsoups << "\n";
        ss << "@NUM_OBJECTS " << totobjs << "\n";

        ss << "\n@CENSUS TABLE\n";

        for (int i = censusList.size() - 1; i >= 0; i--) {
            ss << censusList[i].second << " " << censusList[i].first << "\n";
        }

        ss << "\n@SAMPLE_SOUPIDS\n";

        for (int i = censusList.size() - 1; i >= 0; i--) {
            std::vector<std::string> occurrences = alloccur[censusList[i].second];
            if (occurrences.size() == 0) { continue; }

            ss << censusList[i].second;

            #ifdef STDIN_SYM
            ss << " " << occurrences[0];
            #else
            for (unsigned int j = 0; j < occurrences.size(); j++) {
                ss << " " << occurrences[j];
            }
            #endif

            ss << "\n";
        }

        if(local_log) {
            std::ofstream resultsFile;
            std::ostringstream resultsFileName;

            std::time_t timestamp = std::time(NULL);

            resultsFileName << "log." << timestamp << "." << root << ".txt";

            std::cout << "Saving results to " << resultsFileName.str() << std::endl;

            resultsFile.open(resultsFileName.str().c_str());
            resultsFile << ss.str();
            resultsFile.close();
        }

        if (testing) { return "testing"; }

        std::string x = catagolueRequest(ss.str().c_str(), "/apgsearch");
        if (x.length() < 90) {
            std::cerr << "\033[32;1m" << x << "\033[0m" << std::endl;
        } else {
            std::cerr << "\033[31;1m" << x << "\033[0m" << std::endl;
        }
        return x;

    }

};
