#include "defuser.h"

#include <inttypes.h>
#include <stdio.h>

#define MAX_FIELDS 8

/*
 * 把 values 中的 count 个整数写成一行“空格分隔的无符号十进制整数”，存入 out。
 * 成功返回 0；count 为 0（还没有实现）、snprintf 出错或结果被截断时返回非零。
 */
static int write_answer(char *out, size_t cap, const uint32_t *values, size_t count) {
    size_t used = 0;
    for (size_t i = 0; i < count; ++i) {
        int size = snprintf(out + used, cap - used, "%s%" PRIu32, i ? " " : "", values[i]);
        if (size < 0 || (size_t)size >= cap - used) return 1;
        used += (size_t)size;
    }
    return count == 0;
}

int solve_phase_1(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 0;
    uint32_t low  = (uint32_t)challenge;
    uint32_t high = (uint32_t)(challenge >> 32);
    uint32_t mid  = (uint32_t)(challenge >> 16);

    uint32_t a = (high + low * 2u) ^ 0x1e40u;
    uint32_t b = (mid ^ 0x88bfu) + low * 4u;
    values[0] = (a + 0xabc9u) ^ b;

    count = 1;
    // TODO: 根据 phase_01 的汇编，用 challenge 算出口令中的各个整数，依次存入 values，并把 count 改为整数个数。
    return write_answer(out, cap, values, count);
}

int solve_phase_2(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 0;
    uint32_t prev = (uint32_t)(((challenge >> 5) & 0xffu) * 2u & 0xffu) + 0x1du;
    values[0] = prev;

    for (uint32_t i = 1; i < 6; ++i) {
        uint32_t bits = (uint32_t)((challenge >> (23u + 3u * (i - 1))) & 0x1fu) + 9u;
        uint32_t next = (prev * 5u - i) ^ bits;
        values[i] = next;
        prev = next;
    }

    count = 6;
    return write_answer(out, cap, values, count);
}
int solve_phase_3(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 0;

    uint32_t selector = (uint32_t)challenge & 7u;

    uint32_t x = (uint32_t)(challenge >> 3) ^ (uint32_t)(challenge >> 32);

    uint32_t result;
    switch (selector) {
        case 0:  result = (x ^ 0xc237u) + (x >> 1);   break; 
        case 1:  result = x * 5u + 0xe11eu;           break; 
        case 2:  result = (x * 4u) ^ 0xfd1bu;         break; 
        case 3:  result = (x * 2u) ^ 0x4f4eu;         break; 
        case 4:  result = x * 5u + 0x424e1u;          break; 
        case 5:  result = (x ^ 0x835fu) + (x >> 1);   break; 
        case 6:  result = x * 3u + 0x4d42u;           break; 
        default: result = x * 5u + 0x48f8au;          break; 
    }

    values[0] = selector;
    values[1] = result;
    count = 2;

    return write_answer(out, cap, values, count);
}

int solve_phase_4(uint64_t challenge, char *out, size_t cap)
{
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 0;

    uint32_t target = (uint32_t)challenge & 127u;
    uint32_t key = (uint32_t)(challenge >> 7)
                 ^ (uint32_t)(challenge >> 32);

    uint32_t left = 0u;
    uint32_t right = 127u;

    uint32_t mids[8];
    uint32_t branch_xor[8];
    size_t depth = 0;

    uint32_t result;

    for (;;) {
        uint32_t mid = left + (right - left) / 2u;

        if (target == mid) {
            result = (mid + 147u) ^ key;
            break;
        }

        mids[depth] = mid;

        if (target < mid) {
            branch_xor[depth] = 9u;
            right = mid - 1u;
        }
        else {
            branch_xor[depth] = 7u;
            left = mid + 1u;
        }

        ++depth;
    }

    while (depth > 0) {
        --depth;
        result = (result * 5u)
               ^ (mids[depth] + key)
               ^ branch_xor[depth];
    }

    values[0] = result;
    count = 1;

    return write_answer(out, cap, values, count);
}

