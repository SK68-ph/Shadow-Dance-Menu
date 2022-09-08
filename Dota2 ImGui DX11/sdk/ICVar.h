#pragma once
#include "color.h"
#include <array>
#include <map>

class CCvar {
public:
    enum class EConvarType : std::uint8_t
    {
        BOOL = 0,
        INT32,
        UINT32,
        INT64,
        UINT64,
        FLOAT,
        DOUBLE,
        STRING,
        COLOR_RGBA,
        UNK_SOME_TWO_FLOATS,
        UNK_SOME_THREE_FLOATS,
        UNK_SOME_FOUR_FLOATS,
        UNK_SOME_THREE_FLOATS_AGAIN,
    };

    union ConVarValue
    {
        bool boolean{};
        std::uint64_t u64a;
        std::int64_t i64;
        std::uint32_t u32;
        std::int32_t i32;
        float flt;
        double dbl;
        const char* str;
        std::uint32_t clr_rgba;
        std::array<float, 2> two_floats;
        std::array<float, 3> three_floats;
        std::array<float, 4> four_floats;
    };

    struct ConVariable
    {
        const char* name{};
        void* next_convar_node_like_shit{};
        void* unk1{};
        void* unk2{};
        const char* help{};
        EConvarType type{};
        int unk_maybe_number_of_times_changed{};
        int flags{};
        void* unk4{};
        ConVarValue value{};
    };

    struct CvarNode
    {
        ConVariable* var{};
        int some_leaf_like_index_shit{};
    };


    std::map<const char*, CvarNode*>initialize() {
        std::map<const char*, CvarNode*> list;
        CvarNode* node1 = (*(CvarNode**)this);
        for (size_t i = 0; i < 4226; i++)
        {
            if (node1->var->name)
            {
                list.insert(std::pair<const char*, CvarNode*>(node1->var->name, node1));
            }
            
            node1 = ((CvarNode*)((u64)node1 + 0x10));
        }
        return list;
    }
};
