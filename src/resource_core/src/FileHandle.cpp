#include "FileHandle.hpp"
#include "ResourceError.hpp"
#include <sstream>

namespace lab4::resource
{

FileHandle::FileHandle() noexcept : file_stream_(nullptr), filename_(""), mode_(std::ios::in) {}

FileHandle::FileHandle(const std::string& filename, std::ios::openmode mode) : filename_(filename), mode_(mode)
{
    file_stream_ = std::make_unique<std::fstream>(filename, mode);
    if (!file_stream_->is_open())
    {
        throw ResourceError("Failed to open file: " + filename);
    }
}

FileHandle::~FileHandle()
{
    close();
}

FileHandle::FileHandle(FileHandle&& other) noexcept
    : file_stream_(std::move(other.file_stream_)), filename_(std::move(other.filename_)), mode_(other.mode_)
{
}

FileHandle& FileHandle::operator=(FileHandle&& other) noexcept
{
    if (this != &other)
    {
        close();
        file_stream_ = std::move(other.file_stream_);
        filename_ = std::move(other.filename_);
        mode_ = other.mode_;
    }
    return *this;
}

bool FileHandle::is_open() const noexcept
{
    return file_stream_ && file_stream_->is_open();
}

const std::string& FileHandle::filename() const noexcept
{
    return filename_;
}

void FileHandle::close()
{
    if (file_stream_)
    {
        file_stream_->close();
        file_stream_.reset();
    }
}

void FileHandle::validate_open() const
{
    if (!is_open())
    {
        throw ResourceError("File is not open: " + filename_);
    }
}

void FileHandle::write(const std::string& data)
{
    validate_open();
    (*file_stream_) << data;
    if (file_stream_->fail())
    {
        throw ResourceError("Failed to write to file: " + filename_);
    }
    file_stream_->flush();
}

std::string FileHandle::read()
{
    validate_open();

    file_stream_->clear();
    file_stream_->seekg(0, std::ios::beg);

    std::ostringstream oss;
    oss << file_stream_->rdbuf();

    if (file_stream_->fail() && !file_stream_->eof())
    {
        throw ResourceError("Failed to read from file: " + filename_);
    }

    return oss.str();
}

std::string FileHandle::read_line()
{
    validate_open();

    std::string line;
    if (!std::getline(*file_stream_, line))
    {
        if (file_stream_->fail() && !file_stream_->eof())
        {
            throw ResourceError("Failed to read line from file: " + filename_);
        }
    }

    return line;
}

std::fstream& FileHandle::stream()
{
    validate_open();
    return *file_stream_;
}

} // namespace lab4::resource
