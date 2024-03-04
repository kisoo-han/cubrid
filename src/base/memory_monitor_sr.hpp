/*
 *
 * Copyright 2016 CUBRID Corporation
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */

/*
 * memory_monitor_sr.hpp - Declaration of APIs and structures, classes
 *                         for memory monitoring module
 */

#ifndef _MEMORY_MONITOR_SR_HPP_
#define _MEMORY_MONITOR_SR_HPP_

#include <unordered_map>
#include <shared_mutex>
#include <mutex>
#include <string>
#include <atomic>

#include "concurrent_unordered_map.h"
#include "memory_monitor_common.hpp"

// IMPORTANT!!
// This meta size is related with allocation byte align
// Don't adjust it freely
// 4 byte tag + 8 byte size + 4 byte line + 4 byte checksum + 12 byte padding
#define MMON_ALLOC_META_SIZE 16

namespace cubmem
{
  class mmon_stat
  {
    public:
      mmon_stat (uint64_t size, uint64_t atomic_size);
      mmon_stat (uint64_t size);

      mmon_stat (const mmon_stat &rhs);
      mmon_stat &operator = (const mmon_stat &rhs);

      mmon_stat (mmon_stat &&) = delete;
      mmon_stat &operator = (mmon_stat &&) = delete;

      ~mmon_stat() = default;

    public:
      uint64_t temp_stat;
      std::atomic<uint64_t> stat;
  };

  class memory_monitor
  {
    public:
      memory_monitor (const char *server_name);
      ~memory_monitor () {}
      size_t get_alloc_size (char *ptr);
      void add_stat (char *ptr, size_t size, const char *file, const int line);
      void sub_stat (char *ptr);
      void aggregate_server_info (MMON_SERVER_INFO &server_info);
      void finalize_dump ();

    private:
      std::string make_tag_name (const char *file, const int line);
      int generate_checksum (int tag_id, uint64_t size);

    private:
      tbb::concurrent_unordered_map<std::string, int> m_tag_map; // tag name <-> tag id
      tbb::concurrent_unordered_map<int, mmon_stat> m_stat_map; // tag id <-> memory usage
      std::string m_server_name;
      std::atomic<uint64_t> m_total_mem_usage;
      std::atomic<int> m_meta_alloc_count;
      const int m_magic_number;
  };
} //namespace cubmem

bool mmon_is_mem_tracked ();
int mmon_initialize (const char *server_name);
void mmon_finalize ();
size_t mmon_get_alloc_size (char *ptr);
void mmon_add_stat (char *ptr, size_t size, const char *file, const int line);
void mmon_sub_stat (char *ptr);
void mmon_aggregate_server_info (MMON_SERVER_INFO &server_info);
#endif // _MEMORY_MONITOR_SR_HPP_
