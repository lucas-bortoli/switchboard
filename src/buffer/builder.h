#ifndef PROTOCOL_BUFFER_BUILDER_H
#define PROTOCOL_BUFFER_BUILDER_H

#include <cstdlib>
#include <cstring>

class BufferBuilder
{
  public:
    char* buffer;
    size_t size;

    BufferBuilder(size_t size) : size(size), offset_(0)
    {
        buffer = (char*)malloc(size);
    }

    ~BufferBuilder()
    {
        free(buffer);
    }

    void begin_message()
    {
        offset_ = 0;
        memset(buffer, 0, size);
    }

    // Escrever no buffer o valor de tipo especificado
    template <typename T> void add(const T& valor)
    {
        if (!can_still_fit(sizeof(T)))
            return;

        memcpy(buffer + offset_, &valor, sizeof(T));
        offset_ += sizeof(T);
    }

    // Escreve o conteúdo de outro buffer (ex. uma string)
    void add_buffer(const void* src_buffer, size_t src_size)
    {
        if (!can_still_fit(src_size))
            return;

        memcpy(buffer + offset_, src_buffer, src_size);
        offset_ += src_size;
    }

  private:
    // Offset atual de escrita no buffer
    size_t offset_;

    // Verifica se há espaço suficiente no buffer para o tamanho especificado
    bool can_still_fit(size_t dataSize)
    {
        return (offset_ + dataSize <= size);
    }
};

#endif // PROTOCOL_BUFFER_BUILDER_H
