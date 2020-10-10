#pragma once

/*
 * This contains everything necessary for performing a soup search.
 */
class SoupSearcher {

public:

    std::map<std::string, int64_t> census;
    std::map<std::string, std::vector<std::string> > alloccur;
    uint64_t tilesProcessed = 0;

    SoupSearcher *parent = nullptr;

    explicit SoupSearcher() = default;
    explicit SoupSearcher(SoupSearcher *parent) : parent(parent) {}

    void aggregate(std::map<std::string, int64_t> *newcensus, std::map<std::string, std::vector<std::string> > *newoccur) {

        for (const auto& kv : *newcensus) {
            census[kv.first] += kv.second;
        }
        for (const auto& kv : *newoccur) {
            auto& all_occurrences_of_this_apgcode = alloccur[kv.first];
            for (const auto& occurrence : kv.second) {
                if (all_occurrences_of_this_apgcode.size() >= 10) {
                    break;
                }
                all_occurrences_of_this_apgcode.push_back(occurrence);
            }
        }

    }

    #ifdef STANDARD_LIFE

    void methudetect(UPATTERN &pat, apg::base_classifier<BITPLANES> &cfier, const std::string& seedroot, std::string suffix) {

        int fpop = pat.totalPopulation();

        #ifndef LARGE_SYMMETRY
        if (fpop >= 3000) {
            std::cerr << "Soup " << (seedroot + suffix) << " has a final population of \033[1;34m";
            std::cerr << fpop << "\033[0m cells." << std::endl;
            std::string apgcode = strConcat("megasized_", (fpop / 100), "h");
            census[apgcode] += 1;
            if (alloccur[apgcode].size() < 10) { alloccur[apgcode].push_back(suffix); }
        }
        #endif

        bool nonempty = pat.nonempty();
        #ifdef LARGE_SYMMETRY
        if (nonempty) return;
        #endif

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
                std::string apgcode = strConcat("messless_", (estgen / 100), "h");
                census[apgcode] += 1;
                if (alloccur[apgcode].size() < 10) { alloccur[apgcode].push_back(suffix); }
            }

