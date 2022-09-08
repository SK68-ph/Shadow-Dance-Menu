#pragma once
#include "color.h"
#include <array>
#include <map>

class ConCommandBase {
public:
    struct ConCMDID
    {
        static inline constexpr auto BAD_ID = 0xFFFF;
        std::uint64_t impl{};

        bool IsGood() const noexcept
        {
            return impl != BAD_ID;
        }

        void Invalidate() noexcept
        {
            impl = BAD_ID;
        }
    };
    struct ConCMDRegistrationInfo
    {
        const char* cmd_name{};
        const char* help_str{};
        std::uint64_t flags{};
        void* callback{};
        void* unk1{};
        void* unk2{};
        void* unk3{};
        void* output_id_holder{};
    };
    void ConCommandSource2(const std::string_view& name, const std::string_view& desc, void(*callback)(void*, const ConCMDID&))
    {
        ConCommandBase::ConCMDRegistrationInfo info{};
        info.cmd_name = name.data();
        info.help_str = desc.data();
        info.callback = callback;
        info.output_id_holder = this;
        //Constructor::Invoke(&info);
    }
};


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


    void initialize() {
        CvarNode* node1 = (*(CvarNode**)this);
        for (size_t i = 0; i < 4226; i++)
        {
            if (strcmp(node1->var->name, "dota_camera_distance") == 0)
            {
                node1->var->value.flt = 3000.0f;
            }
            node1 = ((CvarNode*)((u64)node1 + 0x10));
        }
    }
};
