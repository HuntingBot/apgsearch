#pragma once

#include <fstream>
#include <algorithm>
#include <cstdlib>

#include "../lifelib/upattern.h"
#include "../lifelib/classifier.h"
#include "../lifelib/incubator.h"

#ifndef PREFIX_VERSION
#define PREFIX_VERSION ""
#endif

#define APG_VERSION PREFIX_VERSION "v5.19-" LIFELIB_VERSION

#include "utilities.h"
#include "params2.h"
#include "md5.h"
#include "payosha256.h"
#include "hashsoup2.h"

#define LIFETREE_MEM 100

#include "detection.h"
#include "stabilise.h"
#include "searcher.h"