            #ifndef LARGE_SYMMETRY
            if (estgen >= 25000) {
                std::string apgcode = strConcat("methuselah_", (estgen / 1000), "k");
                census[apgcode] += 1;
                if (alloccur[apgcode].size() < 10) { alloccur[apgcode].push_back(suffix); }
            }
            #endif
        }
    }

    #endif

    bool separate(UPATTERN &pat, int duration, int attempt, apg::base_classifier<BITPLANES> &cfier, const std::string& seedroot, const std::string& suffix) {

        bool proceedNonetheless = (attempt >= 5);
        std::map<std::string, int64_t> tally;
        cfier.gmax = (1024 << (attempt * 2));

        #ifdef HASHLIFE_ONLY

        tally = cfier.census(pat, duration, &classifyAperiodic);

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

        cfier.deeppurge(tally, icb, &classifyAperiodic, remove_annoyances, remove_gliders);

        apg::bitworld bwv0;
        icb.to_bitworld(bwv0, 0);

        int64_t n_gliders = bwv0.population() / 5;

        #ifndef HASHLIFE_ONLY
        n_gliders += pat.glider_count;
        #endif

        if (n_gliders > 0) {
            tally["xq4_153"] += n_gliders;
        }

        #else
        std::vector<apg::bitworld> bwv(BITPLANES + 1);
        pat.extractPattern(bwv);
        cfier.census(tally, bwv, &classifyAperiodic, true);
        #endif

        #endif

        cfier.gmax = 1048576;


        bool ignorePathologicals = false;
        int pathologicals = 0;

        for (const auto& kv : tally) {
            const auto& apgcode = kv.first;
            if (apgcode[0] == 'z') {
                #ifdef STANDARD_LIFE
                pathologicals += ((attempt <= 2) ? 1 : 0);
                #endif
            } else if (apgcode[0] == 'y') {
                ignorePathologicals = true;
            } else if (apgcode == "PATHOLOGICAL") {
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

        for (const auto& kv : tally) {
            const auto& apgcode = kv.first;
            if ((ignorePathologicals == false) || (apgcode != "PATHOLOGICAL")) {
                census[apgcode] += kv.second;
                if (alloccur[apgcode].empty() || alloccur[apgcode].back() != suffix) {
                    if ((suffix.length() < 1920) && (alloccur[apgcode].size() < 10)) {
                        alloccur[apgcode].push_back(suffix);
                    }
                }
            }

            if (census[apgcode] > 10) { continue; }

            if ((parent != nullptr) && (parent->census.count(apgcode)) && (parent->census[apgcode] > 10)) { continue; }

            #ifdef STANDARD_LIFE
            if ((apgcode[0] == 'x') && (apgcode[1] == 'p')) {
                if ((apgcode[2] != '2') || (apgcode[3] != '_')) {
                    if (apgcode != "xp3_co9nas0san9oczgoldlo0oldlogz1047210127401" && apgcode != "xp15_4r4z4r4") {
                        // Interesting oscillator:
                        std::cout << "Rare oscillator detected: \033[1;31m" << apgcode << "\033[0m" << std::endl;
                    }
                }
            } else if ((apgcode[0] == 'x') && (apgcode[1] == 'q')) {
                if (apgcode != "xq4_153" && apgcode != "xq4_6frc" && apgcode != "xq4_27dee6" && apgcode != "xq4_27deee6") {
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

    void censusSoup(const std::string& seedroot, const std::string& suffix, apg::base_classifier<BITPLANES> &cfier, std::vector<apg::bitworld> &vbw) {

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

    void censusSoup(const std::string& seedroot, const std::string& suffix, apg::base_classifier<BITPLANES> &cfier) {

        std::vector<apg::bitworld> vbw = apg::hashsoup(seedroot + suffix, SYMMETRY);
        censusSoup(seedroot, suffix, cfier, vbw);

    }

    std::vector<std::pair<int64_t, std::string>> getCensusListSortedByFrequency() const {

        std::vector<std::pair<int64_t, std::string>> result;
        for (const auto& kv : census) {
            if ((kv.second != 0) && (kv.first != "xs0_0")) {
                result.emplace_back(kv.second, kv.first);
            }
        }
        std::sort(result.begin(), result.end(), std::greater<std::pair<int64_t, std::string>>());
        return result;

    }

    std::string submitResults(const std::string& payoshakey, const std::string& root, uint64_t numsoups, int local_log, bool testing) {

        std::string authstring = "testing";

        if (!testing) {
            authstring = authenticate(payoshakey.c_str(), "post_apgsearch_haul");
        }

        // Authentication failed:
        if (authstring.empty()) {
            return "";
        }

        std::vector<std::pair<int64_t, std::string>> censusList = getCensusListSortedByFrequency();

        int64_t numObjects = 0;
        for (const auto& kv : censusList) {
            numObjects += kv.first;
        }

        std::ostringstream ss;

        ss << authstring << "\n";
        ss << "@VERSION " << APG_VERSION << "\n";
        ss << "@MD5 " << md5(root) << "\n";
        ss << "@ROOT " << root << "\n";
        ss << "@RULE " << RULESTRING << "\n";
        ss << "@SYMMETRY " << SYMMETRY << "\n";
        ss << "@NUM_SOUPS " << numsoups << "\n";
        ss << "@NUM_OBJECTS " << numObjects << "\n";

        ss << "\n@CENSUS TABLE\n";

        for (const auto& kv : censusList) {
            ss << kv.second << " " << kv.first << "\n";
        }

        ss << "\n@SAMPLE_SOUPIDS\n";

        for (const auto& kv : censusList) {
            const auto& occurrences = alloccur[kv.second];
            if (occurrences.empty()) { continue; }

            ss << kv.second;

            #ifdef STDIN_SYM
            ss << " " << occurrences[0];
            #else
            for (const std::string& s : occurrences) {
                ss << " " << s;
            }
            #endif

            ss << "\n";
        }

        if (local_log) {
            std::time_t timestamp = std::time(NULL);
            std::string resultsFileName = strConcat("log.", timestamp, ".", root, ".txt");

            std::cout << "Saving results to " << resultsFileName << std::endl;

            std::ofstream resultsFile;
            resultsFile.open(resultsFileName.c_str());
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
