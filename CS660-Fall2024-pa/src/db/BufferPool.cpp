#include <db/BufferPool.hpp>
#include <db/Database.hpp>
#include <numeric>
#include <stdexcept>

using namespace db;

BufferPool::BufferPool()
    : maxSize(DEFAULT_NUM_PAGES), currentSize(0)
// TODO pa1: add initializations if needed
{
  // TODO pa1: additional initialization if needed
}

BufferPool::~BufferPool() {
  // TODO pa1: flush any remaining dirty pages
  flushAllPages();
}

const size_t BufferPool::convertPageId2SizeT(const PageId &pid) const {

  std::hash<const db::PageId> hashPid;

  return hashPid(pid);
}
void BufferPool::evictPage() {
    // Get the least recently used page (the last item in the recentAccessList)
    PageId lruPid = recentAccessList.back();
    
    // If the page is dirty, flush it to disk
    if (dirtyList[lruPid]) {
        flushPage(lruPid);
    }

    // Remove the page from the buffer pool
    pages.erase(lruPid);
    dirtyList.erase(lruPid);
    recentAccessList.pop_back();  // Remove from the LRU list

    // Decrease the current size of the buffer pool
    currentSize--;
    
    std::cout << "Evicted page " << lruPid.page << " from file " << lruPid.file << std::endl;
}
Page &BufferPool::getPage(const PageId &pid) {
  // TODO pa1: If already in buffer pool, make it the most recent page and return it

  // TODO pa1: If there are no available pages, evict the least recently used page. If it is dirty, flush it to disk

  // TODO pa1: Read the page from disk to one of the available slots, make it the most recent page
  if (contains(pid)) {
    moveToFront(pid);
    return *pages.find(pid)->second.get();
  }

  if (currentSize >= maxSize) {
    PageId lruPid = recentAccessList.back();

    auto newPid = std::move(pages.find(lruPid)->second);
    newPid.get()->fill(pid.page);
    pages.insert(std::make_pair(pid, std::move(newPid)));
    //if (lruPid.page)
    {
      getDatabase().get(lruPid.file).writePage(getPage(lruPid), lruPid.page);
      pages.erase(lruPid);
    }
    getDatabase().get(pid.file).readPage(*pages.find(pid)->second.get(), pid.page);
    if (dirtyList[lruPid]) {
      flushPage(lruPid);
    }
  dirtyList[pid] = std::move(false);
    // pageAccessOrder.erase(lruPid);
    recentAccessList.pop_front();
  
  recentAccessList.push_front(pid);
  
    return *pages.find(pid)->second.get();
  }
  
  Page page;
  page.fill((char)pid.page);
  pages.insert(std::make_pair(pid, std::make_unique<Page>(page)));
  recentAccessList.push_front(pid);
  currentSize++;
  getDatabase().get(pid.file).readPage(page, pid.page);
  dirtyList[pid] = std::move(false);

  // return pages.at(hashPid);
  return *pages.find(pid)->second.get();

  /*if (currentSize >= maxSize) {
    PageId lruPid = recentAccessList.back();
    if (pages[lruPid]->isDirty()) {
      flushPage(lruPid);
    }
    pages.erase(lruPid);
    pageAccessOrder.erase(lruPid);
    recentAccessList.pop_back();
    currentSize--;
  }

  auto newPage = Database::readPageFromDisk(pid);
  pages[pid] = newPage;
  recentAccessList.push_front(pid);
  pageAccessOrder[pid] = recentAccessList.begin();
  currentSize++;

  return pages[pid];*/
}

void BufferPool::moveToFront(const PageId &pid) {
  //recentAccessList.splice(recentAccessList.begin(), recentAccessList);
  auto itor = std::find(recentAccessList.begin(), recentAccessList.end(), pid); 
  if (itor != recentAccessList.end())
  {
    recentAccessList.erase(itor);
  }
  
  recentAccessList.push_front(pid);
  // pageAccessOrder[hashPid] = recentAccessList.begin();
}

void BufferPool::markDirty(const PageId &pid) {
  // TODO pa1: Mark the page as dirty. Note that the page must already be in the buffer pool
  if (contains(pid)) {
    std::hash<const db::PageId> hashPid;
    dirtyList[pid] = std::move(true);
  }
}

bool BufferPool::isDirty(const PageId &pid) const {
  // TODO pa1: Return whether the page is dirty. Note that the page must already be in the buffer pool
  if (!contains(pid)) {
    throw std::logic_error("File not exists: " + pid.file);
  }
  return dirtyList.at(pid);
}

bool BufferPool::contains(const PageId &pid) const {
  // TODO pa1: Return whether the page is in the buffer pool
  // return pages.find(pid) != pages.end();
  size_t hashPid = convertPageId2SizeT(pid);
  // bool ss = pages.find(pid) != pages.end();
  return pages.find(pid) != pages.end();
}

void BufferPool::discardPage(const PageId &pid) {
  // TODO pa1: Discard the page from the buffer pool. Note that the page must already be in the buffer pool
  if (contains(pid)) {
    size_t hashPid = convertPageId2SizeT(pid);
    pages.erase(pid);
    // pageAccessOrder.erase(pid);
    // recentAccessList.remove(pid);
    dirtyList.erase(pid);
    currentSize--;
  }
}

void BufferPool::flushPage(const PageId &pid) {
  // TODO pa1: Flush the page to disk. Note that the page must already be in the buffer pool
  if (contains(pid)) {
    auto writer = getDatabase().get(pid.file).getWrites();
    if (std::find(writer.begin(), writer.end(), pid.page) == writer.end())
    {
      getDatabase().get(pid.file).writePage(getPage(pid), pid.page);
      dirtyList[pid] = std::move(false);
    }

  }
}

void BufferPool::flushFile(const std::string &file) {
  // TODO pa1: Flush all pages of the file to disk
  for (const auto &[pid, page] : pages) {
    if (pid.file == file && isDirty(pid)) {
      flushPage(pid);
    }
  }
}

void BufferPool::flushAllPages() {
  // for (const auto &pair : pages) {
  //   if (pair.second->isDirty()) {
  //     flushPage(pair.first);
  //   }
  // }
}
