#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>
#include <cstdio>
#include <fstream>

#include "../src/resource_core/include/FileHandle.hpp"
#include "../src/resource_core/include/ResourceError.hpp"
#include "../src/resource_core/include/ResourceManager.hpp"

using namespace lab4::resource;

TEST_CASE("RAII - файл автоматически закрывается при выходе из области видимости", "[FileHandle][RAII]")
{
    ResourceManager::instance().clear_cache();
    std::ofstream("test.txt") << "Hello, RAII!";

    {
        FileHandle fh("test.txt", std::ios::in);
        REQUIRE(fh.is_open());
    }

    FileHandle fh2("test.txt", std::ios::in);
    REQUIRE(fh2.is_open());

    ResourceManager::instance().clear_cache();
    std::remove("test.txt");
}

TEST_CASE("Исключение при открытии несуществующего файла", "[FileHandle][exceptions]")
{
    ResourceManager::instance().clear_cache();

    REQUIRE_THROWS_AS(FileHandle("nonexistent_file_12345.txt", std::ios::in), ResourceError);

    ResourceManager::instance().clear_cache();
}

TEST_CASE("Запись строки в файл и последующее чтение", "[FileHandle][IO]")
{
    ResourceManager::instance().clear_cache();
    const std::string test_data = "RAII is great!";

    {
        FileHandle fh("test_write.txt", std::ios::out);
        REQUIRE_NOTHROW(fh.write(test_data));
    }

    FileHandle fh("test_write.txt", std::ios::in);
    std::string content = fh.read();
    REQUIRE(content == test_data);

    ResourceManager::instance().clear_cache();
    std::remove("test_write.txt");
}

TEST_CASE("Move-конструктор FileHandle", "[FileHandle][move]")
{
    ResourceManager::instance().clear_cache();
    std::ofstream("test.txt") << "Hello, RAII!";

    FileHandle fh1("test.txt", std::ios::in);
    REQUIRE(fh1.is_open());
    REQUIRE(fh1.filename() == "test.txt");

    FileHandle fh2 = std::move(fh1);

    REQUIRE_FALSE(fh1.is_open());
    REQUIRE(fh2.is_open());
    REQUIRE(fh2.filename() == "test.txt");
    REQUIRE(fh2.read() == "Hello, RAII!");

    ResourceManager::instance().clear_cache();
    std::remove("test.txt");
}

TEST_CASE("Move-оператор присваивания FileHandle", "[FileHandle][move]")
{
    ResourceManager::instance().clear_cache();
    std::ofstream("test.txt") << "Hello, RAII!";
    std::ofstream("multi_line.txt") << "Line 1\nLine 2\nLine 3";

    FileHandle fh1("test.txt", std::ios::in);
    FileHandle fh2("multi_line.txt", std::ios::in);

    fh2 = std::move(fh1);

    REQUIRE_FALSE(fh1.is_open());
    REQUIRE(fh2.is_open());
    REQUIRE(fh2.read() == "Hello, RAII!");

    ResourceManager::instance().clear_cache();
    std::remove("test.txt");
    std::remove("multi_line.txt");
}

TEST_CASE("ResourceManager - начальное состояние кеша", "[ResourceManager][cache]")
{
    ResourceManager::instance().clear_cache();

    auto& rm = ResourceManager::instance();
    REQUIRE(rm.cache_size() == 0);

    ResourceManager::instance().clear_cache();
}

TEST_CASE("ResourceManager - первый запрос создаёт новый ресурс", "[ResourceManager][cache]")
{
    ResourceManager::instance().clear_cache();
    std::ofstream("test.txt") << "Hello, RAII!";

    auto& rm = ResourceManager::instance();
    auto h1 = rm.acquire("test.txt", std::ios::in);

    REQUIRE(rm.cache_size() == 1);
    REQUIRE(h1.use_count() == 1);

    ResourceManager::instance().clear_cache();
    std::remove("test.txt");
}

TEST_CASE("ResourceManager - повторный запрос возвращает тот же ресурс", "[ResourceManager][cache]")
{
    ResourceManager::instance().clear_cache();
    std::ofstream("test.txt") << "Hello, RAII!";

    auto& rm = ResourceManager::instance();
    auto h1 = rm.acquire("test.txt", std::ios::in);
    auto h2 = rm.acquire("test.txt", std::ios::in);

    REQUIRE(rm.cache_size() == 1);
    REQUIRE(h1.use_count() == 2);
    REQUIRE(h2.use_count() == 2);
    REQUIRE(h1.get() == h2.get());

    ResourceManager::instance().clear_cache();
    std::remove("test.txt");
}

