#include "ResourceError.hpp"

namespace lab4::resource
{

ResourceError::ResourceError(const std::string& message) : std::runtime_error("ResourceError: " + message) {}

} // namespace lab4::resource
