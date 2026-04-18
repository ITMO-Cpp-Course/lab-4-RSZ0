#include "ResourceManager.hpp"
#include "FileHandle.hpp"
#include "ResourceError.hpp"

namespace lab4::resource
{

ResourceManager& ResourceManager::instance()
{
    static ResourceManager manager;
    return manager;
}

std::shared_ptr<FileHandle> ResourceManager::acquire(const std::string& filename, std::ios::openmode mode)
{

    auto it = cache_.find(filename);
    if (it != cache_.end())
    {

        auto shared = it->second.handle.lock();
        if (shared)
        {

            if (it->second.mode != mode)
            {
                throw ResourceError("File '" + filename + "' already opened with different mode");
            }
            return shared;
        }

        cache_.erase(it);
    }

    auto handle = std::make_shared<FileHandle>(filename, mode);
    cache_[filename] = {handle, mode};
    return handle;
}

void ResourceManager::release(const std::string& filename)
{
    auto it = cache_.find(filename);
    if (it != cache_.end() && it->second.handle.expired())
    {
        cache_.erase(it);
    }
}

void ResourceManager::clear_cache()
{
    cache_.clear();
}

size_t ResourceManager::cache_size() const
{
    return cache_.size();
}

} // namespace lab4::resource
