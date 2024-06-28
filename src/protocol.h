#pragma once
#include <cstdint>

enum class ProtocolMessageKind : uint8_t
{
    REMOTE_LIST_FOLDER_REQUEST,
    REMOTE_LIST_FOLDER_RESPONSE_START,
    REMOTE_LIST_FOLDER_RESPONSE_ITEM,
    UPLOAD_REQUEST,
    DOWNLOAD_REQUEST,
};

struct __attribute__((packed)) RemoteListRequest
{
    ProtocolMessageKind kind;
};

struct __attribute__((packed)) RemoteListResponseStart
{
    uint64_t itemCount;
    char remotePath[128];
};

struct __attribute__((packed)) RemoteListResponseItem
{
    uint64_t size;
    char name[128];
};

struct __attribute__((packed)) UploadRequest
{
    ProtocolMessageKind kind;
    uint64_t size;
    uint64_t chunkCount;
    char name[128];
};