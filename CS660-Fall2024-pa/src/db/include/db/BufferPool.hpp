#pragma once

#include <db/types.hpp>
#include <list>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace db {
constexpr size_t DEFAULT_NUM_PAGES = 50;
/**
 * @brief Represents a buffer pool for database pages.
 * @details The BufferPool class is responsible for managing the database pages in memory.
 * It provides functions to get a page, mark a page as dirty, and check the status of pages.
 * The class also supports flushing pages to disk and discarding pages from the buffer pool.
 * @note A BufferPool owns the Page objects that are stored in it.
 */
class BufferPool {
  // TODO pa1: add private members
private:
    struct Frame {
        PageId pid;
        Page page;
        bool dirty;
    };

    std::list<PageId> recentAccessList;  // LRU list to track page access order
    std::unordered_map<PageId, std::unique_ptr<Page>> pages;  // Stores pages in memory
    std::unordered_map<PageId, bool> dirtyList;  // Tracks which pages are dirty

    size_t maxSize;      // Max number of pages in the buffer pool
    size_t currentSize;  // Current number of pages in the buffer pool

    // Helper functions
    void evictPage();  // Evicts the least recently used page
    void moveToFront(const PageId &pid);  // Moves a page to the front of the LRU list
    const size_t convertPageId2SizeT(const PageId &pid) const;

public:
  /**
   * @brief: Constructs a BufferPool object with the default number of pages.
   */
  explicit BufferPool();

  /**
   * @brief: Destructs a BufferPool object after flushing all dirty pages to disk.
   */
  ~BufferPool();

  BufferPool(const BufferPool &) = delete;

  BufferPool(BufferPool &&) = delete;

  BufferPool &operator=(const BufferPool &) = delete;

  BufferPool &operator=(BufferPool &&) = delete;

  /**
   * @brief: Returns the page with the specified page id.
   * @param pid: The page id of the page to return.
   * @return: The page with the specified page id.
   * @note This method should make this page the most recently used page.
   */
  Page &getPage(const PageId &pid);

  /**
   * @brief: Marks the page with the specified page id as dirty.
   * @param pid: The page id of the page to mark as dirty.
   */
  void markDirty(const PageId &pid);

  /**
   * @brief: Returns whether the page with the specified page id is dirty.
   * @param pid: The page id of the page to check.
   * @return: True if the page is dirty, false otherwise.
   */
  bool isDirty(const PageId &pid) const;

  /**
   * @brief: Returns whether the buffer pool contains the page with the specified page id.
   * @param pid: The page id of the page to check.
   * @return: True if the buffer pool contains the page, false otherwise.
   */
  bool contains(const PageId &pid) const;

  /**
   * @brief: Discards the page with the specified page id from the buffer pool.
   * @param pid: The page id of the page to discard.
   * @note This method does NOT flush the page to disk.
   * @note This method also updates the LRU and dirty pages to exclude tracking this page.
   */
  void discardPage(const PageId &pid);

  /**
   * @brief: Flushes the page with the specified page id to disk.
   * @param pid: The page id of the page to flush.
   * @note This method should remove the page from dirty pages.
   */
  void flushPage(const PageId &pid);
  /**
   * @brief: Flushes all dirty pages in the specified file to disk.
   * @param file: The name of the associated file.
   * @note This method should call BufferPool::flushPage(pid).
   */
  void flushFile(const std::string &file);

  void flushAllPages(); // Clear flush all pages, added method

private:
  struct PageInfo {
    PageId pid;
    Page page;
  };

  static const size_t DEFAULT_NUM_PAGES = 50; // Adjusted for consistency
  size_t maxSize;
  size_t currentSize;
  // std::vector<PageInfo> pages;
  // std::unordered_map<size_t, Page> pages;
  std::unordered_map<PageId, std::unique_ptr<Page>, std::hash<const db::PageId>> pages;
  std::unordered_map<size_t, std::list<PageId>::iterator> pageAccessOrder;
  std::list<PageId> recentAccessList;
  std::unordered_map<PageId, bool, std::hash<const db::PageId>> dirtyList;
  std::unordered_map<size_t, PageId> hashMap;
  void moveToFront(const PageId &pid);

  const size_t convertPageId2SizeT(const PageId &pid) const;
};
} // namespace db
