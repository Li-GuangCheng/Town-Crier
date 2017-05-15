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

#include "encoding.h"

#include <string>

#include "commons.h"
#include "debug.h"

using std::vector;
using std::string;

uint8_t get_n_th_byte(uint64_t in, int n) {
  if (n > 8) {
    printf_sgx("n is too big\n");
    return 0xFF;
  }
  return (in >> (8 * n)) & 0xff;
}

/*!
 * encode in in big-endian order. Prepend zeros to make the result of 32-byte aligned.
 * @param out
 * @param in input int
 * @param len length of int in byte
 * @return
 */
int append_as_uint256(bytes &out, uint64_t in, int len) {
  if (len > 32) {
    printf_sgx("Error: too big\n");
    return -1;
  }
  // padding with 0
  for (int i = 0; i < 32 - len; i++) { out.push_back(0); }
  // push big-endian int
  for (int i = len - 1; i >= 0; i--) { out.push_back(get_n_th_byte(in, i)); }
  return 0;
}

// compute how many (non-zero) bytes there are in _i
template<typename T>
static uint8_t byte_length(T _i) {
  uint8_t i = 0;
  for (; _i != 0; ++i, _i >>= 8) {}
  return i;
}

uint8_t bytesRequired(uint64_t _i) { return byte_length<uint64_t>(_i); }

bytes::bytes(bytes a, bytes b) {
  std::vector<uint8_t>::insert(std::vector<uint8_t>::end(), a.begin(), a.end());
  std::vector<uint8_t>::insert(std::vector<uint8_t>::end(), b.begin(), b.end());
}

void bytes::replace(const bytes &in) {
  vector<uint8_t>::clear();
  vector<uint8_t>::insert(vector<uint8_t>::end(), in.begin(), in.end());
}

void bytes::from_hex(const char *src) {
  if (strlen(src) % 2 != 0) { LL_CRITICAL("Error: input is not of even len\n"); }
  if (strncmp(src, "0x", 2) == 0) src += 2;
  while (*src && src[1]) {
    std::vector<uint8_t>::push_back(hex2int(*src) * 16 + hex2int(src[1]));
    src += 2;
  }
}

void bytes::rlp(bytes &out, size_t len) {
  int32_t i;
  int32_t len_len;
  if (len == 1 && (std::vector<uint8_t>::operator[](0)) < 0x80) {
    out.push_back(std::vector<uint8_t>::operator[](0));
    return;
  }
  // longer than 1
  if (len < 56) {
    out.push_back(0x80 + static_cast<uint8_t>(len));
    for (i = 0; i < len; i++) out.push_back(std::vector<uint8_t>::operator[](i));
  } else {
    len_len = byte_length<size_t>(len);
    if (len_len > 8) { throw std::invalid_argument("Error: len_len > 8"); }
    out.push_back(0xb7 + static_cast<uint8_t>(len_len));

    vector<uint8_t> b_len = itob(len);
    out.insert(out.end(), b_len.begin(), b_len.end());
    out.insert(out.end(), vector<uint8_t>::begin(), vector<uint8_t>::begin() + len);
  }
}

void bytes::to_rlp(bytes &out) {
  return rlp(out, std::vector<uint8_t>::size());
}

void bytes::toString(const string &title) {
#ifdef DEBUG
  int debugging;
  ocall_is_debug(&debugging);
  if (debugging) {
    hexdump(title.c_str(), &std::vector<uint8_t>::operator[](0),
            std::vector<uint8_t>::size());
  }
#endif
}

void bytes::toString() { toString("bytes"); }

bytes32::bytes32(uint64_t in) {
  // push big-endian int (i.e. prepend 0 until 32 bytes)
  BYTE b_in = itob(in, 32);
  vector::insert(vector::end(), b_in.begin(), b_in.end());
}

bytes32::bytes32(std::string in) {
  if (in.length() > 32) {
    throw std::invalid_argument("too big");
  }
  // push string
  vector::insert(vector::end(), in.begin(), in.end());
  // rear padding with 0
  vector::insert(vector::end(), 32 - in.length(), 0);
}

void bytes32::replace(const BYTE &in) {
  if (in.size() > 32) {
    throw std::invalid_argument("too large");
  }

  vector<uint8_t>::clear();
  vector<uint8_t>::insert(vector<uint8_t>::end(), in.begin(), in.end());
}

std::vector<uint8_t> itob(uint64_t num, size_t width) {
  std::vector<uint8_t> out;
  size_t len_len = byte_length<size_t>(num);
  for (long i = len_len - 1; i >= 0; i--) {
    out.push_back(static_cast<uint8_t>((num >> (8 * i)) & 0xFF));
  }

  // prepend zero until width
  if (width > out.size()) {
    out.insert(out.begin(), width - out.size(), 0x0);
  }

  return out;
}
