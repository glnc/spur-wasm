/**
 * component_cache.cpp
 *
 * Purpose: Defines methods for the ComponentCache() class for storing components in the cache.
 *
 * @author Zayd Hammoudeh <zayd@ucsc.edu>
 * @version 0.00.00
 *
 * Copyright (C) 2018 Zayd Hammoudeh.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms of the MIT license.  See the
 * LICENSE file for details.
 *
 * Original Author: Marc Thurley.
 */

#include <algorithm>
#include <vector>
#include "component_cache.h"
#include "sampler_tools.h"

#if defined(__linux__) || defined(__EMSCRIPTEN__)

  #include <cstdint>

  uint64_t freeram() {
     return (uint64_t) 128 * 1024 * 1024;
  }

#elif __APPLE__ && __MACH__

  #include <sys/sysctl.h>

  uint64_t freeram() {
    int mib[2];
    int64_t physical_memory;
    mib[0] = CTL_HW;
    mib[1] = HW_MEMSIZE;
    size_t length = sizeof(int64_t);
    sysctl(mib, 2, &physical_memory, &length, NULL, 0);

    return physical_memory;
  }

#else

#endif



ComponentCache::ComponentCache(DataAndStatistics &statistics, SolverConfiguration &config) :
    statistics_(statistics), config_(config) {
}

void ComponentCache::init(Component &super_comp, bool quiet) {
  if (!quiet) {
    std::cout << "Initialize cache\n"
              << "Size of Cacheable Component:\t" << sizeof(CacheableComponent) << "\n"
              << "Size of MPZ Class:\t" << sizeof(mpz_class) << std::endl;
  }
  CacheableComponent &packed_super_comp = *new CacheableComponent(super_comp);
  my_time_ = 1;

  entry_base_.clear();
  entry_base_.reserve(2000000);
  entry_base_.push_back(new CacheableComponent());  // dummy Element
  table_.clear();
  table_.resize(1024*1024, 0);
  table_size_mask_ = table_.size() - 1;

  free_entry_base_slots_.clear();
  free_entry_base_slots_.reserve(10000);

  uint64_t free_ram = freeram();
  uint64_t max_cache_bound = 95 * (free_ram / 100);

  if (statistics_.maximum_cache_size_bytes_ == 0) {
    statistics_.maximum_cache_size_bytes_ = max_cache_bound;
  }

  if (!quiet) {
    if (statistics_.maximum_cache_size_bytes_ > free_ram) {
      std::cout << "\n";
      //PrintWarning("Maximum cache size larger than free RAM available");
      std::cout << " Free RAM " << free_ram / 1000000 << "MB\n";
    }
    std::cout << "Maximum cache size:\t"
         << statistics_.maximum_cache_size_bytes_ / 1000000 << " MB\n"
         << std::endl;
  }

  assert(!statistics_.cache_full());

  if (entry_base_.capacity() == entry_base_.size())
    entry_base_.reserve(2 * entry_base_.size());

  entry_base_.push_back(&packed_super_comp);

  statistics_.incorporate_cache_store(packed_super_comp);

  super_comp.set_id(1);
}

void ComponentCache::test_descendantstree_consistency() {
  for (unsigned id = 2; id < entry_base_.size(); id++)
    if (entry_base_[id] != nullptr) {
      CacheEntryID act_child = entry(id).first_descendant();
      while (act_child) {
        CacheEntryID next_child = entry(act_child).next_sibling();
        assert(entry(act_child).father() == id);

        act_child = next_child;
      }
      CacheEntryID father = entry(id).father();
      CacheEntryID act_sib = entry(father).first_descendant();

      bool found = false;
      while (act_sib && !found) {
        CacheEntryID next_sib = entry(act_sib).next_sibling();
        if (act_sib == id)
          found = true;
        act_sib = next_sib;
      }
      assert(found);
    }
}

bool ComponentCache::deleteEntries() {
  assert(statistics_.cache_full());

  // Build a list of all the scores, sorts the list, then delete all
  // entries with a score below the median.
  std::vector<double> scores;
  for (auto it = entry_base_.begin() + 1; it != entry_base_.end(); it++)
    if (*it != nullptr && (*it)->isDeletable()) {
      scores.push_back(static_cast<double>((*it)->creation_time()));
    }
  std::sort(scores.begin(), scores.end());
  double cutoff = scores[scores.size() / 2];

  //std::cout << "cutoff" << cutoff  << " entries: "<< entry_base_.size()<< std::endl;

  // first : go through the EntryBase and mark the entries to be deleted as deleted (i.e. EMPTY
  // note we start at index 2,
  // since index 1 is the whole formula,
  // should always stay here!
  for (unsigned id = 2; id < entry_base_.size(); id++)
    if (entry_base_[id] != nullptr &&
        entry_base_[id]->isDeletable() &&
        static_cast<double>(entry_base_[id]->creation_time()) <= cutoff) {
      removeFromDescendantsTree(id);
      eraseEntry(id);
    }
  // then go through the Hash Table and erase all Links to empty entries


#ifdef DEBUG
  test_descendantstree_consistency();
#endif

  reHashTable(table_.size());
  statistics_.sum_size_cached_components_ = 0;
  statistics_.sum_bytes_cached_components_ = 0;
  statistics_.sys_overhead_sum_bytes_cached_components_ = 0;

  statistics_.sum_bytes_pure_cached_component_data_ = 0;

  for (unsigned id = 2; id < entry_base_.size(); id++)
    if (entry_base_[id] != nullptr) {
      statistics_.sum_size_cached_components_ +=
          entry_base_[id]->num_variables();
      statistics_.sum_bytes_cached_components_ +=
          entry_base_[id]->SizeInBytes();
      statistics_.sum_bytes_pure_cached_component_data_ +=
          entry_base_[id]->data_only_byte_size();
       statistics_.sys_overhead_sum_bytes_cached_components_ +=
           entry_base_[id]->sys_overhead_SizeInBytes();
    }

  statistics_.num_cached_components_ = entry_base_.size();
  compute_byte_size_infrastructure();

  //std::cout << " \t entries: "<< entry_base_.size() - free_entry_base_slots_.size()<< std::endl;
  return true;
}


uint64_t ComponentCache::compute_byte_size_infrastructure() {
  statistics_.cache_infrastructure_bytes_memory_usage_ =
      sizeof(ComponentCache)
      + sizeof(CacheEntryID)* table_.capacity()
      + sizeof(CacheableComponent *)* entry_base_.capacity()
      + sizeof(CacheEntryID) * free_entry_base_slots_.capacity();
  return statistics_.cache_infrastructure_bytes_memory_usage_;
}

void ComponentCache::debug_dump_data() {
  std::cout << "sizeof (CacheableComponent *, CacheEntryID) "
       << sizeof(CacheableComponent *) << ", "
       << sizeof(CacheEntryID) << std::endl;
  std::cout << "table (size/capacity) " << table_.size()
       << "/" << table_.capacity() << std::endl;
  std::cout << "entry_base_ (size/capacity) " << entry_base_.size()
       << "/" << entry_base_.capacity() << std::endl;
  std::cout << "free_entry_base_slots_ (size/capacity) " << free_entry_base_slots_.size()
       << "/" << free_entry_base_slots_.capacity() << std::endl;

//  uint64_t size_model_counts = 0;
  uint64_t alloc_model_counts = 0;
  for (auto &pentry : entry_base_)
    if (pentry != nullptr) {
//      size_model_counts += pentry->size_of_model_count();
      alloc_model_counts += pentry->alloc_of_model_count();
    }
  std::cout << "model counts size " << alloc_model_counts << std::endl;
}