int solve_phase_5(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 0;
    (void)challenge;
        const uint32_t node_value[] = {
        0xb843u, 0x289fu, 0x0dc0u, 0x3215u,
        0x31c6u, 0x757au, 0xadc8u, 0xb614u,
        0x9500u, 0x9a40u, 0x8dc6u, 0x726cu
    };

    const uint32_t node_tag[] = {
        0x0450u, 0x057du, 0x0a79u, 0x073cu,
        0x0026u, 0x0233u, 0x035eu, 0x064fu,
        0x0934u, 0x0b31u, 0x0870u, 0x0135u
    };

    const int node_next[] = {
        10, 9, 5, 2, 8, 11, 4, 6, 1, 0, -1, 7
    };

    const int start_table[] = {
        3, 2, 5, 11, 7, 6, 4, 8
    };

    uint32_t idx = (uint32_t)challenge & 7u;
    int curr_node = start_table[idx];

    uint32_t visit_cnt =
        1u + (uint32_t)((challenge >> 3) & 3u);

    uint32_t acc = (uint32_t)(challenge >> 6)
                 ^ (uint32_t)(challenge >> 32)
                 ^ 0x1998u;

    uint32_t last_tag = 0u;

    for (uint32_t i = 0; i < visit_cnt; ++i) {
        acc = (acc ^ node_value[curr_node])
            + node_tag[curr_node];

        last_tag = node_tag[curr_node];
        curr_node = node_next[curr_node];
    }

    values[0] = acc;
    values[1] = last_tag;
    count = 2;
    return write_answer(out, cap, values, count);
}

int solve_phase_6(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 0;
    (void)challenge;
    // TODO: 根据 phase_06 的汇编，用 challenge 算出口令中的各个整数，依次存入 values，并把 count 改为整数个数。
        const uint8_t op_map[] = {
        1, 2, 4, 0, 5, 4, 3, 4
    };

    const uint32_t indices[] = {
        (uint32_t)challenge & 7u,
        (uint32_t)(challenge >> 3) & 7u
    };

    uint32_t x = (uint32_t)(challenge >> 6)
               ^ (uint32_t)(challenge >> 32);

    for (size_t round = 0; round < 2; ++round) {
        switch (op_map[indices[round]]) {
            case 0:
                x = (4u * x + 0xa697u) ^ (7u * x);
                break;

            case 1:
                x = (x ^ 0x1aa4u) + 4u * x;
                break;

            case 2:
                x = ((x >> 2u) ^ x) + 0x1ec4u;
                break;

            case 3:
                x = ((x << 1u) | (x >> 31u)) ^ 0x93ebu;
                break;

            case 4:
                x = (x + 0x6bd7u) ^ (x >> 2u);
                break;

            case 5:
                x = (x >> 1u) | (x << 31u);
                break;
        }

        if (round == 0) {
            x ^= 0xd7ffu;
        }
    }

    values[0] = x + 0x80e6u;
    count = 1;
    return write_answer(out, cap, values, count);
}

int solve_secret(uint64_t challenge, uint64_t context, char *out, size_t cap) {
    (void)challenge;
    (void)context;
    (void)out;
    (void)cap;
    // TODO（选做）: 根据 secret_gate、secret_copy 和 secret_phase 的汇编构造完整的第七行，写入 out 后返回 0。
    // 第七行不一定只有整数，可以直接用 snprintf 写入；还没完成时保持返回 1，测试会显示 not attempted。
    uint32_t T_lo = (uint32_t)context;
    uint32_t T_hi = (uint32_t)(context >> 32);
    uint32_t C_lo = (uint32_t)challenge;
    uint32_t C_hi = (uint32_t)(challenge >> 32);

    uint32_t k = T_lo & 3u;

    uint32_t A = C_hi ^ C_lo;
    uint32_t B = T_hi ^ T_lo;

    uint32_t R = (k * 0xdae1u) ^ ((uint32_t)(context >> 16));

    uint32_t mixed = (7u * (A ^ B) + 0xdae1u) ^ 0x3affu;
    uint32_t second = mixed ^ R;

    uint32_t term = B + k + 0x3affu + (A >> 5u);
    uint32_t third = second ^ term;

    int n = snprintf(
        out, cap,
        "%032uOPEN%" PRIu32 " %" PRIu32 " %" PRIu32,
        0u, k, second, third
    );

    if (!(n < 0 || (size_t)n >= cap))
    {
        return 0;
    }

    return 1;
}
