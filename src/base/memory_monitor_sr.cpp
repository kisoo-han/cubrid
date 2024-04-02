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
 * memory_monitor_sr.cpp - Implementation of memory monitor module
 */

#include <stdio.h>
#include <malloc.h>
#include <cstring>
#include <algorithm>

#include "error_manager.h"
#include "system_parameter.h"
#include "memory_monitor_sr.hpp"

#ifndef HAVE_USR_INCLUDE_MALLOC_H
#define HAVE_USR_INCLUDE_MALLOC_H
#endif

typedef struct mmon_metainfo   // 16 bytes
{
  uint64_t size;
  int tag_id;
  int magic_number;
} MMON_METAINFO;

bool is_mem_tracked = false;

namespace cubmem
{
  memory_monitor *mmon_Gl = nullptr;

  mmon_stat::mmon_stat (uint64_t size, uint64_t atomic_size)
    : temp_stat {size},
      stat {atomic_size}
  {}

  mmon_stat::mmon_stat (uint64_t size)
    : temp_stat {size},
      stat {size}
  {}

  mmon_stat::mmon_stat (const mmon_stat &rhs)
    : temp_stat {rhs.temp_stat},
      stat {rhs.stat.load ()}
  {}

  mmon_stat &mmon_stat::operator = (const mmon_stat &rhs)
  {
    if (&rhs != this)
      {
	temp_stat = rhs.temp_stat;
	stat = rhs.stat.load ();
      }
    return *this;
  }

  memory_monitor::memory_monitor (const char *server_name)
    : m_tag_map {5000},
      m_stat_map {},
      m_server_name {server_name},
      m_magic_number {*reinterpret_cast <const int *> ("MMON")},
      m_total_mem_usage {0},
      m_meta_alloc_count {0}
  {}

  size_t memory_monitor::get_alloc_size (char *ptr)
  {
#if defined(WINDOWS)
    size_t alloc_size = 0;
    assert (false);
#else
    size_t alloc_size = malloc_usable_size ((void *)ptr);
#endif // !WINDOWS

    if (alloc_size <= MMON_ALLOC_META_SIZE)
      {
	return alloc_size;
      }

    char *meta_ptr = ptr + alloc_size - MMON_ALLOC_META_SIZE;
    MMON_METAINFO *metainfo = (MMON_METAINFO *) meta_ptr;

    if (metainfo->magic_number == m_magic_number)
      {
	alloc_size = (size_t) metainfo->size - MMON_ALLOC_META_SIZE;
      }

    return alloc_size;
  }

  std::string memory_monitor::make_tag_name (const char *file, const int line)
  {
    std::string filecopy (file);
#if defined(WINDOWS)
    std::string target ("");
    assert (false);
#else
    std::string target ("/src/");
#endif // !WINDOWS

    size_t pos = filecopy.rfind (target);

    if (pos != std::string::npos)
      {
	filecopy = filecopy.substr (pos + target.length ());
      }

    return filecopy + ':' + std::to_string (line);
  }

  void memory_monitor::add_stat (char *ptr, size_t size, const char *file, const int line)
  {
    std::string tag_name;
    char *meta_ptr = NULL;
    MMON_METAINFO metainfo;

    assert (size >= 0);

    metainfo.size = (uint64_t) size;
    m_total_mem_usage += metainfo.size;

    tag_name = make_tag_name (file, line);

    //std::unique_lock<std::mutex> tag_map_lock (m_tag_map_mutex);
retry:
    const auto tag_search = m_tag_map.find (tag_name);
    if (tag_search != m_tag_map.end ())
      {
	metainfo.tag_id = tag_search->second;
	m_stat_map[metainfo.tag_id] += metainfo.size;
	/*auto stat_search = m_stat_map.find (metainfo.tag_id);
	if (stat_search != m_stat_map.end ())
	  {
	    stat_search->second.stat += metainfo.size;
	    //m_stat_map.find (metainfo.tag_id)->second.stat += metainfo.size;
	  }
	else
	  {
	    goto retry;
	  }*/
	//m_stat_map[metainfo.tag_id].stat += metainfo.size;
      }
    else
      {
	std::pair<tbb::concurrent_unordered_map<std::string, int>::iterator, bool> tag_map_success;
	//std::pair<tbb::concurrent_unordered_map<int, mmon_stat>::iterator, bool> stat_map_success;
	metainfo.tag_id = m_tag_map.size ();
	// tag is start with 0
	std::pair <std::string, int> tag_map_entry (tag_name, metainfo.tag_id);
	tag_map_success = m_tag_map.insert (tag_map_entry);
	if (!tag_map_success.second)
	  {
	    goto retry;
	  }

	//stat_map_success = m_stat_map.insert (std::make_pair (metainfo.tag_id, mmon_stat (metainfo.size)));
	m_stat_map[metainfo.tag_id] += metainfo.size;
      }
    //tag_map_lock.unlock ();

    // put meta info into the alloced chunk
    meta_ptr = ptr + metainfo.size - MMON_ALLOC_META_SIZE;
    metainfo.magic_number = m_magic_number;
    memcpy (meta_ptr, &metainfo, MMON_ALLOC_META_SIZE);
    m_meta_alloc_count++;
  }