TEST_CASE("ResourceManager - разделяемое использование ресурсов", "[ResourceManager][shared]")
{
    ResourceManager::instance().clear_cache();
    std::ofstream("test.txt") << "Hello, RAII!";

    auto& rm = ResourceManager::instance();
    auto h1 = rm.acquire("test.txt", std::ios::in);
    auto h2 = rm.acquire("test.txt", std::ios::in);

    REQUIRE(h1.get() == h2.get());
    REQUIRE(h1->read() == "Hello, RAII!");

    ResourceManager::instance().clear_cache();
    std::remove("test.txt");
}

TEST_CASE("ResourceManager - явное освобождение ресурса", "[ResourceManager][release]")
{
    ResourceManager::instance().clear_cache();
    std::ofstream("test.txt") << "Hello, RAII!";

    auto& rm = ResourceManager::instance();

    {
        auto h1 = rm.acquire("test.txt", std::ios::in);
        REQUIRE(rm.cache_size() == 1);
    }

    REQUIRE(rm.cache_size() == 1);

    rm.release("test.txt");
    REQUIRE(rm.cache_size() == 0);

    ResourceManager::instance().clear_cache();
    std::remove("test.txt");
}

TEST_CASE("ResourceManager - несовместимые режимы открытия", "[ResourceManager][errors]")
{
    ResourceManager::instance().clear_cache();
    std::ofstream("test.txt") << "Hello, RAII!";

    auto& rm = ResourceManager::instance();
    auto h1 = rm.acquire("test.txt", std::ios::in);

    REQUIRE_THROWS_AS(rm.acquire("test.txt", std::ios::out), ResourceError);

    ResourceManager::instance().clear_cache();
    std::remove("test.txt");
}

TEST_CASE("ResourceManager - очистка кеша", "[ResourceManager][clear]")
{
    ResourceManager::instance().clear_cache();
    std::ofstream("test.txt") << "Hello, RAII!";
    std::ofstream("multi_line.txt") << "Line 1\nLine 2\nLine 3";

    auto& rm = ResourceManager::instance();
    auto h1 = rm.acquire("test.txt", std::ios::in);
    auto h2 = rm.acquire("multi_line.txt", std::ios::in);

    REQUIRE(rm.cache_size() == 2);

    rm.clear_cache();
    REQUIRE(rm.cache_size() == 0);

    ResourceManager::instance().clear_cache();
    std::remove("test.txt");
    std::remove("multi_line.txt");
}

TEST_CASE("Конструктор по умолчанию создаёт закрытый файл", "[FileHandle][errors]")
{
    ResourceManager::instance().clear_cache();

    FileHandle fh;

    REQUIRE_FALSE(fh.is_open());
    REQUIRE(fh.filename() == "");

    ResourceManager::instance().clear_cache();
}

TEST_CASE("Попытка записи в закрытый файл вызывает исключение", "[FileHandle][errors]")
{
    ResourceManager::instance().clear_cache();

    FileHandle fh;
    REQUIRE_THROWS_AS(fh.write("data"), ResourceError);

    ResourceManager::instance().clear_cache();
}

TEST_CASE("Попытка чтения из закрытого файла вызывает исключение", "[FileHandle][errors]")
{
    ResourceManager::instance().clear_cache();

    FileHandle fh;
    REQUIRE_THROWS_AS(fh.read(), ResourceError);
    REQUIRE_THROWS_AS(fh.read_line(), ResourceError);

    ResourceManager::instance().clear_cache();
}

TEST_CASE("Чтение файла построчно", "[FileHandle][IO]")
{
    ResourceManager::instance().clear_cache();
    std::ofstream("multi_line.txt") << "Line 1\nLine 2\nLine 3";

    FileHandle fh("multi_line.txt", std::ios::in);

    REQUIRE(fh.read_line() == "Line 1");
    REQUIRE(fh.read_line() == "Line 2");
    REQUIRE(fh.read_line() == "Line 3");

    ResourceManager::instance().clear_cache();
    std::remove("multi_line.txt");
}

TEST_CASE("Доступ к сырому потоку файла", "[FileHandle][stream]")
{
    ResourceManager::instance().clear_cache();
    std::ofstream("test.txt") << "Hello, RAII!";

    FileHandle fh("test.txt", std::ios::in);

    std::string content;
    std::getline(fh.stream(), content);

    REQUIRE(content == "Hello, RAII!");

    ResourceManager::instance().clear_cache();
    std::remove("test.txt");
}

