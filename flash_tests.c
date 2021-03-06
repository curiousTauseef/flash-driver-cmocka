#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "flash.h"

static void expect_io_write(void **state, io_address addr, io_data data)
{
    expect_value(io_write, offset, addr);
    expect_value(io_write, data, data);
}

static void expect_io_read(void **state, io_address addr, io_data value_to_return)
{
    expect_value(io_read, offset, addr);
    will_return(io_read, value_to_return);
}

static void expect_reset(void **state)
{
    expect_value(io_write, offset, 0);
    expect_value(io_write, data, 0xff);
}

static void expect_enter_programming_mode(void **state)
{
    expect_io_write(state, 0, 0x40);
    expect_io_write(state, 0xdead, 0xbeef);
}

static void test_program_succeeds_ready_immediately(void **state) {
    expect_enter_programming_mode(state);
    expect_io_read(state, 0, 1<<7);

    assert_int_equal(FLASH_SUCCESS, flash_program(0xdead, 0xbeef));
}

static void test_program_succeeds_after_waiting_for_ready(void **state) {
    expect_enter_programming_mode(state);
    expect_io_read(state, 0, 0);
    expect_io_read(state, 0, 0);
    expect_io_read(state, 0, 1<<7);

    assert_int_equal(FLASH_SUCCESS, flash_program(0xdead, 0xbeef));
}

static void test_program_vpp_error(void **state) {
    expect_enter_programming_mode(state);
    expect_io_read(state, 0, 1<<7 | 1<<3);
    expect_reset(state);

    assert_int_equal(FLASH_VPP_ERROR, flash_program(0xdead, 0xbeef));
}

static void test_program_program_error(void **state) {
    expect_enter_programming_mode(state);
    expect_io_read(state, 0, 1<<7 | 1<<4);
    expect_reset(state);

    assert_int_equal(FLASH_PROGRAM_ERROR, flash_program(0xdead, 0xbeef));
}

static void test_program_protected_block_error(void **state) {
    expect_enter_programming_mode(state);
    expect_io_read(state, 0, 1<<7 | 1<<1);
    expect_reset(state);

    assert_int_equal(FLASH_PROTECTED_BLOCK_ERROR, flash_program(0xdead, 0xbeef));
}

static void test_program_invalid_error(void **state) {
    expect_enter_programming_mode(state);
    expect_io_read(state, 0, 1<<7 | 1<<2);
    expect_reset(state);

    assert_int_equal(FLASH_INVALID_STATUS_ERROR, flash_program(0xdead, 0xbeef));
}

static const UnitTest tests[] = {
        unit_test(test_program_succeeds_ready_immediately),
        unit_test(test_program_succeeds_after_waiting_for_ready),
        unit_test(test_program_vpp_error),
        unit_test(test_program_program_error),
        unit_test(test_program_protected_block_error),
        unit_test(test_program_invalid_error),
};

int run_flash_tests() {
    print_message("\n============ starting %s\n", __FILE__);
    return run_tests(tests);
}

