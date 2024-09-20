#include <db/Database.hpp>
#include <memory>
#include <stdexcept>
#include <unordered_map>

using namespace db;

BufferPool &Database::getBufferPool() { return bufferPool; }

Database &db::getDatabase() {
  static Database instance;
  return instance;
}

void Database::add(std::unique_ptr<DbFile> file) {
  // TODO pa1: add the file to the catalog. Note that the file must not exist.
  const std::string &fileName = file->getName(); // 假设 DbFile 有一个 getName() 方法
  if (fileCatalog.find(fileName) != fileCatalog.end()) {
    throw std::logic_error("File already exists: " + fileName);
  }
  fileCatalog[fileName] = std::move(file);

}

std::unique_ptr<DbFile> Database::remove(const std::string &name) {
  // TODO pa1: remove the file from the catalog. Note that the file must exist.
  auto it = fileCatalog.find(name);
  if (it == fileCatalog.end()) {
    throw std::logic_error("File does not exist: " + name);
  }
  bufferPool.flushFile(name); // 假设 BufferPool 有一个 flushFile(name) 方法
  auto file = std::move(it->second);
  fileCatalog.erase(it);
  return file;
}

DbFile &Database::get(const std::string &name) const {
  // TODO pa1: get the file from the catalog. Note that the file must exist.
  auto it = fileCatalog.find(name);
  if (it == fileCatalog.end()) {
    throw std::logic_error("File does not exist: " + name);
  }
  return *(it->second);
}
