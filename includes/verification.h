#pragma once

std::string obtainWork(const std::string& payoshakey) {

    std::string authstring = authenticate(payoshakey.c_str(), "verify_apgsearch_haul");

    // Authentication failed:
    if (authstring.length() == 0) {
        std::cout << "Authentication failed." << std::endl;
        return "";
    }

    std::string payload = strConcat(authstring, RULESTRING, "\n", SYMMETRY, "\n");
    return catagolueRequest(payload.c_str(), "/verify");

}

bool verifySearch(const std::string& payoshakey) {

    std::string response = obtainWork(payoshakey);

    if (response.size() <= 3) {
        std::cout << "Received no response from /verify." << std::endl;
        return 1;
    }

    std::stringstream iss(response);
    std::vector<std::string> stringlist;

    std::string sub;
    while (std::getline(iss, sub, '\n')) {
        stringlist.push_back(sub);
        // std::cout << sub << std::endl;
    }

    if (stringlist.size() < 4) {
        std::cout << "No more hauls to verify." << std::endl;
        return 1;
    }

    std::string authstring = authenticate(payoshakey.c_str(), "submit_verification");

    // Authentication failed:
    if (authstring.empty()) {
        std::cout << "Authentication failed." << std::endl;
        return 1;
    }

    std::ostringstream ss;
    ss << authstring << "\n";
    ss << "@MD5 " << stringlist[2] << "\n";
    ss << "@PASSCODE " << stringlist[3] << "\n";
    ss << "@RULE " << RULESTRING << "\n";
    ss << "@SYMMETRY " << SYMMETRY << "\n";

    SoupSearcher soup;
    apg::lifetree<uint32_t, BITPLANES> lt(LIFETREE_MEM);
    apg::base_classifier<BITPLANES> cfier(&lt, RULESTRING);

    for (unsigned int i = 4; i < stringlist.size(); i++) {
        std::string symslash = SYMMETRY "/";
        std::string seed = stringlist[i];
        if ((seed.size() >= 4) && (seed.substr(0,symslash.length()).compare(symslash) == 0)) {
            soup.censusSoup(seed.substr(symslash.size()), "", cfier);
        } else {
            std::cout << "[" << seed << "]" << std::endl;
        }
    }

    for (const auto& kv : soup.getCensusListSortedByFrequency()) {
        ss << kv.second << " " << kv.first << "\n";
    }

    catagolueRequest(ss.str().c_str(), "/verify");

    return 0;

}