  void memory_monitor::sub_stat (char *ptr)
  {
#if defined(WINDOWS)
    size_t alloc_size = 0;
    assert (false);
#else
    size_t alloc_size = malloc_usable_size ((void *)ptr);
#endif // !WINDOWS

    assert (ptr != NULL);

    if (alloc_size >= MMON_ALLOC_META_SIZE)
      {
	char *meta_ptr = ptr + alloc_size - MMON_ALLOC_META_SIZE;
	MMON_METAINFO *metainfo = (MMON_METAINFO *)meta_ptr;

	if (metainfo->magic_number == m_magic_number)
	  {
	    assert ((metainfo->tag_id >= 0 && metainfo->tag_id <= m_tag_map.size()));
	    //assert (m_stat_map.find (metainfo->tag_id)->second.stat.load () >= metainfo->size);
	    assert (m_stat_map[metainfo->tag_id].load() >= metainfo->size);
	    assert (m_total_mem_usage >= metainfo->size);

	    m_total_mem_usage -= metainfo->size;
	    m_stat_map[metainfo->tag_id] -= metainfo->size;
	    //m_stat_map.find (metainfo->tag_id)->second.stat -= metainfo->size;

	    memset (meta_ptr, 0, MMON_ALLOC_META_SIZE);
	    m_meta_alloc_count--;
	    assert (m_meta_alloc_count >= 0);
	  }
      }
  }

  void memory_monitor::aggregate_server_info (MMON_SERVER_INFO &server_info)
  {
    strncpy (server_info.server_name, m_server_name.c_str (), m_server_name.size () + 1);
    server_info.total_mem_usage = m_total_mem_usage.load ();
    server_info.monitoring_meta_usage = m_meta_alloc_count * MMON_ALLOC_META_SIZE;
    server_info.num_stat = m_tag_map.size ();

    for (auto it = m_tag_map.begin (); it != m_tag_map.end (); ++it)
      {
	//server_info.stat_info.push_back (std::make_pair (it->first, m_stat_map.find (it->second)->second.stat.load ()));
	server_info.stat_info.push_back (std::make_pair (it->first, m_stat_map[it->second].load ()));
      }

    const auto &comp = [] (const auto& stat_pair1, const auto& stat_pair2)
    {
      return stat_pair1.second > stat_pair2.second;
    };
    std::sort (server_info.stat_info.begin (), server_info.stat_info.end (), comp);
  }

  void memory_monitor::finalize_dump ()
  {
    double mem_usage_ratio = 0.0;
    FILE *outfile_fp = fopen ("finalize_dump.txt", "w+");
    MMON_SERVER_INFO server_info;

    auto MMON_CONVERT_TO_KB_SIZE = [] (uint64_t size)
    {
      return ((size) / 1024);
    };

    aggregate_server_info (server_info);

    fprintf (outfile_fp, "====================cubrid memmon====================\n");
    fprintf (outfile_fp, "Server Name: %s\n", server_info.server_name);
    fprintf (outfile_fp, "Total Memory Usage(KB): %lu\n\n", MMON_CONVERT_TO_KB_SIZE (server_info.total_mem_usage));
    fprintf (outfile_fp, "-----------------------------------------------------\n");

    fprintf (outfile_fp, "\t%-100s | %17s(%s)\n", "File Name", "Memory Usage", "Ratio");

    for (const auto &s_info : server_info.stat_info)
      {
	if (server_info.total_mem_usage != 0)
	  {
	    mem_usage_ratio = s_info.second / (double) server_info.total_mem_usage;
	    mem_usage_ratio *= 100;
	  }
	fprintf (outfile_fp, "\t%-100s | %17lu(%3d%%)\n",s_info.first.c_str (), MMON_CONVERT_TO_KB_SIZE (s_info.second),
		 (int)mem_usage_ratio);
      }
    fprintf (outfile_fp, "-----------------------------------------------------\n");
    fflush (outfile_fp);
    fclose (outfile_fp);
  }
} // namespace cubmem

using namespace cubmem;

bool mmon_is_mem_tracked ()
{
  return (mmon_Gl != nullptr);
}

int mmon_initialize (const char *server_name)
{
  int error = NO_ERROR;

  assert (server_name != NULL);
  assert (mmon_Gl == nullptr);

  if (prm_get_bool_value (PRM_ID_MEMORY_MONITORING))
    {
      mmon_Gl = new (std::nothrow) memory_monitor (server_name);

      if (mmon_Gl == nullptr)
	{
	  er_set (ER_ERROR_SEVERITY, ARG_FILE_LINE, ER_OUT_OF_VIRTUAL_MEMORY, 1, sizeof (memory_monitor));
	  error = ER_OUT_OF_VIRTUAL_MEMORY;
	  return error;
	}
    }
  return error;
}

void mmon_finalize ()
{
  if (mmon_is_mem_tracked ())
    {
#if !defined (NDEBUG)
      mmon_Gl->finalize_dump ();
#endif
      delete mmon_Gl;
      mmon_Gl = nullptr;
    }
}

size_t mmon_get_alloc_size (char *ptr)
{
  if (mmon_is_mem_tracked ())
    {
      return mmon_Gl->get_alloc_size (ptr);
    }
  // unreachable
  return 0;
}

void mmon_add_stat (char *ptr, size_t size, const char *file, const int line)
{
  if (mmon_is_mem_tracked ())
    {
      mmon_Gl->add_stat (ptr, size, file, line);
    }
}


void mmon_sub_stat (char *ptr)
{
  if (mmon_is_mem_tracked ())
    {
      mmon_Gl->sub_stat (ptr);
    }
}

void mmon_aggregate_server_info (MMON_SERVER_INFO &server_info)
{
  if (mmon_is_mem_tracked ())
    {
      mmon_Gl->aggregate_server_info (server_info);
    }
}
