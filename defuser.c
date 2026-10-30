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

static uint32_t p1_step1(uint32_t a, uint32_t b) {
    return (a + 2u * b) ^ UINT32_C(0xc1da);
}

static uint32_t p1_step0(uint32_t a, uint32_t b) {
    return (3u * a) ^ (b + UINT32_C(0x7fa8));
}

static uint32_t p1_step2(uint32_t a, uint32_t b) {
    return (2u * a) + (b ^ UINT32_C(0x8b05));
}

static uint32_t p1_compute(uint64_t c) {
    uint32_t hi = (uint32_t)(c >> 32);
    uint32_t mid = (uint32_t)(c >> 16);
    uint32_t lo = (uint32_t)c;
    uint32_t s1 = p1_step1(hi, mid);
    uint32_t s0 = p1_step0(lo, hi);
    return p1_step2(s0, s1);
}

int solve_phase_1(uint64_t challenge, char *out, size_t cap){
    uint32_t values[MAX_FIELDS] = {0};
    values[0] = p1_compute(challenge);
    return write_answer(out, cap, values, 1);
}


int solve_phase_2(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 6;
    values[0] = (uint32_t)((challenge >> 18) << 1);
    values[0] = (values[0] & 0xffu) + 29u;

    for (uint32_t i = 1; i < 6; ++i) {
        uint32_t shift = 3u * i + 2u;
        uint32_t part = (uint32_t)(challenge >> shift) & 0xfu;
        values[i] = 4u * values[i - 1] + (part ^ i);
    }

    return write_answer(out, cap, values, count);
}



static uint32_t p3_compute(uint64_t challenge){
    uint32_t index = (uint32_t)(challenge & 7u);
    uint32_t x = (uint32_t)(challenge >> 32)
               ^ (uint32_t)(challenge >> 3);

    switch (index) {
    case 0:
        return (x * 2u) ^ 0x9b64u;
    case 1:
        return (x * 2u) ^ 0x5f36u;
    case 2:
        return x * 3u + 0x2bd2u;
    case 3:
        return x * 5u + 0x1aeau;
    case 4:
        return (x ^ 0x5cb3u) + (x >> 2);
    case 5:
        return (x ^ 0xd104u) + (x >> 1);
    case 6:
        return x * 5u + 0x17f43u;
    case 7:
        return x * 5u + 0xfa45u;
    default:
        return 0;
    }
}

int solve_phase_3(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    values[0] = (uint32_t)(challenge & 7u);
    values[1] = p3_compute(challenge);
    return write_answer(out, cap, values, 2);
}



static uint32_t p4_walk(uint32_t target, uint32_t low, uint32_t high, uint32_t salt){
    uint32_t mid = low + (high - low) / 2u;

    if (mid == target) {
        return (mid + 0xf5u) ^ salt;
    }

    uint32_t result;

    if (target < mid) {
        result = p4_walk(target, low, mid - 1u, salt);
        return ((result * 5u) ^ (mid + salt)) ^ 0x0fu;
    }

    result = p4_walk(target, mid + 1u, high, salt);
    return ((result * 5u) ^ (mid + salt)) ^ 0x0cu;
}

int solve_phase_4(uint64_t challenge, char *out, size_t cap) {
     uint32_t target = (uint32_t)(challenge & 0x7fu);
    uint32_t salt = (uint32_t)(challenge >> 7)
                  ^ (uint32_t)(challenge >> 32);

    uint32_t values[MAX_FIELDS] = {0};

    values[0] = p4_walk(target, 0u, 127u, salt);

    return write_answer(out, cap, values, 1);
}



typedef struct {
    uint64_t next;
    uint32_t value;
    uint32_t tag;
} P5Node;

static const P5Node p5_nodes[12] = {
    {0x405c40u, 0x0a25u, 0xd9a5u},  /* 0: 0x405ba0 */
    {0x405c10u, 0x0b68u, 0xf174u},  /* 1: 0x405bb0 */
    {0x405bb0u, 0x0279u, 0x13acu},  /* 2: 0x405bc0 */
    {0x405ba0u, 0x0516u, 0x5a58u},  /* 3: 0x405bd0 */
    {0x405c00u, 0x0655u, 0x249fu},  /* 4: 0x405be0 */
    {0x405c30u, 0x006du, 0xd3d2u},  /* 5: 0x405bf0 */
    {0x405bd0u, 0x0469u, 0xaf65u},  /* 6: 0x405c00 */
    {0x405c20u, 0x0850u, 0x023fu},  /* 7: 0x405c10 */
    {0x405be0u, 0x036du, 0xbc67u},  /* 8: 0x405c20 */
    {0x00000000u, 0x0125u, 0xd51au},/* 9: 0x405c30 */
    {0x405c50u, 0x072au, 0x7920u},  /* 10: 0x405c40 */
    {0x405bf0u, 0x0955u, 0x0c7fu}   /* 11: 0x405c50 */
};

