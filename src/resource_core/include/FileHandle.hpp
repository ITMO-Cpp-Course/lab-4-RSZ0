#ifndef LAB4_FILE_HANDLE_HPP
#define LAB4_FILE_HANDLE_HPP

#include <fstream>
#include <memory>
#include <string>

namespace lab4::resource
{

class FileHandle
{
  public:
    FileHandle() noexcept;

    explicit FileHandle(const std::string& filename, std::ios::openmode mode);

    ~FileHandle();

    // Запрет копирования
    FileHandle(const FileHandle&) = delete;
    FileHandle& operator=(const FileHandle&) = delete;

    FileHandle(FileHandle&& other) noexcept;

    FileHandle& operator=(FileHandle&& other) noexcept;

    bool is_open() const noexcept;

    void close();

    void write(const std::string& data);

    std::string read();

    std::string read_line();

    std::fstream& stream();

    const std::string& filename() const noexcept;

  private:
    std::unique_ptr<std::fstream> file_stream_;
    std::string filename_;
    std::ios::openmode mode_;

    void validate_open() const;
};

} // namespace lab4::resource

#endif // LAB4_FILE_HANDLE_HPP
