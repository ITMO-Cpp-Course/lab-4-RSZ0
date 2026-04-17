#ifndef LAB4_RESOURCE_MANAGER_HPP
#define LAB4_RESOURCE_MANAGER_HPP

#include <fstream>
#include <memory>
#include <string>
#include <unordered_map>

namespace lab4::resource
{

class FileHandle;

class ResourceManager
{
  public:
    static ResourceManager& instance();

    std::shared_ptr<FileHandle> acquire(const std::string& filename, std::ios::openmode mode);

    void release(const std::string& filename);
    void clear_cache();
    size_t cache_size() const;

    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;
    ResourceManager(ResourceManager&&) = delete;
    ResourceManager& operator=(ResourceManager&&) = delete;

  private:
    ResourceManager() = default;
    ~ResourceManager() = default;

    struct CacheEntry
    {
        std::weak_ptr<FileHandle> handle;
        std::ios::openmode mode;
    };

    std::unordered_map<std::string, CacheEntry> cache_;
};

} // namespace lab4::resource

#endif // LAB4_RESOURCE_MANAGER_HPP
