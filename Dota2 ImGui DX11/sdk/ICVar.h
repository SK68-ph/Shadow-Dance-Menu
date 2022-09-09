#pragma once
#include <array>
#include <map>


class ICVar {
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
        int unk4{};
        int CALLBACK_INDEX{};
        int unk5{};
        ConVarValue value{};
    };

    struct CvarNode
    {
        ConVariable* var{};
        int some_leaf_like_index_shit{};
    };


    struct ConVarID
    {
        static inline constexpr auto BAD_ID = 0xFFFFFFFF;
        std::uint64_t impl{};
        void* var_ptr{};

        bool IsGood() const noexcept
        {
            return impl != BAD_ID;
        }

        void Invalidate() noexcept
        {
            impl = BAD_ID;
        }
    };
    using CvarCallBack = void(*)(const ConVarID& id, int unk1, const ConVarValue* val, const ConVarValue* old_val);

    CvarCallBack GetCVarCallback(int index)
    {
        if (index)
        {
            uintptr_t* table = (*(uintptr_t**)(this + 0x80));
            if (table)
                return *reinterpret_cast<CvarCallBack*>(reinterpret_cast<std::uintptr_t>((void*)table) + 24 * index);
        }
        return nullptr;
    }
};
