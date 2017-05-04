//
// Copyright (c) 2016-2017 by Cornell University.  All Rights Reserved.
//
// Permission to use the "TownCrier" software ("TownCrier"), officially
// docketed at the Center for Technology Licensing at Cornell University
// as D-7364, developed through research conducted at Cornell University,
// and its associated copyrights solely for educational, research and
// non-profit purposes without fee is hereby granted, provided that the
// user agrees as follows:
//
// The permission granted herein is solely for the purpose of compiling
// the TowCrier source code. No other rights to use TownCrier and its
// associated copyrights for any other purpose are granted herein,
// whether commercial or non-commercial.
//
// Those desiring to incorporate TownCrier software into commercial
// products or use TownCrier and its associated copyrights for commercial
// purposes must contact the Center for Technology Licensing at Cornell
// University at 395 Pine Tree Road, Suite 310, Ithaca, NY 14850; email:
// ctl-connect@cornell.edu; Tel: 607-254-4698; FAX: 607-254-5454 for a
// commercial license.
//
// IN NO EVENT SHALL CORNELL UNIVERSITY BE LIABLE TO ANY PARTY FOR
// DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES,
// INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF TOWNCRIER AND ITS
// ASSOCIATED COPYRIGHTS, EVEN IF CORNELL UNIVERSITY MAY HAVE BEEN
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// THE WORK PROVIDED HEREIN IS ON AN "AS IS" BASIS, AND CORNELL
// UNIVERSITY HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
// ENHANCEMENTS, OR MODIFICATIONS.  CORNELL UNIVERSITY MAKES NO
// REPRESENTATIONS AND EXTENDS NO WARRANTIES OF ANY KIND, EITHER IMPLIED
// OR EXPRESS, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, OR THAT THE USE
// OF TOWNCRIER AND ITS ASSOCIATED COPYRIGHTS WILL NOT INFRINGE ANY
// PATENT, TRADEMARK OR OTHER RIGHTS.
//
// TownCrier was developed with funding in part by the National Science
// Foundation (NSF grants CNS-1314857, CNS-1330599, CNS-1453634,
// CNS-1518765, CNS-1514261), a Packard Fellowship, a Sloan Fellowship,
// Google Faculty Research Awards, and a VMWare Research Award.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <Log.h>

#include "tls_client.h"
#include "scrapers.h"

static int construct_query(const char* symbol, char** buf) {
    int len;
    char query[1000];
    query[0] = 0;

    strncat(query, "/d/quotes.csv?s=", sizeof query);
    strncat(query, symbol, sizeof query);
    strncat(query, "&f=l1", sizeof query);
    strncat(query, " HTTP/1.1", sizeof query);

    len = strlen(query);
    *buf = (char*)malloc(len+1);
    memcpy(*buf, query, len);
    (*buf)[len] = 0;
    return len;
}


static double parse_response(const char* resp) {
    int len;
    len = strlen(resp);
    /// FIXME PLEASE ADD SOME ERROR HANDLING
    //if (len > 8)
    //{
    //    return 0.0;
    //}
    //else
    //{
    return std::strtod(resp, NULL);
    //}
}

int yahoo_current(const char* symbol, double* r) {

    if (symbol == NULL || r == NULL){
        LL_CRITICAL("Error: Passed in NULL pointer args");
        return -1;
    }

    int ret = 0;
    char* query = NULL;

    ret = construct_query(symbol, &query);
    if (ret < 0)
        return -1;

    vector<string> header;
    header.push_back("Host: download.finance.yahoo.com");
    HttpRequest httpRequest("download.finance.yahoo.com", query, header);
    HttpsClient httpClient(httpRequest);

    try {
        HttpResponse response = httpClient.getResponse();
        *r  =parse_response(response.getContent().c_str());
        return 0;
    }
    catch (std::runtime_error& e){
        LL_CRITICAL("Https error: %s", e.what());
        LL_CRITICAL("Details: %s", httpClient.getError().c_str());
        httpClient.close();
    }

    free(query);
    return 0;
}