TEST_CASE("Кеш хранит несколько разных файлов", "[ResourceManager][multiple]")
{
    ResourceManager::instance().clear_cache();
    std::ofstream("file1.txt") << "File 1";
    std::ofstream("file2.txt") << "File 2";
    std::ofstream("file3.txt") << "File 3";

    auto& rm = ResourceManager::instance();
    auto h1 = rm.acquire("file1.txt", std::ios::in);
    auto h2 = rm.acquire("file2.txt", std::ios::in);
    auto h3 = rm.acquire("file3.txt", std::ios::in);

    REQUIRE(rm.cache_size() == 3);

    ResourceManager::instance().clear_cache();
    std::remove("file1.txt");
    std::remove("file2.txt");
    std::remove("file3.txt");
}

TEST_CASE("Разные файлы в кеше - разные объекты", "[ResourceManager][multiple]")
{
    ResourceManager::instance().clear_cache();
    std::ofstream("file1.txt") << "File 1";
    std::ofstream("file2.txt") << "File 2";

    auto& rm = ResourceManager::instance();
    auto h1 = rm.acquire("file1.txt", std::ios::in);
    auto h2 = rm.acquire("file2.txt", std::ios::in);

    REQUIRE(h1.get() != h2.get());

    ResourceManager::instance().clear_cache();
    std::remove("file1.txt");
    std::remove("file2.txt");
}

TEST_CASE("RAII - файл закрывается даже при выбросе исключения", "[FileHandle][RAII][exceptions]")
{
    ResourceManager::instance().clear_cache();
    std::ofstream("test.txt") << "Hello, RAII!";

    try
    {
        FileHandle fh("test.txt", std::ios::in);
        REQUIRE(fh.is_open());
        throw std::runtime_error("Test exception");
    }
    catch (const std::runtime_error&)
    {
        // Исключение поймано
    }

    FileHandle fh2("test.txt", std::ios::in);
    REQUIRE(fh2.is_open());

    ResourceManager::instance().clear_cache();
    std::remove("test.txt");
}

TEST_CASE("Явное закрытие файла освобождает ресурс", "[FileHandle][close]")
{
    ResourceManager::instance().clear_cache();
    std::ofstream("test.txt") << "Hello, RAII!";

    FileHandle fh("test.txt", std::ios::in);
    REQUIRE(fh.is_open());

    fh.close();

    REQUIRE_FALSE(fh.is_open());
    REQUIRE_THROWS_AS(fh.read(), ResourceError);

    ResourceManager::instance().clear_cache();
    std::remove("test.txt");
}

TEST_CASE("Повторное закрытие файла безопасно", "[FileHandle][close]")
{
    ResourceManager::instance().clear_cache();
    std::ofstream("test.txt") << "Hello, RAII!";

    FileHandle fh("test.txt", std::ios::in);

    REQUIRE_NOTHROW(fh.close());
    REQUIRE_NOTHROW(fh.close());

    ResourceManager::instance().clear_cache();
    std::remove("test.txt");
}

TEST_CASE("ResourceManager - Singleton всегда возвращает один объект", "[ResourceManager][singleton]")
{
    ResourceManager::instance().clear_cache();

    auto& rm1 = ResourceManager::instance();
    auto& rm2 = ResourceManager::instance();

    REQUIRE(&rm1 == &rm2);

    ResourceManager::instance().clear_cache();
}

TEST_CASE("Нельзя писать в файл, открытый только для чтения", "[FileHandle][errors]")
{
    ResourceManager::instance().clear_cache();
    std::ofstream("test.txt") << "Hello, RAII!";

    FileHandle fh("test.txt", std::ios::in);
    REQUIRE_THROWS_AS(fh.write("Cannot write"), ResourceError);

    ResourceManager::instance().clear_cache();
    std::remove("test.txt");
}

TEST_CASE("read_line возвращает пустую строку в конце файла", "[FileHandle][IO]")
{
    ResourceManager::instance().clear_cache();
    std::ofstream("test.txt") << "Hello, RAII!";

    FileHandle fh("test.txt", std::ios::in);

    REQUIRE(fh.read_line() == "Hello, RAII!");
    REQUIRE(fh.read_line() == "");

    ResourceManager::instance().clear_cache();
    std::remove("test.txt");
}

TEST_CASE("read возвращает пустую строку для пустого файла", "[FileHandle][IO]")
{
    ResourceManager::instance().clear_cache();
    std::ofstream("empty.txt").close();

    FileHandle fh("empty.txt", std::ios::in);
    REQUIRE(fh.read() == "");

    ResourceManager::instance().clear_cache();
    std::remove("empty.txt");
}