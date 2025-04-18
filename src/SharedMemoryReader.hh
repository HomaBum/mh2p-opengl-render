#ifndef SHAREDMEMORYREADER_H
#define SHAREDMEMORYREADER_H

#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <stdexcept>
#include <iostream>
#include <cstring>
#include <atomic>

class SharedMemoryReader {
private:
    int fd = -1;
    void* addr = MAP_FAILED;
    size_t size = 0;
    std::string path;
    volatile uint8_t* flags; // Указатель на флаги в shared memory

public:
    // Конструктор
    SharedMemoryReader(const std::string& path = "/dev/shmem/mh2p_opengl_render", size_t size = 1024*4)
        : path(path), size(size) {
        open();
        flags = static_cast<volatile uint8_t*>(addr);
    }

    // Деструктор
    ~SharedMemoryReader() {
        close();
    }

    bool isDataReady() const {
        return flags[0] == 1 && flags[1] == 0;
    }

    // Открыть shared memory
    void open() {
        // Открываем существующую shared memory (только чтение)
        fd = ::open(path.c_str(), O_RDWR | O_CREAT, 0666);
        if (fd == -1) {
            throw std::runtime_error("Failed to open shared memory: " + std::string(strerror(errno)));
        }

        if (ftruncate(fd, size) == -1) {
        	::close(fd);
            throw std::runtime_error("ftruncate failed");
        }

        // Отображаем в память
        addr = mmap(nullptr, size, PROT_READ, MAP_SHARED, fd, 0);
        if (addr == MAP_FAILED) {
            ::close(fd);
            throw std::runtime_error("Failed to mmap shared memory: " + std::string(strerror(errno)));
        }
    }

    // Закрыть shared memory
    void close() {
        if (addr != MAP_FAILED) {
            munmap(addr, size);
            addr = MAP_FAILED;
        }
        if (fd != -1) {
            ::close(fd);
            fd = -1;
        }
    }

    // Чтение данных
    void read(void* buffer, size_t offset, size_t count) {
        if (addr == MAP_FAILED) {
            throw std::runtime_error("Shared memory not mapped");
        }

        // Получаем указатель на флаги
        flags = static_cast<volatile uint8_t*>(addr);

        // Ждем готовности данных (data_ready == 1) и отсутствия блокировки (write_lock == 0)
        while (flags[0] != 1 || flags[1] == 1) {
            usleep(1000); // Задержка 1 мс вместо busy-wait
        }

        // Проверяем границы
        if (offset + count + 2 > size) { // +2 для флагов
            throw std::runtime_error("Read operation out of bounds");
        }

        // Копируем данные (пропускаем 2 байта флагов)
        memcpy(buffer, static_cast<char*>(addr) + offset + 2, count);
    }

    // Получить указатель на данные
    const void* getData() const {
        if (addr == MAP_FAILED) {
            throw std::runtime_error("Shared memory not mapped");
        }
        return addr;
    }

    // Получить размер shared memory
    size_t getSize() const {
        return size;
    }
};

#endif