static const unsigned p5_heads[8] = {
    2u, 1u, 7u, 8u, 4u, 6u, 3u, 0u
};

static uint32_t p5_traverse_local(
    uint64_t challenge,
    uint32_t *last_value
) {
    unsigned index = p5_heads[(unsigned)challenge & 7u];

    uint32_t steps = (uint32_t)((challenge >> 3) & 3u);

    uint32_t acc = (uint32_t)(challenge >> 6)
                 ^ (uint32_t)(challenge >> 32)
                 ^ p5_nodes[index].tag
                 ^ 0x4de3u;

    uint32_t value = p5_nodes[index].value;
    acc += value;

    for (uint32_t i = 0; i < steps; ++i) {
        uint64_t next = p5_nodes[index].next;

        if (next == 0)
            break;

        index = (unsigned)((next - 0x405ba0u) / 0x10u);

        value = p5_nodes[index].value;
        acc ^= p5_nodes[index].tag;
        acc += value;
    }

    *last_value = value;
    return acc;
}

int solve_phase_5(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    uint32_t last_value = 0;

    values[0] = p5_traverse_local(challenge, &last_value);
    values[1] = last_value;

    return write_answer(out, cap, values, 2);
}


static uint32_t p6_op(uint32_t op, uint32_t x)
{
    switch (op) {
    case 0:
        return (x ^ (x >> 2)) + 0xeef0u;

    case 1:
        return (x >> 3) | (x << 29);

    case 2:
        return ((x << 5) + 0x0dd0u) ^ (x * 3u);

    case 3:
        return (x ^ 0x6959u) + (x << 7);

    case 4:
        return (x + 0xac7fu) ^ (x >> 7);

    case 5:
        return ((x << 1) | (x >> 31)) ^ 0x4c11u;

    default:
        return 0u;
    }
}

static uint32_t p6_dispatch_local(uint64_t challenge){
    static const uint32_t op_table[8] = {
        5u, 2u, 1u, 4u, 0u, 4u, 3u, 2u
    };

    uint32_t first_index = (uint32_t)(challenge & 7u);
    uint32_t second_index = (uint32_t)((challenge >> 3) & 7u);

    uint32_t x = (uint32_t)(challenge >> 6)
               ^ (uint32_t)(challenge >> 32);

    uint32_t first = p6_op(op_table[first_index], x);
    uint32_t second_input = first ^ 0xf35au;

    uint32_t second = p6_op(op_table[second_index], second_input);

    return second + 0x8d74u;
}

int solve_phase_6(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    values[0] = p6_dispatch_local(challenge);
    return write_answer(out, cap, values, 1);
    
}



static uint32_t secret_mix_local(uint32_t x) {
    x = (x << 7) | (x >> 25);
    x += 0x682eu;
    x ^= 0xf0bbu;
    return x;
}

int solve_secret(uint64_t challenge, uint64_t context,
                 char *out, size_t cap) {
    uint32_t c = (uint32_t)context & 3u;

    uint32_t context_xor =
        (uint32_t)context ^ (uint32_t)(context >> 32);

    uint32_t challenge_xor =
        (uint32_t)challenge ^ (uint32_t)(challenge >> 32);

    uint32_t mix_input = context_xor ^ challenge_xor;

    uint32_t second = secret_mix_local(mix_input)
                    ^ (c * 0x682eu)
                    ^ (uint32_t)(context >> 16);

    uint32_t third = second
                   ^ (context_xor + c + 0xf0bbu
                      + (challenge_xor >> 7));

    int n = snprintf(out, cap,
                     "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
                     "OPEN%u %u %u",
                     c, second, third);

    if (n < 0 || (size_t)n >= cap)
        return 1;

    return 0;
}