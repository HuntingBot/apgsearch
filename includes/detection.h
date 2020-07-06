#pragma once

double regress(const std::vector<std::pair<double, double>>& pairlist) {

    double cumx = 0.0;
    double cumy = 0.0;
    double cumvar = 0.0;
    double cumcov = 0.0;

    for (const auto& xy : pairlist) {
        cumx += xy.first;
        cumy += xy.second;
    }
    cumx /= pairlist.size();
    cumy /= pairlist.size();

    for (const auto& xy : pairlist) {
        cumvar += (xy.first - cumx) * (xy.first - cumx);
        cumcov += (xy.first - cumx) * (xy.second - cumy);
    }

    return (cumcov / cumvar);

}

std::string powerlyse(const apg::pattern& ipat, int stepsize, int numsteps, int startgen) {

    apg::pattern pat = ipat;

    std::vector<std::pair<double, double> > pairlist;
    std::vector<std::pair<double, double> > pairlist2;
    double cumpop = 1.0;

    for (int i = 0; i < numsteps; i++) {
        pat = pat[stepsize];
        cumpop += pat.popcount((1 << 30) + 3);
        pairlist.push_back(std::make_pair(std::log(i*stepsize+startgen), std::log(cumpop)));
        pairlist2.push_back(std::make_pair(std::log(i+1), std::log(cumpop)));
    }

    double power = regress(pairlist);
    double power2 = regress(pairlist2);

    if (power2 < 1.15) {
        return "PATHOLOGICAL";
    } else if (power < 1.65) {
        return "zz_REPLICATOR";
    } else if (power < 2.1) {
        return "zz_LINEAR";
    } else if (power < 2.9) {
        return "zz_EXPLOSIVE";
    } else {
        return "zz_QUADRATIC";
    }

}

#ifdef HASHLIFE_ONLY

std::vector<int> get_popseq(const apg::pattern& ipat, int ngens, int stepsize) {

    std::vector<int> poplist(ngens);
    apg::pattern pat = ipat;
    for (int i = 0; i < ngens; i += stepsize) {
        poplist[i] = pat.popcount((1 << 30) + 3);
        pat = pat[stepsize];
    }
    return poplist;

}

#else

void pat2vec(apg::pattern pat, UPATTERN &upat) {

    std::vector<apg::bitworld> vbw;
    for (int i = 0; i < BITPLANES; i++) {
        vbw.push_back(pat.flatlayer(i));
    }
    upat.insertPattern(vbw);

}

std::vector<int> get_popseq(apg::pattern ipat, int ngens, int stepsize) {

    UPATTERN pat0;
    UPATTERN pat1;

    pat2vec(ipat, pat0);
    pat2vec(ipat[stepsize], pat1);

    std::vector<int> poplist(ngens);

    for (int i = 0; i < ngens; i += stepsize) {
        if ((i / stepsize) % 2) {
            poplist[i] = pat1.totalPopulation();
            pat1.advance(0, 0, 2*stepsize);
        } else {
            poplist[i] = pat0.totalPopulation();
            pat0.advance(0, 0, 2*stepsize);
        }
    }

    return poplist;

}

#endif

std::string linearlyse(const apg::pattern& ipat, int maxperiod, int stepsize) {

    std::vector<int> poplist = get_popseq(ipat, 3 * maxperiod, stepsize);
    std::vector<int> difflist(2 * maxperiod);

    int period = -1;

    for (int p = 1; p < maxperiod; p++) {
        for (int i = 0; i < 2 * maxperiod; i++) {
            difflist[i] = poplist[i + p] - poplist[i];
        }

        bool correct = true;

        for (int i = 0; i < maxperiod; i++) {
            if (difflist[i] != difflist[i + p]) {
                correct = false;
            }
        }

        if (correct == true) {
            period = p;
            break;
        }
    }

    if (period == -1)
        return "PATHOLOGICAL";

    int qeriod = -1;

    for (int q = 1; q < maxperiod; q++) {
        bool correct = true;

        for (int i = 0; i < maxperiod; i++) {
            if (difflist[i] != difflist[i + q]) {
                correct = false;
            }
        }

        if (correct == true) {
            qeriod = q;
            break;
        }
    }

    if (qeriod == -1)
    {
        // std::cout << "Something is seriously wrong!" << std::endl;
        return "PATHOLOGICAL";
    }

    int moment0 = 0;
    int moment1 = 0;
    int moment2 = 0;

    for (int i = 0; i < period; i++) {

        int instadiff = (poplist[i + qeriod] - poplist[i]);

        moment0 += instadiff;
        moment1 += (instadiff * instadiff);
        moment2 += (instadiff * instadiff * instadiff);

    }

    if (moment0 == 0)
        return "PATHOLOGICAL";

    std::string posthash = md5(strConcat(moment1, '#', moment2));
    std::string repr = strConcat("yl", period, "_", qeriod, "_", moment0, "_", posthash);

    // std::cout << "Linear-growth pattern identified: \033[1;32m" << repr << "\033[0m" << std::endl;

    return repr;

}

/*
 * Get the representation of a single object:
 */
std::string classifyAperiodic(apg::pattern pat) {

    uint64_t vm = apg::uli_valid_mantissa(apg::rule2int(pat.getrule()));
    int lss = __builtin_ctzll(vm - 1);
    std::string repr = linearlyse(pat, 4800, lss);
    if (repr[0] != 'y') {
        repr = powerlyse(pat, 128, 8000, 4800);
    }

    return repr;

}

