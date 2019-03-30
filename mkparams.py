#!/usr/bin/python

import sys
import re

from lifelib.genera import rule_property
from lifelib.autocompile import set_rules, sanirule
from lifelib.pythlib.samples import validate_symmetry

def main():

    if (len(sys.argv) < 3):
        print("Usage:")
        print("python mkparams.py b3s23 C1")
        exit(1)

    rulestring = sys.argv[1]
    symmetry = sys.argv[2]

    using_gpu = (len(sys.argv) > 3) and (sys.argv[3] == 'true')

    # Convert rulestrings such as 'B3/S23' into 'b3s23':
    newrule = sanirule(rulestring)
    if newrule != rulestring:
        print("Warning: \033[1;31m" + rulestring + "\033[0m interpreted as \033[1;32m" + newrule + "\033[0m")
        rulestring = newrule

    validate_symmetry(rulestring, symmetry)

    print("Valid symmetry: \033[1;32m"+symmetry+"\033[0m")

    set_rules(rulestring)

    m = re.match('b1?2?3?4?5?6?7?8?s0?1?2?3?4?5?6?7?8?$', rulestring)

    bitplanes = rule_property(rulestring, 'bitplanes')
    family = rule_property(rulestring, 'family')

    if m is None:
        # Arbitrary rules should use the Universal Leaf Iterator:
        upattern = "apg::upattern<apg::UTile<BITPLANES + 1, BITPLANES>, 16>"
    else:
        # Special speedup for life-like rules to ensure comparable performance to v3.x:
        upattern = "apg::upattern<apg::VTile28, 28, 28>"

    with open('includes/params.h', 'w') as g:

        g.write('#define PYTHON_VERSION "%s"\n' % repr(sys.version.replace('\n', ' ')))
        g.write('#define BITPLANES %d\n' % bitplanes)
        g.write('#define SYMMETRY "%s"\n' % symmetry)
        g.write('#define RULESTRING "%s"\n' % rulestring)
        g.write('#define CLASSIFIER apg::base_classifier<BITPLANES>\n')

        if symmetry in ['C1', 'G1']:
            g.write('#define C1_SYMMETRY 1\n')
        elif 'stdin' in symmetry:
            g.write('#define STDIN_SYM 1\n')

        if using_gpu:
            g.write('#define USING_GPU 1\n')

        if (family >= 6):
            g.write('#define HASHLIFE_ONLY 1\n')
            g.write('#define UPATTERN apg::pattern\n')
        elif (rulestring == 'b3s23'):
            g.write('#define STANDARD_LIFE 1\n')
            g.write('#ifdef __AVX512F__\n')
            g.write('#define UPATTERN apg::upattern<apg::VTile44, 28, 44>\n')
            g.write('#define INCUBATOR apg::incubator<56, 88>\n')
            g.write('#else\n')
            g.write('#define UPATTERN apg::upattern<apg::VTile28, 28, 28>\n')
            g.write('#define INCUBATOR apg::incubator<56, 56>\n')
            g.write('#endif\n')
        else:
            g.write("#define UPATTERN %s\n" % upattern)
            if 'VTile28' in upattern:
                g.write('#define INCUBATOR apg::incubator<56, 56>\n')

        if (re.match('b36?7?8?s0?235?6?7?8?$', rulestring)):
            g.write('#define GLIDERS_EXIST 1\n')
        else:
            g.write('#define DISABLE_GLIDERS 1\n')


main()

print("Success!")
